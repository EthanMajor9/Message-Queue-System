#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Define the message queue key and message types
#define MSG_OK 0
#define MSG_FAIL 1
#define MSG_SAFTEYFAIL 2
#define MSG_NOMATERIAL 3
#define MSG_TEMPOUTOFRANGE 4
#define MSG_OPERERROR 5
#define MSG_OFFLINE 6

// Define the range for the random time interval between 10 and 30 seconds
#define MIN_INTERVAL 10
#define TIME_LENGTH 25

#define MESSAGE_SIZE 50

const char* logfilepath = "/tmp/dataCreator.log";

typedef struct Message {
	long msgType;
	pid_t pid;
	int status;
	char statusMsg[MESSAGE_SIZE];
} Message;

typedef struct Status { 
    int status;
    long msgType;
    char* statusMsg;
} Status;

typedef struct Log {
	struct tm* timestamp;
	pid_t pid;
	int status;
	const char* statusmsg;
} Log;

void LogMessage(FILE* logfile, Log* log);
void generate_message_status(Message *msg);