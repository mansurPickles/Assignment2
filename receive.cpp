#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "msg.h"
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <iostream>
#include <cstring>
#include <fstream>

using namespace std;


/* README:
 * to run: ./receive
*/

int debug =1;

/* For the message struct */

/* The size of the shared memory segment */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr = NULL;


/**
 * The function for receiving the name of the file
 * @return - the name of the file received from the sender
 */
string recvFileName()
{
    /* The file name received from the sender */
    string fileName;


    fileNameMsg fname;

    cout << "waiting to receive\n";
    if(msgrcv(msqid,&fname, sizeof(fileNameMsg)-sizeof(long), FILE_NAME_TRANSFER_TYPE, 0) < 0){
        perror("msgrcv");
        exit(-1);
    }

    fileName = string (fname.fileName);

    /* TODO: declare an instance of the fileNameMsg struct to be
     * used for holding the message received from the sender.
         */

        /* TODO: Receive the file name using msgrcv() */

    /* TODO: return the received file name */
        cout << "after receive\n";
        cout << "fname: " << fileName << endl;
        return fileName;
}
 /**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */
void init(int& shmid, int& msqid, void*& sharedMemPtr)
{

    /* TODO:
        1. Create a file called keyfile.txt containing string "Hello world" (you may do
        so manually or from the code).
    2. Use ftok("keyfile.txt", 'a') in order to generate the key.
    3. Use will use this key in the TODO's below. Use the same key for the queue
       and the shared memory segment. This also serves to illustrate the difference
       between the key and the id used in message queues and shared memory. The key is
       like the file name and the id is like the file object.  Every System V object
       on the system has a unique id, but different objects may have the same key.
    */


    /* TODO: Allocate a shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE. */

    /* TODO: Attach to the shared memory */

    /* TODO: Create a message queue */

    /* TODO: Store the IDs and the pointer to the shared memory region in the corresponding parameters */

    //2
    string fname = "keyfile.txt";       //string fname
    char fname2 [fname.size()+1];       //convert string to char array
    strcpy(fname2,fname.c_str());

    key_t key = ftok(fname2,'a');       //unique key using fname2 and 'a' as ID

    /* Make sure the key generation succeeded */
    if(key < 0)
    {
        perror("ftok");
        exit(-1);
    }

    cout << "key: " << key << endl;

    //create shmid using key, 1000 block size, read/write, create if not made, exit if already made
    shmid = shmget(key,SHARED_MEMORY_CHUNK_SIZE, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL);

    //error check shmget
    if(shmid < 0)
    {
        perror("shmget failed");
        exit(-1);
    }

    else {
        cout << "mem allocated: " << shmid << endl;
    }

    //create pointer to start of mem segment
    char* shared_memory = (char*) shmat (shmid, NULL, 0);
    sharedMemPtr = (void*)shared_memory;

    // -------------- this is just for debug, deallocating --------------



    //-----------------  creation of message queue   ---------------------------


    cout << "key: " << key << endl;

    /* Make sure the key generation succeeded */
    if(key < 0)
    {
        perror("ftok");
        exit(-1);
    }

    //get message queue with read and write priv. If does not exist make one
    msqid = msgget(key, S_IRUSR | S_IWUSR | IPC_CREAT);

    if(msqid < 0)
    {
        perror("msgget failed");
        exit(-1);
    }

    else {
        cout << "msg queue allocated: " << msqid << endl;

    }

}


/**
 * The main loop
 * @param fileName - the name of the file received from the sender.
 * @return - the number of bytes received
 */
unsigned long mainLoop(const char* fileName)
{
    cout << "in receive mainloop\n";


    /* The size of the message received from the sender */
    int msgSize = -1;

    /* The number of bytes received */
    int numBytesRecv = 0;

    /* The string representing the file name received from the sender */
    string recvFileNameStr = fileName;

    /* TODO: append __recv to the end of file name */

    recvFileNameStr+="__recv";

    /* Open the file for writing */
    FILE* fp = fopen(recvFileNameStr.c_str(), "w");

    /* Error checks */
    if(!fp)
    {
        perror("fopen");
        exit(-1);
    }


    /* Keep receiving until the sender sets the size to 0, indicating that
     * there is no more data to send.
     */
    while(msgSize != 0)
    {
        cout << "msgSize!=0 first loop\n";


//        return 0;
        /* TODO: Receive the message and get the value of the size field. The message will be of
         * of type SENDER_DATA_TYPE. That is, a message that is an instance of the message struct with
         * mtype field set to SENDER_DATA_TYPE (the macro SENDER_DATA_TYPE is defined in
         * msg.h).  If the size field of the message is not 0, then we copy that many bytes from
         * the shared memory segment to the file. Otherwise, if 0, then we close the file
         * and exit.
         *
         * NOTE: the received file will always be saved into the file called
         * <ORIGINAL FILENAME__recv>. For example, if the name of the original
         * file is song.mp3, the name of the received file is going to be song.mp3__recv.
         */

        /* If the sender is not telling us that we are done, then get to work */
        message msg;
        ackMessage confirm;

        if(msgSize != 0)
        {
            /* TODO: record the number of bytes received */

            msgSize = msg.size;
            numBytesRecv = msgSize;

            /* Save into the file the data in shared memory (that was put there
             *  by the sender)
                         */

            if(msgrcv(msqid,&msg, sizeof(message)-sizeof(long), SENDER_DATA_TYPE, 0) < 0){
                perror("msgrcv");
                exit(-1);
            }

            else {
                cout << "message receive\n";
                confirm.mtype = RECV_DONE_TYPE;
                char temp [msgSize];                   //creation of char array to store whole textFile
                int walker = 0;

                return 0;

                //*********************************issue here******************************
                char *memPtr = (char*) & sharedMemPtr;

                for (int i=0; i< msgSize; i++){
                    temp[i] = *(memPtr+i);
                }

                cout << string(temp) << endl;


                fwrite(temp,sizeof(char), msgSize, fp);
            }

            /* TODO: Tell the sender that we are ready for the next set of bytes.
             * I.e., send a message of type RECV_DONE_TYPE. That is, a message
             * of type ackMessage with mtype field set to RECV_DONE_TYPE.
             */

            if(msgsnd(msqid,&confirm,sizeof(ackMessage)-sizeof(long),  0) < 0){
                perror("confirm failed\n");
            }
        }
        /* We are done */
        else
        {
            /* Close the file */
            fclose(fp);
        }
    }

    return numBytesRecv;
}



/**
 * Performs cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
    /* TODO: Detach from shared memory */

    /* TODO: Deallocate the shared memory segment */

    /* TODO: Deallocate the message queue */

    /* Deallocate the memory segment */

    // Deallocate Shared Memory
    if(shmctl (shmid, IPC_RMID, 0) < 0)
    {
        perror("shmctl");
    }

    else {
        cout << "mem deallocated\n";
    }

    // Deallocate Message Queue

    if (msgctl(msqid, IPC_RMID, NULL) < 0){
        perror("msgctl");

    }

    else {
        cout << "msg queue deallocated\n";
    }

}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */
void ctrlCSignal(int signal)
{
    /* Free system V resources */
    cleanUp(shmid, msqid, sharedMemPtr);
}

int main(int argc, char** argv)
{

    /* TODO: Install a signal handler (see signaldemo.cpp sample file).
     * If user presses Ctrl-c, your program should delete the message
     * queue and the shared memory segment before exiting. You may add
     * the cleaning functionality in ctrlCSignal().
     */

    /* Initialize */
    init(shmid, msqid, sharedMemPtr);

    /* Receive the file name from the sender */
    string fileName = recvFileName();


    //---------------HERE DEBUGGING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!--------------------------------------------------------------------
    cout << "received fname\n";
//    cleanUp(shmid,msqid, sharedMemPtr);
//    return 0;

    //---------------HERE DEBUGGING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!--------------------------------------------------------------------


    /* Go to the main loop */
    fprintf(stderr, "The number of bytes received is: %lu\n", mainLoop(fileName.c_str()));

    /* TODO: Detach from shared memory segment, and deallocate shared memory
     * and message queue (i.e. call cleanup)
     */

        cleanUp(shmid,msqid, sharedMemPtr);


    return 0;
}
