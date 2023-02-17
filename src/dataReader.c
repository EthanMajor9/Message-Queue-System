#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#define MAX_MACHINES 10
#define MAX_MSG_LEN 100

#define TIMEOUT 60

struct machine_status {
    int id;
    char status[MAX_MSG_LEN];
    time_t last_msg_time;
};


struct master_list {
    struct machine_status machines[MAX_MACHINES];
    int num_active;
    int msgq_id;
};

int main() {
    int shm_id;
    int msgq_id;
    int i;
    
    struct master_list *mlist;
    
    struct msgbuf {
        long mtype;
        char mtext[MAX_MSG_LEN];
    } msg;

     struct shmid_ds shmid_ds;

    // Create message queue
    key_t msgKey = ftok(".", 16535);
    msgq_id = msgget(msgKey, IPC_CREAT | 0666);
    if (msgq_id == -1) {
        perror("msgget");
        exit(1);
    }

    // Create shared memory segment for master list
    key_t shmKey = ftok(".", 16535);
    shm_id = shmget(shmKey, sizeof(struct master_list), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    // Attach shared memory segment to process
    mlist = shmat(shm_id, NULL, 0);
    if (mlist == (struct master_list *) -1) {
        perror("shmat");
        exit(1);
    }

    // Initialize master list
    mlist->num_active = 0;
    mlist->msgq_id = msgq_id;
    for (i = 0; i < MAX_MACHINES; i++) {
        mlist->machines[i].id = -1;
        strcpy(mlist->machines[i].status, "offline");
    }

    // Main loop to process messages
    while (1) {
        // Wait for message from message queue
        if (msgrcv(msgq_id, &msg, MAX_MSG_LEN, 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        // Parse message to extract machine ID and status
        int id;
        char status[MAX_MSG_LEN];
        sscanf(msg.mtext, "%d %s", &id, status);

        // Check if machine is already in master list
        int idx = -1;
        for (i = 0; i < MAX_MACHINES; i++) {
            if (mlist->machines[i].id == id) {
                idx = i;
                break;
            }
        }

        // If machine is new, add it to master list
        if (idx == -1) {
            for (i = 0; i < MAX_MACHINES; i++) {
                if (mlist->machines[i].id == -1) {
                    idx = i;
                    mlist->num_active++;
                    break;
                }
            }
        }

        // Update machine status in master list
        if (idx != -1) {
            strcpy(mlist->machines[idx].status, status);
        }

        // Log event as required

        // Check for machines that have gone offline
        time_t curr_time = time(NULL);
        for (i = 0; i < MAX_MACHINES; i++) {
            if (mlist->machines[i].id != -1) {
                struct shmid_ds shmid_ds;
                // If machine is new, add it to master list
                if (idx == -1) {
                    for (i = 0; i < MAX_MACHINES; i++) {
                        if (mlist->machines[i].id == -1) {
                            idx = i;
                            mlist->num_active++;
                            mlist->machines[idx].last_msg_time = time(NULL); // initialize last_msg_time
                            break;
                        }
                    }
                }
                // Update machine status in master list
                if (idx != -1) {
                    strcpy(mlist->machines[idx].status, status);
                }

                // Log event as required

                // Check for machines that have gone offline
                time_t curr_time = time(NULL);
                for (i = 0; i < MAX_MACHINES; i++) {
                    if (mlist->machines[i].id != -1) {
                        if (strcmp(mlist->machines[i].status, "offline") != 0 &&
                            curr_time - mlist->machines[i].last_msg_time > TIMEOUT) {
                            mlist->machines[i].last_msg_time = curr_time;
                            strcpy(mlist->machines[i].status, "offline");

                            printf("Machine %d has gone offline\n", mlist->machines[i].id);
                        }
                    }
                }

                // Detach shared memory segment from process
                if (shmdt(mlist) == -1) {
                    perror("shmdt");
                    exit(1);
                }

                // Log event as required

                // Check for termination signal from controller process
                if (msgrcv(msgq_id, &msg, MAX_MSG_LEN, 1, IPC_NOWAIT) != -1) {
                    printf("Master process received termination signal\n");
                    break;
                }
            }
        }
    }
    // Remove message queue
    if (msgctl(msgq_id, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    // Remove shared memory segment
    if (shmctl(shm_id, IPC_RMID, &shmid_ds) == -1) {
        perror("shmctl");
        exit(1);
    }

    return 0;
}