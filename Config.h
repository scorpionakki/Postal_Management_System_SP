#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <pthread.h>
#include <semaphore.h>

#define READ 0	/* The index of the “read” end of the pipe */
#define WRITE 1 /* The index of the “write” end of the pipe */

int parentPID, childPID; //For determining parent or child
int fd[100][2];			 //pipes - Parent Write, Child Read (For Letter Data)
int fd_ack[100][2];		 //pipes - Parent Read, Child Write (For Acknowledgement)
int totalPOs = 0;		 //total POs added. This will keep a count
int childIDS_PINCODE[100][2];
int childIDS_PINCODE_ctr = 0;
int letterPIPEcheck[100]; // For checking on which pipe the letter is going
int letterPIPEcheck_ctr = 0;
int templetterPIPEcheck_ctr = 0; //Doing this because in letterPIPEcheck[letterPIPEcheck_ctr++] = i; It will add data and then letterPIPEcheck_ctr will go +1 i.e. to an empty space in array

pthread_t threadID;
sem_t semaphore;

struct PO
{
	char name[100];
	char area[100];
	int areaCode;
}; //For post office

struct Letter
{
	char nameL[100];
	int to;
	int from;
	char status[50];
}; //For letter

struct PO childPO;	//Temporary child for containing the data before pushing it to array of pos
struct PO pos[100]; //Array of post offices

