/*
* FILE: dataCreator.c
* PROJECT: A-03 The Hoochamacallit System
* FIRST VERSION: 03/11/2023
* PROGRAMMER(s): Ethan Major, Caleb Brown
* DESCRIPTION: This file contains the functions for the data creator utility which is responsible for sending messages at random through the message queue to the data reader
*/
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
	struct sembuf acquireOp = { 0, -1, SEM_UNDO };
	struct sembuf releaseOp = { 0, 1, SEM_UNDO };

	// Generate the msg queue key
	msgQueueKey = (ftok("../../common/bin",  15));

	// Check if the message queue exists, sleep and try again if not
	while ((qID = msgget(msgQueueKey, 0)) == -1) {
		sleep(MIN_INTERVAL);
	}
	
	// Generate semaphore key
	int semID;
	key_t semKey = ftok(".", 20);

	// Check if the semaphore exists
	if((semID = semget(semKey, 1, 0)) == -1) {
		// Create the semaphore
		if((semID = semget(semKey, 1, (IPC_CREAT | 0666))) == -1) {
			perror("SEMGET Error:");
			exit(EXIT_FAILURE);
		}

		// Initialize the semaphore
		if(semctl(semID, SETALL, SEM_INIT_VAL) == -1) {
			perror("SEMCTL Error:");
			exit(EXIT_FAILURE);
		}
	}

	// Release semaphore
	if(semop(semID, &releaseOp, 1) == -1) {
		perror("Line 59 SEMOP Error:");
		exit(EXIT_FAILURE);
	}

	// Format the first message
	msg.pid = getpid();
    msg.status = STATUSES[0].status;
    msg.msgType = STATUSES[0].msgType;
    strcpy(msg.statusMsg, STATUSES[0].statusMsg);

	// Send the message over the queue
	if(msgsnd(qID, &msg, (sizeof(msg) - sizeof(long)), 0) == -1) {
		perror("MSGSND Error:");
		exit(-1);
	}

	// Seed the random generator
	srand(time(NULL));

	// Main processing loop
	while (running) {
		// Sleep for random interval
		int interval = (rand() % 21) + MIN_INTERVAL;
		sleep(interval);

		// Generate message
		generate_message_status(&msg);

		if(msgsnd(qID, &msg, (sizeof(msg) - sizeof(long)), 0) == -1) {
			perror("MSGSND Error:");
			exit(EXIT_FAILURE);
		}

		// Format log message
		log.pid = msg.pid;
		log.status = msg.status;
		log.statusmsg = msg.statusMsg;
		time_t currentTime = time(NULL);
		log.timestamp = localtime(&currentTime);

		// Check if the machine is going offline
		if(msg.status == MSG_OFFLINE) {	
			running = 0;
		}

		// Acquire semaphore
		if(semop(semID, &acquireOp, 1) == -1) {
			perror("SEMOP Error:");
			exit(EXIT_FAILURE);
		}

		// Open logfile
		if((logfile = fopen(logfilepath, "a")) == NULL) {
			perror("FOPEN Error:");
			exit(EXIT_FAILURE);
		}

		// Log message
		LogMessage(logfile, &log);

		// Close file
		fclose(logfile);

		// Release semaphore
		if(semop(semID, &releaseOp, 1) == -1) {
			perror("SEMOP Error:");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

// FUNCTION: 	void generate_message_status(Message *msg)
// DESCRIPTION: This function generates a status message based on a random value between 0 and 6 for the utility to send over the message queue
// PARAMETERS:  Message* msg : pointer to the message struct
// RETURNS: 	None
void generate_message_status(Message *msg) {
    int status = rand() % 7;

	// Fill message struct
    msg->pid = getpid();
    msg->status = STATUSES[status].status;
    msg->msgType = STATUSES[status].msgType;
    strcpy(msg->statusMsg, STATUSES[status].statusMsg);
}

// FUNCTION: 	void LogMessage(FILE* logfile, Log* log)
// DESCRIPTION: This function logs the message to the file
// PARAMETERS:  FILE* logfile : pointer to the file to write to
//				Log* log : pointer to log struct
// RETURNS: 	None
void LogMessage(FILE* logfile, Log* log) {
	char formattedTime[TIME_LENGTH];
	// Format timestamp
	strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", log->timestamp);
	fprintf(logfile, "[%s] : DC [%d] - %s - %d (%s)\n", formattedTime, log->pid, log->statusmsg, log->status, log->statusmsg);
}