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

#define MAX_DC_ROLES 10
#define WOD_SIZE  20

typedef struct
{
    pid_t dcProcessID;
    time_t lastTimeHeardFrom;
} DCInfo;

typedef struct
{
    int msgQueueID;
    int numberOfDCs;
    DCInfo dc[MAX_DC_ROLES];
} MasterList;


int main() {
    int shm_id = 0;
    int msgq_id = 0;
    int retries = 0;
    int dc_exists = 0;
    key_t shm_key;
    key_t msgq_key;
    MasterList* master_list;
    int wod[WOD_SIZE] = {0, 1, 2, 3 ,4 ,5 ,6 ,7 ,8 ,9, 10, 11, 12, 13, 14, 15, 16 ,17, 18, 19, 20};
    // initialize random number generator
    srand(time(NULL));

    // create the shared memory key
    shm_key = ftok(".", 16535);
    if (shm_key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // try to attach to shared memory
    while (retries < 100) {
        shm_id = shmget(shm_key, sizeof(int), 0);
        if (shm_id != -1) {
            break;
        }
        retries++;
        sleep(10);
    }

    // exit if retries hit 100
    if (retries == 100) {
        exit(EXIT_FAILURE);
    }

    // attach to shared memory
    master_list = (MasterList*) shmat(shm_id, NULL, 0);
    if (master_list == (MasterList*) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // create the message queue key
    msgq_key = ftok(".", 16536);
    if (msgq_key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // try to attach to message queue
    msgq_id = msgget(msgq_key, 0);
    if (msgq_id == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // main processing loop
    while (1) {
        // sleep for a random amount of time between 10 and 30
        sleep(rand() % 21 + 10);

        // check if message queue still exists
        if (msgctl(msgq_id, IPC_STAT, NULL) == -1) {
            if (dc_exists) {
                fprintf(stderr, "msgQ is gone");
                shmdt(master_list);
                exit(EXIT_SUCCESS);
            }
        } else {
            dc_exists = 1;
        }

        int action = wod[rand() % WOD_SIZE];

        switch (action)
        {
            case 0:

                break;
            case 1: 

                break;
            case 2:

                break;

        }

    return 0;
}