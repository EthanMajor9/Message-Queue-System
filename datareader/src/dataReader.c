/*
* FILE: dataReader.c
* PROJECT: A-03 The Hoochamacallit System
* FIRST VERSION: 03/11/2023
* PROGRAMMER(s): Ethan Major, Caleb Brown
* DESCRIPTION: This file contains the functions for the data reader utility which is responsible for creating and allocation the message queue and shared memory segemnt,
* as well as maintaining a master list of all DC's
*/
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

	// Generate the msg queue and shared mem keys
	msgQueueKey = ftok(".",  15);
	shmKey = ftok(".",  16535);

	// Check existence/create the queue and shared mem
	qID = checkMessageQueueExists(msgQueueKey);
	shmID = checkSharedMemExists(shmKey);

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

	// Sleep for 15 seconds
	sleep(15);

	// Open logfile
	logfile = fopen(logfilepath, "a");

	if(logfile == NULL) {
		perror("fopen");
		exit(-1);
	}

	// Start processing messages
	processMessages(masterList, logfile);

	// Detach the shared memory from the masterlist
	if (shmdt(masterList) == -1) {
        perror("SHMDT Error:");
    }

	// Deallocate the message queue and shared memory
	if (msgctl(qID, IPC_RMID, NULL) == -1 || shmctl(shmID, IPC_RMID, NULL) == -1) {
		perror("MSGCTL Error:");
	}

	// Close log file
	fclose(logfile);
}


// FUNCTION: 	int checkMessageQueueExists(key_t msgQueueKey)
// DESCRIPTION: Checks if the message queue exists and allocates it if not
// PARAMETERS:  key_t msgQueueKey : key for the message queue
// RETURNS: 	int qID : ID of the message queue
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

// FUNCTION: 	int checkSharedMemExists(key_t sharedMemKey)
// DESCRIPTION: Checks if the shared mem exists and allocates it if not
// PARAMETERS:  key_t sharedMemKey : key for the shared mem segment
// RETURNS: 	int shmID : ID of the shared mem
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

// FUNCTION: 	void LogMessage(FILE* logfile, Log* log)
// DESCRIPTION: Logs the message to the file
// PARAMETERS:  FILE* logfile : File to be written to
//				Log* log : Log struct
// RETURNS: 	Returns a filled log struct with all the information
void LogMessage(FILE* logfile, Log* log) {
	char formattedTime[TIME_LENGTH];
	// Format the time
	strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", log->timestamp);
	// Log to the file
	fprintf(logfile, "[%s] : DC-%d [%d] %s - %s - %d (%s)\n", formattedTime, log->entryNum, log->pid, log->operation,log->statusmsg, log->status, log->statusmsg);
}

// FUNCTION: 	Log createLogEntry(int entryNum, int pid, int status, char* statusMsg, char* operation)
// DESCRIPTION: Fills the log struct with the information passed in 
// PARAMETERS:  int entryNum : 
//				int pid : PID of the DC  
//				int status : Status sent by the DC
//				char* statusMsg : Status message sent by the DC
//				char* operation : Operation performed
// RETURNS: 	Returns a filled log struct with all the information
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

// FUNCTION: 	void removeEntryFromMasterList(MasterList* masterList, int index)
// DESCRIPTION: Checks if a new DC is running and adds it to the master list or updates a currently existing DC in the list
// PARAMETERS:  MasterList* masterList : pointer to the masterlist in shared memory
//				int index : index of the DC to remove
// RETURNS: 	None
void removeEntryFromMasterList(MasterList* masterList, int index) {
	// Iterate through the list and collapse it
    for (int j = index; j < MAX_DC_ROLES - 1; j++) {
        masterList->dc[j] = masterList->dc[j + 1];
    }
    memset(&masterList->dc[MAX_DC_ROLES - 1], 0, sizeof(DCInfo));
}

// FUNCTION: 	void checkAndRemoveTimedOutEntries(MasterList* masterList, Message message, FILE* logfile)
// DESCRIPTION: Checks if a new DC is running and adds it to the master list or updates a currently existing DC in the list
// PARAMETERS:  MasterList* masterList : pointer to the masterlist in shared memory
//				Message message: Message struct 
//				FILE* logfile: file to be written to
// RETURNS: 	None
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

// FUNCTION: 	void updateOrAddDCMessage(MasterList* masterList, Message message, FILE* logfile)
// DESCRIPTION: Checks if a new DC is running and adds it to the master list or updates a currently existing DC in the list
// PARAMETERS:  MasterList* masterList : pointer to the masterlist in shared memory
//				Message message: Message struct 
//				FILE* logfile: file to be written to
// RETURNS: 	None
void updateOrAddDCMessage(MasterList* masterList, Message message, FILE* logfile) {
    for (int i = 0; i < MAX_DC_ROLES; i++) {
		// DC exists in list
        if(message.pid == masterList->dc[i].dcProcessID) {
			// Update time entry
            masterList->dc[i].lastTimeHeardFrom = time(NULL);
			// Log event
            Log log = createLogEntry(i, message.pid, message.status, message.statusMsg, "updated in the master list");
            LogMessage(logfile, &log);
            return;
        } else if (masterList->dc[i].dcProcessID == 0) { // New DC
			// Incrememnt number of DCs
            masterList->numberOfDCs++;
			// Add DC to list
            masterList->dc[i].dcProcessID = message.pid;
            masterList->dc[i].lastTimeHeardFrom = time(NULL);
			// Log event
            Log log = createLogEntry(i, message.pid, message.status, message.statusMsg, "added to the master list");
            LogMessage(logfile, &log);
            return;
        }
    }
}

// FUNCTION: 	void processStatus6Message(MasterList* masterList, Message message, FILE* logfile)
// DESCRIPTION: Processes the status 6 messages indicating a machine has gone offline, it removes the DC from the list and collapses it
// PARAMETERS:  MasterList* masterList : pointer to the masterlist in shared memory
//				Message message: Message struct 
//				FILE* logfile: file to be written to
// RETURNS: 	None
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

// FUNCTION: 	void processMessages(MasterList* masterList, FILE* logfile)
// DESCRIPTION: this function acts as the main processing function for the reader utility and is responsible for receiving the messages
// PARAMETERS:  MasterList* masterList : pointer to the masterlist in shared memory
//				FILE* logfile: file to be written to
// RETURNS: 	None
void processMessages(MasterList* masterList, FILE* logfile) {
	int running = 1;
	int isFirstIt = 1;
    Message message = {0};
    while (running) {
		// Receive a message from the message queue
        if (msgrcv(masterList->msgQueueID, &message, (sizeof(message) - sizeof(long)), 0, IPC_NOWAIT) == -1) {
            if (errno != ENOMSG) {
                perror("MSGRCV Error:");
                exit(-1);
            }
        } else {
			isFirstIt--;
            // Check if the machine has gone offline
            if (message.status == 6) {
                processStatus6Message(masterList, message, logfile);
            } else {
                updateOrAddDCMessage(masterList, message, logfile);
            }
        }

		// Check the masterlist for any entries that have timed out
        checkAndRemoveTimedOutEntries(masterList, message, logfile);

		// Check if any DC's are currently running
        if (masterList->numberOfDCs == 0 && isFirstIt <= 0) {
            running = 0;
        } else {
			sleep(1.5);
		}
    }
}