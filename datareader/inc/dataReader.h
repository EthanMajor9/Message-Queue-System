/*
* FILE: dataReader.h
* PROJECT: A-03 The Hoochamacallit System
* FIRST VERSION: 03/11/2023
* PROGRAMMER(s): Ethan Major, Caleb Brown
* DESCRIPTION: This file contains the functions prototypes, structure definitions and constants required for dataReader utility
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <errno.h>

#define MAX_DC_ROLES 10
#define MAX_MSG_SIZE 100
#define TIME_LENGTH 25

const char* logfilepath = "/tmp/dataMonitor.log";

typedef struct DCInfo {
	pid_t dcProcessID;
	time_t lastTimeHeardFrom;
} DCInfo;

typedef struct MasterList {
	int msgQueueID;
	int numberOfDCs;
	DCInfo dc[MAX_DC_ROLES];
} MasterList;

typedef struct Message{
	long msgType;
	pid_t pid;
	int status;
	char statusMsg[MAX_MSG_SIZE];
} Message;

typedef struct Log {
	struct tm* timestamp;
	pid_t pid;
	int entryNum;
	const char* operation;
	int status;
	const char* statusmsg;
} Log;

int checkSharedMemExists(key_t sharedMemKey);
int checkMessageQueueExists(key_t msgQueueKey);
void LogMessage(FILE* logfile, Log* log);
Log createLogEntry(int entryNum, int pid, int status, char* statusMsg, char* operation);
void removeEntryFromMasterList(MasterList* masterList, int index);
void checkAndRemoveTimedOutEntries(MasterList* masterList, Message message, FILE* logfile);
void updateOrAddDCMessage(MasterList* masterList, Message message, FILE* logfile);
void processStatus6Message(MasterList* masterList, Message message, FILE* logfile);
void processMessages(MasterList* masterList, FILE* logfile);