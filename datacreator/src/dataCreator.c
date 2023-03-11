
#include "../inc/dataCreator.h"

// TODO:
//	FIX KEY GENERATION

const Status STATUSES[] = {
    {MSG_OK, 1, "Everything is OKAY"},
    {MSG_FAIL, 2, "Hydraulic Pressure Failure"},
    {MSG_SAFTEYFAIL, 3, "Safety Button Failure"},
    {MSG_NOMATERIAL, 4, "No Raw Material In the Process"},
    {MSG_TEMPOUTOFRANGE, 5, "Operating Temperature Out of Range"},
    {MSG_OPERERROR, 6, "Operator Error"},
    {MSG_OFFLINE, 7, "Machine is Off-line"}
};

int main(int argc, char* argv[]) 
{
	int qID = 0;
	key_t msgQueueKey;
	Message msg;
	int running = 1;
	Log log;

	msgQueueKey = ftok(".",  16535);

	while ((qID = msgget(msgQueueKey, 0)) == -1) 
	{
		sleep(MIN_INTERVAL);
	}

	msg.pid = getpid();
    msg.status = STATUSES[0].status;
    msg.msgType = STATUSES[0].msgType;
    strcpy(msg.statusMsg, STATUSES[0].statusMsg);

	if(msgsnd(qID, &msg, (sizeof(msg) - sizeof(long)), 0) == -1) 
	{
		perror("msgsnd failed\n");
		exit(-1);
	}

	FILE* logfile = fopen(logfilepath, "w");

	if(logfile == NULL) 
	{
		perror("Failed to open file");
		exit(-1);
	}

	srand(time(NULL));

	while (running)
	{
		int interval = (rand() % 21) + MIN_INTERVAL;
		sleep(interval);

		generate_message_status(&msg);

		if(msgsnd(qID, &msg, (sizeof(msg) - sizeof(long)), 0) == -1) 
		{
			perror("msgsnd failed");
		}

		log.pid = msg.pid;
		log.status = msg.status;
		log.statusmsg = msg.statusMsg;
		time_t currentTime = time(NULL);
		log.timestamp = localtime(&currentTime);
		printf("%d\n", msg.status);

		LogMessage(logfile, &log);

		if(msg.status == MSG_OFFLINE) 
		{	
			running = 0;
		}
	}

	fclose(logfile);
}

void generate_message_status(Message *msg) 
{
    int status = rand() % 7;

    msg->pid = getpid();
    msg->status = STATUSES[status].status;
    msg->msgType = STATUSES[status].msgType;
    strcpy(msg->statusMsg, STATUSES[status].statusMsg);
}

void LogMessage(FILE* logfile, Log* log) 
{
	char formattedTime[TIME_LENGTH];
	strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", log->timestamp);
	fprintf(logfile, "[%s] : DC [%d] - %s - %d (%s)\n", formattedTime, log->pid, log->statusmsg, log->status, log->statusmsg);
}