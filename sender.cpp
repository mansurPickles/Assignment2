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
#include <vector>

using namespace std;


/* README: so there are a bunch of debug statements and repetetive functions in this. Basically I am trying to
 * make sure all these functions work before throwing them all together. How it is right now the sender and
 * receiver are unaware of each other. With the debug set to 1 all these do is make the shmid and
 * msgid get the pointer and then deallocate itself.
 *
 * for this one I also started working on the structure created the filename one and started working on the
 * actual contents of the file. Whats left is to init it the message queue. Finish the send message
 *
 * to run: ./send keyfile.txt
*/


string fname; //global var of file name

int debug = 1;
ifstream f;

/* The size of the shared memory segment */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void* sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the allocated message queue
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



    /* TODO: Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE */
    /* TODO: Attach to the shared memory */
    /* TODO: Attach to the message queue */
    /* Store the IDs and the pointer to the shared memory region in the corresponding function parameters */

    char fname2 [fname.size()+1];       //convert string to char array
    strcpy(fname2,fname.c_str());
    key_t key = ftok(fname2,'a');       //unique key using fname2 and 'a' as ID

    /* Make sure the key generation `ceeded */
    if(key < 0)
    {
        perror("ftok");
        exit(-1);
    }

    cout << "key: " << key << endl;

    //create shmid using key, 1000 block size, read/write
    //REMOVE IPC_CREAT
    shmid = shmget(key,SHARED_MEMORY_CHUNK_SIZE, S_IRUSR | S_IWUSR | IPC_CREAT);

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

    //remove IPC_CREAT after testing
    msqid = msgget(key, S_IRUSR | S_IWUSR | IPC_CREAT);     //dont nead to crate just join

    if(msqid < 0)
    {
        perror("msgget failed");
        exit(-1);
    }


    // -------------- this is just for debug, deallocating --------------
    if (debug==4){

        /* Deallocate the memory segment */
        if(shmctl (shmid, IPC_RMID, 0) < 0)
        {
            perror("shmctl");
        }

        else {
            cout << "mem deallocated\n";
        }


        if (msgctl(msqid, IPC_RMID, NULL) < 0){
            perror("msgctl");

        }

        else {
            cout << "msg queue deallocated\n";
        }
    }



}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
    /* TODO: Detach from shared memory */
}

/**
 * The main send function
 * @param fileName - the name of the file
 * @return - the number of bytes sent
 */
unsigned long sendFile(const char* fileName)
{

    char buffer[SHARED_MEMORY_CHUNK_SIZE];
    int counter = 0;
    int fileSize;
    size_t hello;

    vector<string> text;

    /* A buffer to store message we will send to the receiver. */
    message sndMsg;

    /* A buffer to store message received from the receiver. */
    ackMessage rcvMsg;

    /* The number of bytes sent */
    unsigned long numBytesSent = 0;

    /* Open the file */
    FILE* fp =  fopen(fileName, "r");
//    f.open(fileName);



    if (fp==NULL) {
        fputs ("File error",stderr); exit (-1);
    }



    // obtain file size:
    fseek (fp , 0 , SEEK_END);
    fileSize = ftell (fp);
    rewind (fp);

    char temp [fileSize];                   //creation of char array to store whole textFile
    fread(temp,1,fileSize,fp);              //read the whole file
    string str = string(temp);              //convert to string for ease of use
    fclose(fp);                             //done with file

    //section off into SHARED_MEM chunks
    for(int i=0; i< fileSize/SHARED_MEMORY_CHUNK_SIZE; i++ ){
        string s1 = str.substr(i*SHARED_MEMORY_CHUNK_SIZE,SHARED_MEMORY_CHUNK_SIZE);
        text.push_back(s1);
    }

    //this is for last block which isnt full
    int last = fileSize/SHARED_MEMORY_CHUNK_SIZE;
    last = last* SHARED_MEMORY_CHUNK_SIZE;
    string s1 = str.substr(last,fileSize-last);
    text.push_back(s1);

    for (int i=0; i< text.size(); i++){
    char *memPtr = (char*) & sharedMemPtr;
    strcpy(memPtr,text.at(i).c_str());
    sndMsg.mtype = SENDER_DATA_TYPE;
    sndMsg.size = text.at(i).size();

    if(msgsnd(msqid, &sndMsg, sizeof(message) - sizeof(long), 0) <0){
        perror("msgsend");
        exit(-1);
    }
    cout << "package sent: " << endl;
    cout << sndMsg.mtype << endl;
    cout << sndMsg.size << endl;
    cout << string(memPtr) << endl;
    if(msgrcv(msqid,&rcvMsg, sizeof(ackMessage)-sizeof(long), RECV_DONE_TYPE, 0) < 0){
        perror("msgrcv");
        exit(-1);
    }

    else {
        cout << "waiting\n";
    }




    }


    return 0;


    /* Read the whole file */

    //    while(!feof(fp))
    {
        /* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and
         * store them in shared memory.  fread() will return how many bytes it has
         * actually read. This is important; the last chunk read may be less than
         * SHARED_MEMORY_CHUNK_SIZE. Save the number of bytes that were actually
         * read in numBytesSent to record how many bytes were actually send.
         */



        /* TODO: count the number of bytes sent. */

        /* TODO: Send a message to the receiver telling him that the data is ready
         * to be read (message of type SENDER_DATA_TYPE).
         */

        /* TODO: Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us
         * that he finished saving a chunk of memory.
         */
    }


    /** TODO: once we are out of the above loop, we have finished sending the file.
      * Lets tell the receiver that we have nothing more to send. We will do this by
      * sending a message of type SENDER_DATA_TYPE with size field set to 0.
      */


    /* Close the file */
    //    fclose(fp);

    return numBytesSent;
}

/**
 * Used to send the name of the file to the receiver
 * @param fileName - the name of the file to send
 */
void sendFileName(const char* fileName)
{
    /* Get the length of the file name */
    int fileNameSize = strlen(fileName);

    /* TODO: Make sure the file name does not exceed
     * the maximum buffer size in the fileNameMsg
     * struct. If exceeds, then terminate with an error.
     */

    if (fileNameSize>MAX_FILE_NAME_SIZE){
        perror("fileName exceeded maxsize\n");
        exit (-1);
    }

    fileNameMsg nameMsg;

    char fname2 [fname.size()+1];                           // convert string to char array
    strcpy(fname2,fname.c_str());

    for (int i=0; i<= fileNameSize; i++){                   // copying chars from fname2 to nameMsg.fileName
        nameMsg.fileName[i] = fname2[i];
    }

    nameMsg.mtype = FILE_NAME_TRANSFER_TYPE;                // setting mtype to 3

    if (debug){
        cout << "fname size: " << fileNameSize << endl;
        cout << "fname: " <<string(nameMsg.fileName) << endl;
        cout << "mtype: " << nameMsg.mtype << endl;
    }




    /* TODO: Create an instance of the struct representing the message
     * containing the name of the file.
     */

    /* TODO: Set the message type FILE_NAME_TRANSFER_TYPE */

    /* TODO: Set the file name in the message */

    /* TODO: Send the message using msgsnd */
}


int main(int argc, char* argv[])
{

    /* Check the command line arguments */
    if(argc < 2)
    {
        fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
        exit(-1);
    }


    //only checks the first argument
    else {

        if (debug){
            fname = argv[1];
            cout << "sender works\n";
            cout << fname << endl;
        }

        //checking to see if this works
        //this code does nothing but checks if we are able to create the filename and send the file

        if (debug){
            init(shmid, msqid, sharedMemPtr);

            sendFileName(argv[1]);      //these are repeated down below -- FOR TESTING ONLY
            sendFile(argv[1]);          //these are repeated down below -- FOR TESTING ONLY
        }

        return 0;
    }

    /* Connect to shared memory and the message queue */
//    init(shmid, msqid, sharedMemPtr);

    /* Send the name of the file */
    sendFileName(argv[1]);

    /* Send the file */
    fprintf(stderr, "The number of bytes sent is %lu\n", sendFile(argv[1]));

    /* Cleanup */
    cleanUp(shmid, msqid, sharedMemPtr);

    return 0;
}
