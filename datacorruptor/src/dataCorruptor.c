#include "../inc/dataCorruptor.h"


// TODO:
//	LOGGING
//  ERROR HANDLING
//	CLEANUP

int main() {
    int shmID = 0;
    int retries = 0;
    key_t shmKey;
    key_t msgQueueKey;
    MasterList* masterList;

    // initialize random number generator
    srand(time(NULL));

    // create the shared memory key
    shmKey = ftok(".", 16535);
    if (shmKey == -1) {
        perror("FTOK Error");
        exit(EXIT_FAILURE);
    }

    // try to attach to shared memory
    while (retries < MAX_RETRIES) {
        if((shmID = shmget(shmKey, sizeof(MasterList), 0)) != -1) {
			break;
		}
        retries++;
        sleep(10);
    }

    // exit if retries hit 100
    if (retries == MAX_RETRIES) {
        exit(EXIT_FAILURE);
    }

	// Attach shared mem to masterList
	if((masterList = shmat(shmID, NULL, 0)) == (void*) -1) {
		perror("SHMAT error:");
		exit(EXIT_FAILURE);
	}

	wheelOfDeath(masterList, shmID);

	// Detach the shared memory from the masterlist
	if (shmdt(masterList) == -1) {
        perror("SHMDT Error:");
        exit(1);
    }
}


void wheelOfDeath(MasterList* masterList, int shmID) {
	int running = 1;
	int randomNum;
	int interval;

	while(running) {
		// Sleep for random interval between 10 and 30 seconds
		interval = (rand() % 21) + MIN_INTERVAL;
		sleep(interval);

		if(checkMessageQueueExists(masterList) == 0) {
			return;
		}

		randomNum = rand() % 21;
		printf("Randomnum = %d\n", randomNum);
		switch(randomNum) {
			// Do nothing
			case 0:
			case 8:
			case 19:
				break;
			//Kill DC-01
			case 1:
			case 4:
			case 11:
				if(masterList->dc[0].dcProcessID != 0) {
					killDC(masterList->dc[0].dcProcessID);
				}
				break;
			// Kill DC-02
			case 3:
			case 6:
			case 13:
				if(masterList->dc[1].dcProcessID != 0) {
					killDC(masterList->dc[1].dcProcessID);
				}
				break;
			// Kill DC-03
			case 2:
			case 5:
			case 15:
				if(masterList->dc[2].dcProcessID != 0) {
					killDC(masterList->dc[2].dcProcessID);
				}
				break;
			// Kill DC-04
			case 7:
				if(masterList->dc[3].dcProcessID != 0) {
					killDC(masterList->dc[3].dcProcessID);
				}
				break;
			// Kill DC-05
			case 9:
				if(masterList->dc[4].dcProcessID != 0) {
					killDC(masterList->dc[4].dcProcessID);
				}
				break;
			// Delete message queue
			case 10:
			case 17:
				killMessageQueue(masterList->msgQueueID);
				break;
			// Kill DC-06
			case 12:
				if(masterList->dc[5].dcProcessID != 0) {
					killDC(masterList->dc[5].dcProcessID);
				}
				break;
			// Kill DC-07
			case 14:
				if(masterList->dc[6].dcProcessID != 0) {
					killDC(masterList->dc[6].dcProcessID);
				}
				break;
			// Kill DC-08
			case 16:
				if(masterList->dc[7].dcProcessID != 0) {
					killDC(masterList->dc[7].dcProcessID);
				}
				break;
			// Kill DC-09
			case 18:
				if(masterList->dc[8].dcProcessID != 0) {
					killDC(masterList->dc[8].dcProcessID);
				}
				break;
			// Kill DC-10
			case 20:
				if(masterList->dc[9].dcProcessID != 0) {
					killDC(masterList->dc[9].dcProcessID);
				}
				break;
		}
	}	
} 

int checkMessageQueueExists(MasterList* masterList) {
    // Check if queue exists
	printf("MSG Queue ID : %d\n", masterList->msgQueueID);
	if(msgctl(masterList->msgQueueID, IPC_STAT, NULL) == -1) {
		if (errno == ENOENT) {
			// Detach the shared memory from the masterlist
			if (shmdt(masterList) == -1) {
				perror("SHMDT Error:");
				exit(EXIT_FAILURE);
			}
			return 0;
		}
	}
	return 1;
}

void killDC(int DCtoKill){
	if(kill(DCtoKill, SIGHUP) == -1) {
		perror("KILL Error:");
	}
}

void killMessageQueue(int msgQueueID) {
	if(msgctl(msgQueueID, IPC_RMID, NULL) == -1) {
		perror("MSGCCTL Error:");
	}
}