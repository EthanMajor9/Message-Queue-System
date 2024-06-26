/*
* FILE: dataCorruptor.c
* PROJECT: A-03 The Hoochamacallit System
* FIRST VERSION: 03/11/2023
* PROGRAMMER(s): Ethan Major, Caleb Brown
* DESCRIPTION: This file contains the functions for the data corruptor utility that is used for the purpose of killing DC's and deleting the message queue at random
*/

#include "../inc/dataCorruptor.h"

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
        exit(EXIT_FAILURE);
    }
	
	return 0;
}

// FUNCTION: 	void wheelOfDeath(MasterList* masterList, int shmID)
// DESCRIPTION: This function generates a random number between 0 and 20 and either kills the specified DC or deletes the message queue
// PARAMETERS:  MasterList* masterList : pointer to the masterlist in shared memory
//				int shmID: ID of the shared mem segment
// RETURNS: 	None
void wheelOfDeath(MasterList* masterList, int shmID) {
	int running = 1;
	int randomNum;
	int interval;
	Log log;
	time_t currentTime;
	FILE* logfile;

	if((logfile = fopen(logfilepath, "a")) == NULL) {
		perror("FOPEN Error:");
		exit(EXIT_FAILURE);
	}

	while(running) {
		// Sleep for random interval between 10 and 30 seconds
		interval = (rand() % 21) + MIN_INTERVAL;
		sleep(interval);

		// Check if the message queue exists
		if(checkMessageQueueExists(masterList) == 0) {
			fclose(logfile);
			return;
		}

		// Generate random number
		randomNum = rand() % 21;

		// Fill log timestamp with current local time
		currentTime = time(NULL);
		log.timestamp = localtime(&currentTime);

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
					log.pid = masterList->dc[0].dcProcessID;
					LogMessage(logfile, &log, randomNum, 1);
				}
				break;
			// Kill DC-02
			case 3:
			case 6:
			case 13:
				if(masterList->dc[1].dcProcessID != 0) {
					killDC(masterList->dc[1].dcProcessID);
					log.pid = masterList->dc[1].dcProcessID;
					LogMessage(logfile, &log, randomNum, 2);
				}
				break;
			// Kill DC-03
			case 2:
			case 5:
			case 15:
				if(masterList->dc[2].dcProcessID != 0) {
					killDC(masterList->dc[2].dcProcessID);
					log.pid = masterList->dc[2].dcProcessID;
					LogMessage(logfile, &log, randomNum, 3);
				}
				break;
			// Kill DC-04
			case 7:
				if(masterList->dc[3].dcProcessID != 0) {
					killDC(masterList->dc[3].dcProcessID);
					log.pid = masterList->dc[3].dcProcessID;
					LogMessage(logfile, &log, randomNum, 4);
				}
				break;
			// Kill DC-05
			case 9:
				if(masterList->dc[4].dcProcessID != 0) {
					killDC(masterList->dc[4].dcProcessID);
					log.pid = masterList->dc[4].dcProcessID;
					LogMessage(logfile, &log, randomNum, 5);
				}
				break;
			// Delete message queue
			case 10:
			case 17:
				killMessageQueue(masterList->msgQueueID);
				log.pid = masterList->dc[4].dcProcessID;
				LogMessage(logfile, &log, randomNum, 5);
				fclose(logfile);
				return;
			// Kill DC-06
			case 12:
				if(masterList->dc[5].dcProcessID != 0) {
					killDC(masterList->dc[5].dcProcessID);
					log.pid = masterList->dc[5].dcProcessID;
					LogMessage(logfile, &log, randomNum, 6);
				}
				break;
			// Kill DC-07
			case 14:
				if(masterList->dc[6].dcProcessID != 0) {
					killDC(masterList->dc[6].dcProcessID);
					log.pid = masterList->dc[6].dcProcessID;
					LogMessage(logfile, &log, randomNum, 7);
				}
				break;
			// Kill DC-08
			case 16:
				if(masterList->dc[7].dcProcessID != 0) {
					killDC(masterList->dc[7].dcProcessID);
					log.pid = masterList->dc[7].dcProcessID;
					LogMessage(logfile, &log, randomNum, 8);
				}
				break;
			// Kill DC-09
			case 18:
				if(masterList->dc[8].dcProcessID != 0) {
					killDC(masterList->dc[8].dcProcessID);
					log.pid = masterList->dc[8].dcProcessID;
					LogMessage(logfile, &log, randomNum, 9);
				}
				break;
			// Kill DC-10
			case 20:
				if(masterList->dc[9].dcProcessID != 0) {
					killDC(masterList->dc[9].dcProcessID);
					log.pid = masterList->dc[9].dcProcessID;
					LogMessage(logfile, &log, randomNum, 10);
				}
				break;
		}
	}
	fclose(logfile);
	return;
} 

// FUNCTION: 	int checkMessageQueueExists(MasterList* masterList)
// DESCRIPTION: This function checks if the message queue still exists
// PARAMETERS:  MasterList* masterList : pointer to the masterlist in shared memory containing the message queue ID
// RETURNS: 	Returns 0 if the message queue doesn't exist and 1 if it does
int checkMessageQueueExists(MasterList* masterList) {
    // Check if queue exists
	if(msgctl(masterList->msgQueueID, IPC_STAT, NULL) == -1) {
		if (errno == ENOENT) {
			return 0;
		}
	}
	return 1;
}

// FUNCTION: 	void killDC(int DCtoKill)
// DESCRIPTION: This function kills the specified DC by sending it a SIGHUP signal
// PARAMETERS:  int DCtoKill: ID of the DC set for execution
// RETURNS: 	None
void killDC(int DCtoKill) {
	if(kill(DCtoKill, SIGHUP) == -1) {
		perror("KILL Error:");
	}
}

// FUNCTION: 	void killMessageQueue(int msgQueueID)
// DESCRIPTION: This function kills the specified message queue
// PARAMETERS:  int msgQueueID: Id of the message queue set for execution
// RETURNS: 	None
void killMessageQueue(int msgQueueID) {
	if(msgctl(msgQueueID, IPC_RMID, NULL) == -1) {
		perror("MSGCCTL Error:");
	}
}

// FUNCTION: 	void LogMessage(FILE* logfile, Log* log, int action, int DCID)
// DESCRIPTION: This function logs events to the specified logging file
// PARAMETERS:  FILE* logfile: Logfile to write to
//				Log* log: Log struct with the information to be written
// 				int action: Action performed
//				int DCID: DC killed
// RETURNS: 	None
void LogMessage(FILE* logfile, Log* log, int action, int DCID) {
	char formattedTime[TIME_LENGTH];
	strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", log->timestamp);

	// DC Killed
	if(action != 10 && action != 17) {
		fprintf(logfile, "[%s] : WOD Action %d - DC-%d [%d] TERMINATED\n", formattedTime, action, DCID, log->pid);
	} 
	// Message queue Killed
	else  
	{
		fprintf(logfile, "[%s] : DC Deleted the msgQ - the DR/DCs can't talk anymore - exiting\n", formattedTime);
	}
}