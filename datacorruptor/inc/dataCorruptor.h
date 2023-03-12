#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stddef.h>
#include <errno.h>

#define MAX_DC_ROLES 10
#define WOD_SIZE  20
#define MAX_RETRIES 100
#define MIN_INTERVAL 10
#define TIME_LENGTH 25

const char* logfilepath = "/tmp/dataCorruptor.log";

typedef struct DCInfo {
	pid_t dcProcessID;
	time_t lastTimeHeardFrom;
} DCInfo;

typedef struct MasterList {
	int msgQueueID;
	int numberOfDCs;
	DCInfo dc[MAX_DC_ROLES];
} MasterList;

typedef struct Log {
	struct tm* timestamp;
	pid_t pid;
} Log;


void wheelOfDeath(MasterList* masterList, int shmID);
int checkMessageQueueExists(MasterList* masterList);
void killDC(int DCtoKill);
void killMessageQueue(int msgQueueID);
void LogMessage(FILE* logfile, Log* log, int action, int DCID);