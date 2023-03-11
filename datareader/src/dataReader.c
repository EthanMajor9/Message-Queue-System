#include "../inc/dataReader.h"

int main(int argc, char* argv[]) 
{
	MasterList *masterList;
	Message message;
	int qID = 0;
	key_t msgQueueKey;
	int shmID = 0;
	key_t shmKey;
	int running = 1;
	Log log;
	FILE* logfile = NULL;

	msgQueueKey = ftok(".",  16535);
	shmKey = ftok(".",  16535);

	qID = checkMessageQueueExists(msgQueueKey);
	shmID = checkSharedMemExists(shmKey);
	
	printf("Message Queue ID: %d\nShared mem ID: %d\n", qID, shmID);

	// Attach shared mem to masterList
	if((masterList = shmat(shmID, NULL, 0)) == (void*) -1) {
		perror("SHMAT error:");
		exit(-1);
	}

	// Init the master list
	masterList->msgQueueID = qID;
	masterList->numberOfDCs = 0;

	for (int i = 0; i < 10; i++) {
		masterList->dc[i].dcProcessID = 0;
		masterList->dc[i].lastTimeHeardFrom = 0;
	}

	sleep(15);

	// Open logfile
	logfile = fopen(logfilepath, "w");

	if(logfile == NULL) {
		perror("fopen");
		exit(-1);
	}

	// Start processing messages
	processMessages(masterList, logfile);

	// Detach the shared memory from the masterlist
	if (shmdt(masterList) == -1) {
        perror("shmdt");
        exit(1);
    }

	// Deallocate the message queue and shared memory
	if (msgctl(qID, IPC_RMID, NULL) == -1 || shmctl(shmID, IPC_RMID, NULL) == -1) {
		perror("msgctl");
	}

	// Close log file
	fclose(logfile);
}



int checkMessageQueueExists(key_t msgQueueKey) {
	int qID;
	// Check for msg queue exist
	if((qID = msgget(msgQueueKey, 0)) == -1) {
		// Create message queue
		qID = msgget(msgQueueKey, (IPC_CREAT | 0660));

		// Check for any errors when creating message queue
		if(qID == -1) {
			perror("Error creating message queue:");
			exit(-1);
		}
	}
	return qID;
}


int checkSharedMemExists(key_t sharedMemKey) {
	int shmID;
	// Check for shared mem existence
	if((shmID = shmget(sharedMemKey, sizeof(MasterList), 0)) == -1) {	
		// Allocate shared mem segment
		shmID = shmget(sharedMemKey, sizeof(MasterList), (IPC_CREAT | 0660));

		// Check for allocation errors
		if (shmID == -1) {
			perror("Error creating message queue:");
			exit(-1);
		}
	}
	return shmID;
}


void LogMessage(FILE* logfile, Log* log) {
	char formattedTime[TIME_LENGTH];
	// Format the time
	strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", log->timestamp);
	// Log to the file
	fprintf(logfile, "[%s] : DC-%d [%d] %s - %s - %d (%s)\n", formattedTime, log->entryNum, log->pid, log->operation,log->statusmsg, log->status, log->statusmsg);
}

Log createLogEntry(int entryNum, int pid, int status, char* statusMsg, char* operation) {
    Log log;
	// Fill the log struct
    log.entryNum = entryNum;
    log.pid = pid;
    log.status = status;
	time_t current_time = time(NULL);
    log.timestamp = localtime(&current_time);
    log.statusmsg = statusMsg;
    log.operation = operation;
    return log;
}


void removeEntryFromMasterList(MasterList* masterList, int index) {
    for (int j = index; j < MAX_DC_ROLES - 1; j++) {
        masterList->dc[j] = masterList->dc[j + 1];
    }
    memset(&masterList->dc[MAX_DC_ROLES - 1], 0, sizeof(DCInfo));
}

void checkAndRemoveTimedOutEntries(MasterList* masterList, Message message, FILE* logfile) {
	// Iterate over all the DC's in the master list
	for (int i = 0; i < MAX_DC_ROLES; i++) {
		// Check if the DC process ID is not 0, which means there is a DC at this position in the master list
		if (masterList->dc[i].dcProcessID != 0) {
			// Check if the time elapsed since the last time this DC was heard from is longer than 35 seconds
			if (time(NULL) - masterList->dc[i].lastTimeHeardFrom >= 35) {
				Log log = createLogEntry(i, message.pid, message.status, message.statusMsg, "has gone OFFLINE");
				LogMessage(logfile, &log);
				removeEntryFromMasterList(masterList, i);
				masterList->numberOfDCs--;
				i--;
			}
		}
	}
}

void updateOrAddDCMessage(MasterList* masterList, Message message, FILE* logfile) {
    for (int i = 0; i < MAX_DC_ROLES; i++) {
        if(message.pid == masterList->dc[i].dcProcessID) {
            masterList->dc[i].lastTimeHeardFrom = time(NULL);
            Log log = createLogEntry(i, message.pid, message.status, message.statusMsg, "updated in the master list");
            LogMessage(logfile, &log);
            return;
        } else if (masterList->dc[i].dcProcessID == 0) {
            masterList->numberOfDCs++;
            masterList->dc[i].dcProcessID = message.pid;
            masterList->dc[i].lastTimeHeardFrom = time(NULL);
			printf("Added to masterlist\n");
            Log log = createLogEntry(i, message.pid, message.status, message.statusMsg, "added to the master list");
            LogMessage(logfile, &log);
            return;
        }
    }
}

void processStatus6Message(MasterList* masterList, Message message, FILE* logfile) {
	// Iterate through all DC's
    for (int i = 0; i < MAX_DC_ROLES; i++) {
		// Check if the offline DC matches the current DC in the list
        if (masterList->dc[i].dcProcessID == message.pid) {
            Log log = createLogEntry(i, message.pid, message.status, message.statusMsg, "removed from the master list");
            LogMessage(logfile, &log);
            removeEntryFromMasterList(masterList, i);
            masterList->numberOfDCs--;
            break;
        }
    }
}

void processMessages(MasterList* masterList, FILE* logfile) {
	int running = 1;
    Message message;
    while (running) {
		// Receive a message from the message queue
        if (msgrcv(masterList->msgQueueID, &message, (sizeof(message) - sizeof(long)), 0, 0) == -1) {
            if (errno != ENOMSG) {
                perror("msgrcv");
                exit(-1);
            }
        }

		// Check if the machine has gone offline
        if (message.status == 6) {
            processStatus6Message(masterList, message, logfile);
        } else {
            updateOrAddDCMessage(masterList, message, logfile);
        }

		// Check the masterlist for any entries that have timed out
        checkAndRemoveTimedOutEntries(masterList, message, logfile);

		// Check if any DC's are currently running
        if (masterList->numberOfDCs == 0) {
            running = 0;
        } else {
			sleep(1.5);
		}
    }
}