/* 
* FILE              : dataCreator.c
* PROJECT           : SENG2030 - Assignment #3
* PROGRAMMER        : Caleb Brown and Ethan Major
* FIRST VERSION     : 2023-02-13
* DESCRIPTION       : This C porgram simulates a machine sending 
*                     random status updates via a message queue                     
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <time.h>


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
#define MAX_INTERVAL 30

// Define the machine ID as the process ID
#define MACHINE_ID getpid()

// Function to generate a random status value
int getRandomStatus() {
  return rand() % 7;
}

// Define the textual descriptions for the different status codes
const char* statusDescriptions[] = {
    "Everything is OKAY",
    "Machine Failure",
    "Safety Button Failure",
    "No Material",
    "Time Out of Range",
    "Operational Error",
    "Offline"
};

// Function to log a message to the file
void logMessage(int pid, int status) {
    time_t now = time(NULL);
    struct tm* timeInfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, 20, "[%Y-%m-%d %H:%M:%S]", timeInfo);
    char message[100];
    sprintf(message, "DC [%d] - MSG SENT - Status %d (%s)\n", pid, status, statusDescriptions[status]);
    FILE* file = fopen("/tmp/dataCreator.log", "a");
    fprintf(file, "%s : %s", timestamp, message);
    fclose(file);
}




int main() {

    int msgqid;
    int interval;
    int status;
    int count = 0;
    int running = 1;
    
    struct msgbuf {
        int mtype;
        int machine_id;
        int status;
    } msg;

    // Initialize the random number generator
    srand(time(NULL));

    key_t msgKey = ftok(".", 16535);

    // Try to create or get the message queue
    while((msgqid = msgget(msgKey, 0))== -1){ 
        if(errno == ENOENT){
            sleep(10);
            break;
        }
        else {            
            perror("msgget failed");
            exit(EXIT_FAILURE);
        }      
    }
  
    // Send the "Everything is OKAY" message
    msg.mtype = MSG_OK;
    msg.machine_id = MACHINE_ID;
    msg.status = 0;
    if (msgsnd(msgqid, &msg, sizeof(msg.status), IPC_NOWAIT) == -1) {
        perror("msgsnd failed");
        exit(1);
    }
    count++;

    // Enter the main processing loop
    while (running) {
        // Generate a random time interval between 10 and 30 seconds
        interval = rand() % (MAX_INTERVAL - MIN_INTERVAL + 1) + MIN_INTERVAL;

        // Sleep for the random time interval
        sleep(interval);

        // Generate a random status value (except for the first message)
        if (count > 0) {
            status = getRandomStatus();
        } 
        else {
            status = 0;
        }

        // Send the machine status message
        switch (status) {
        case 0:
            msg.mtype = MSG_OK;
            break;
        case 1:
            msg.mtype = MSG_FAIL;
            break;
        case 2:
            msg.mtype = MSG_SAFTEYFAIL;
            break;
        case 3:
            msg.mtype = MSG_NOMATERIAL;
            break;
        case 4:
            msg.mtype = MSG_TEMPOUTOFRANGE;
            break;
        case 5:
            msg.mtype = MSG_OPERERROR;
            break;
        case 6:
            msg.mtype = MSG_OFFLINE;
            break;
        default:
            msg.mtype = MSG_OK;
            break;
        }
        
        //set message struct values
        msg.machine_id = MACHINE_ID;
        msg.status = status;

        if (msgsnd(msgqid, &msg, sizeof(msg.status), IPC_NOWAIT) == -1) {
            perror("msgsnd failed");
            exit(1);
        }

        //write a log message to the temp folder
        logMessage(MACHINE_ID, status);

        count++;

        //stope sending message status if offline
        if (status == MSG_OFFLINE){
            running = 0;
        }
    }

    //clean up message queue
    if (msgctl(msgqid, IPC_RMID, NULL) == -1){
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}