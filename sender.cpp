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


/* README:
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

    string fname = "keyfile.txt";       //string fname
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
    shmid = shmget(key,SHARED_MEMORY_CHUNK_SIZE, S_IRUSR | S_IWUSR);

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

    //get the messeage queue ID
    msqid = msgget(key, S_IRUSR | S_IWUSR);

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
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
    /* TODO: Detach from shared memory */


    /* Deallocate the memory segment */
    if(shmctl (shmid, IPC_RMID, 0) < 0)
    {
        perror("shmctl");
        exit(-1);
    }

    else {
        cout << "mem deallocated\n";
    }

}

/**
 * The main send function
 * @param fileName - the name of the file
 * @return - the number of bytes sent
 */
unsigned long sendFile(const char* fileName)
{

    cout << "in sendFile\n";

    char buffer[SHARED_MEMORY_CHUNK_SIZE];
    int counter = 0;
    int fileSize;

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
    fseek (fp , 0 , SEEK_END);              //find the end of the file
    fileSize = ftell (fp);                  //get the size
    rewind (fp);                            //bring ptr back to beginning of file

    char temp [fileSize];                   //creation of char array to store whole textFile
    fread(temp,1,fileSize,fp);              //read the whole file
    string str = string(temp);              //convert to string for ease of use
    fclose(fp);                             //done with file

    //section off the temp[fileSize] into smaller full 1000byte blocks
    for(int i=0; i< fileSize/SHARED_MEMORY_CHUNK_SIZE; i++ ){
        string s1 = str.substr(i*SHARED_MEMORY_CHUNK_SIZE,SHARED_MEMORY_CHUNK_SIZE);
        text.push_back(s1);
    }

    //this is for last block which isnt full
    int last = fileSize/SHARED_MEMORY_CHUNK_SIZE;       //get the div of fileSize/1000
    last = last* SHARED_MEMORY_CHUNK_SIZE;              //multiply last * 1000
    string s1 = str.substr(last,fileSize-last);         //use last as starting position, end at the (fileSize-last) should be end of array
    text.push_back(s1);                                 //push last block into vector

    cout << "before copy to shared mem\n";

    char* shared_memory = (char*) shmat (shmid, NULL, 0);
    string str2 = text.at(0);
    for (int i=0; i< str2.size(); i++){
        *(shared_memory +i) = str2.at(i);
        cout <<  *(shared_memory +i);
    }

    for (int i=0; i< 1; i++){


        sndMsg.mtype = SENDER_DATA_TYPE;                //set mtype to Sender Data
        sndMsg.size = text.at(i).size();                //set msgSize

        if(msgsnd(msqid, &sndMsg, sizeof(message) - sizeof(long), 0) <0){
            perror("msgsend");
            exit(-1);
        }

        //-------------- for debug purposes ---------------------

        cout << "package sent: " << endl;
        cout << "mType: " << sndMsg.mtype << endl;
        cout << "msgSize: " << sndMsg.size << endl;

        if(msgrcv(msqid,&rcvMsg, sizeof(ackMessage)-sizeof(long), RECV_DONE_TYPE, 0) < 0){
            perror("msgrcv");
            exit(-1);
        }

        else {
            cout << "confirmed msg received\n";
            numBytesSent+= sndMsg.size;

        }

    }


    //reached end
    //sending last message with size of 0

    sndMsg.mtype = SENDER_DATA_TYPE;            //set mtype Send
    sndMsg.size = 0;                            //set size = 0 (END OF SENDING!)

    if(msgsnd(msqid, &sndMsg, sizeof(message) - sizeof(long), 0) <0){
        perror("msgsend");
        exit(-1);
    }

    return numBytesSent;

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



    /** TODO: once we are out of the above loop, we have finished sending the file.
      * Lets tell the receiver that we have nothing more to send. We will do this by
      * sending a message of type SENDER_DATA_TYPE with size field set to 0.
      */

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
    fname = (string)fileName;
    fileNameMsg nameMsg;

    char fname2 [fname.size()+1];                           // convert string to char array
    strcpy(fname2,fname.c_str());

    for (int i=0; i<= fileNameSize; i++){                   // copying chars from fname2 to nameMsg.fileName
        nameMsg.fileName[i] = fname2[i];
    }

    nameMsg.mtype = FILE_NAME_TRANSFER_TYPE;                // setting mtype to 3


    cout << "fname: " <<string(nameMsg.fileName) << endl;


    cout << "before send\n";
    if(msgsnd(msqid, &nameMsg, sizeof(fileNameMsg) - sizeof(long), 0) <0){
        perror("msgsend");
        exit(-1);
    }

    else {
        cout << "after sent fname\n";
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


    /* Connect to shared memory and the message queue */
    init(shmid, msqid, sharedMemPtr);

    /* Send the name of the file */
    sendFileName(argv[1]);


    cout << "fileName sent, back in main now: \n";

    /* Send the file */
    fprintf(stderr, "The number of bytes sent is %lu\n", sendFile(argv[1]));

    /* Cleanup */
    cleanUp(shmid, msqid, sharedMemPtr);

    return 0;
}
