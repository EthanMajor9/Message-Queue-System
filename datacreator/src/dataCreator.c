
#include "../inc/dataCreator.h"

const Status STATUSES[] = {
    {MSG_OK, 1, "Everything is OKAY"},
    {MSG_FAIL, 2, "Hydraulic Pressure Failure"},
    {MSG_SAFTEYFAIL, 3, "Safety Button Failure"},
    {MSG_NOMATERIAL, 4, "No Raw Material In the Process"},
    {MSG_TEMPOUTOFRANGE, 5, "Operating Temperature Out of Range"},
    {MSG_OPERERROR, 6, "Operator Error"},
    {MSG_OFFLINE, 7, "Machine is Off-line"}
};

int main(int argc, char* argv[]) {
	int qID = 0;
	key_t msgQueueKey;
	Message msg;
	int running = 1;
	Log log;
	FILE* logfile = NULL;

	// Idk why but this is how it has to work, sorry 
	msgQueueKey = (ftok("/home/ethan/Desktop/Projects/Hoochamacallit/common/bin",  15));
	printf("MSG Queue Key: %d\n", msgQueueKey);

	while ((qID = msgget(msgQueueKey, 0)) == -1) {
		sleep(MIN_INTERVAL);
	}
	
	int semID;
	key_t semKey = ftok(".", 20);
	printf("Semaphore Key: %d\n", semKey);

	if((semID = semget(semKey, 1, 0)) == -1) {
		if((semID = semget(semKey, 1, (IPC_CREAT | 0666))) == -1) {
			perror("SEMGET Error:");
		} else {
			if(semctl(semID, SETALL, SEM_INIT_VAL) == -1) {
				perror("SEMCTL Error:");
				exit(EXIT_FAILURE);
			}
		}
	}

	if(semop(semID, &releaseOp, 1) == -1) {
		perror("SEMCTL Error:");
		exit(EXIT_FAILURE);
	}

	msg.pid = getpid();
    msg.status = STATUSES[0].status;
    msg.msgType = STATUSES[0].msgType;
    strcpy(msg.statusMsg, STATUSES[0].statusMsg);

	if(msgsnd(qID, &msg, (sizeof(msg) - sizeof(long)), 0) == -1) {
		perror("MSGSND Error:");
		exit(-1);
	}

	srand(time(NULL));

	while (running) {
		int interval = (rand() % 21) + MIN_INTERVAL;
		sleep(interval);

		generate_message_status(&msg);

		if(msgsnd(qID, &msg, (sizeof(msg) - sizeof(long)), 0) == -1) {
			perror("MSGSND Error:");
			exit(EXIT_FAILURE);
		}

		log.pid = msg.pid;
		log.status = msg.status;
		log.statusmsg = msg.statusMsg;
		time_t currentTime = time(NULL);
		log.timestamp = localtime(&currentTime);

		if(msg.status == MSG_OFFLINE) {	
			running = 0;
		}

		if(semop(semID, &acquireOp, 1) == -1) {
			printf("Couldn't acquire semaphore\n");
			continue;
		}

		if((logfile = fopen(logfilepath, "a")) == NULL) {
			perror("FOPEN Error:");
			exit(EXIT_FAILURE);
		}

		LogMessage(logfile, &log);

		fclose(logfile);

		if(semop(semID, &releaseOp, 1) == -1) {
			perror("SEMCTL Error:");
			exit(EXIT_FAILURE);
		}
	}
	semctl (semID, 0, IPC_RMID, 0);
	return 0;
}

void generate_message_status(Message *msg) {
    int status = rand() % 7;

    msg->pid = getpid();
    msg->status = STATUSES[status].status;
    msg->msgType = STATUSES[status].msgType;
    strcpy(msg->statusMsg, STATUSES[status].statusMsg);
}

void LogMessage(FILE* logfile, Log* log) {
	char formattedTime[TIME_LENGTH];
	strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", log->timestamp);
	fprintf(logfile, "[%s] : DC [%d] - %s - %d (%s)\n", formattedTime, log->pid, log->statusmsg, log->status, log->statusmsg);
}