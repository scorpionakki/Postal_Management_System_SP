//LOCKS ARE PENDING WHILE WRITING IN THE FILE

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
int totalPOs = -1;		 //total POs added. This will keep a count
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

//For cleaing the stdin
void clean_stdin(void)
{
	int c;
	do
	{
		c = getchar();
	} while (c != '\n' && c != EOF);
}

//For Calculating the size of string(actual not the max)
int totalSizeString(char *str)
{
	char *c = str;
	int size = 0;
	while (*c != '\0')
	{
		c++;
		size++;
	}
	return size;
}

//For Thread Implementation
void *thread_test_for_eachLetterReceived(void *letter)
{
	// //RECEIVED FILE WRITE
	// struct Letter *letterLocal = letter;
	// char folderFileRec[] = "Letter_Files/";
	// char tempTo[7];
	// char tempFrom[7];
	// sprintf(tempTo, "%d", letterLocal->to);
	// sprintf(tempFrom, "%d", letterLocal->from);

	// strcat(folderFileRec, tempTo);
	// strcat(folderFileRec, ".txt");

	// FILE *fPtrRec;
	// fPtrRec = fopen(folderFileRec, "a+");

	// char tempArr[1000] = "RECEIVED: ";

	// strcat(tempArr, letterLocal->nameL);
	// int tempPos = totalSizeString(tempArr);
	// tempPos--;
	// memmove(&tempArr[tempPos], &tempArr[tempPos + 1], strlen(tempArr) - tempPos);
	// strcat(tempArr, ",");
	// strcat(tempArr, tempTo);
	// strcat(tempArr, ",");
	// strcat(tempArr, tempFrom);

	// sem_wait(&semaphore);
	// fprintf(fPtrRec, "%s\n",tempArr);
	// sem_post(&semaphore);
	// fclose(fPtrRec);

	//SEND FILE WRITE


	struct Letter *letterLocalSend = letter;
	
	printf("%s %d %d %s!!\n",letterLocalSend->nameL,letterLocalSend->to,letterLocalSend->from,letterLocalSend->status);

	char folderFileSend[] = "Letter_Files/";
	char tempToSend[7];
	char tempFromSend[7];
	sprintf(tempToSend, "%d", letterLocalSend->to);
	sprintf(tempFromSend, "%d", letterLocalSend->from);


	if (strcmp(letterLocalSend->status, "SENT: ") == 0)
	{
		strcat(folderFileSend, tempFromSend);
	}
	else if (strcmp(letterLocalSend->status, "RECEIVED: ") == 0)
	{
		strcat(folderFileSend, tempToSend);
	}
	strcat(folderFileSend, ".txt");

	FILE *fPtr;
	fPtr = fopen(folderFileSend, "a+");
	char tempArrSend[1000];
	strcpy(tempArrSend, letterLocalSend->status);

	strcat(tempArrSend, letterLocalSend->nameL);
	// int tempPosSend = totalSizeString(tempArrSend);
	// tempPosSend--;
	// memmove(&tempArrSend[tempPosSend], &tempArrSend[tempPosSend + 1], strlen(tempArrSend) - tempPosSend);
	strcat(tempArrSend, ",");
	strcat(tempArrSend, tempToSend);
	strcat(tempArrSend, ",");
	strcat(tempArrSend, tempFromSend);

	sem_wait(&semaphore);
	fprintf(fPtr, "%s\n", tempArrSend);
	sem_post(&semaphore);

	fclose(fPtr);
	pthread_exit(0);
}

//For Input 1 - Add a new PO
void addNewPO()
{
	int fd = open("tempTextFile.txt", O_WRONLY | O_APPEND);
	int copy_desc = dup(fd);
	char tempArr[1000];
	char tempAreaCode[7];
	int tempSize;

	struct PO *tempNew = (struct PO *)malloc(sizeof(struct PO));

	printf("Please provide the Post Office Name : ");
	fgets(tempNew->name, 100, stdin);
	printf("%s\n", tempNew->name);

	printf("Please provide the Post Office Area : ");
	fgets(tempNew->area, 100, stdin);
	printf("%s\n", tempNew->area);

	printf("Please provide the Post Office AreaCode : ");
	scanf("%d", &tempNew->areaCode);
	printf("%d\n", tempNew->areaCode);

	strcpy(tempArr, tempNew->name);
	tempSize = totalSizeString(tempArr);
	tempSize--;
	tempArr[tempSize] = ',';

	strcat(tempArr, tempNew->area);
	tempSize = totalSizeString(tempArr);
	tempSize--;
	tempArr[tempSize] = ',';

	sprintf(tempAreaCode, "%d", tempNew->areaCode);
	strcat(tempArr, tempAreaCode);

	write(copy_desc, "\n", 1);
	write(copy_desc, tempArr, totalSizeString(tempArr));

	pos[totalPOs] = *tempNew;

	if (getpid() == parentPID)
	{
		if (fork() == 0)
		{

			childPO = pos[totalPOs];

			// printf("Name : %s || Area : %s || Area Code : %d\n", childPO.name, childPO.area, childPO.areaCode);

			childIDS_PINCODE[childIDS_PINCODE_ctr][0] = getpid();
			childIDS_PINCODE[childIDS_PINCODE_ctr][1] = childPO.areaCode;
			childIDS_PINCODE_ctr++;
			// close(fd[totalPOs][WRITE]);
			// read(fd[totalPOs][READ], letter, 100);
			// printf("Letter information passed to Child : %s\n",letter);
			while (1)
				;
		}
	}

	totalPOs++;

	printf("The PO is added\n");
}

void (*oldHandlerSigCHILD)();
void newHandlerSigCHILD()
{
	int childPID, status;
	childPID = wait(&status);
	printf("WAIT CALL: Child : %d exited with status : %d\n", childPID, status);
}

void sigusr_handler(int signum)
{
	switch (signum)
	{
	case SIGUSR1:
		if (getpid() == parentPID) //Parent Block
		{
			printf("INTO SIGUSR1 - PARENT\n");
			int to, from;
			char name[100];
			printf("Please provide your name: ");
			fgets(name, 100, stdin);

			printf("Please enter to-pincode: ");
			scanf("%d", &to);

			printf("Please enter from-pincode: ");
			scanf("%d", &from);

			for (int i = 0; i < childIDS_PINCODE_ctr; i++)
			{
				// printf("%d %d\n", childIDS_PINCODE[i][0], childIDS_PINCODE[i][1]);
				if (childIDS_PINCODE[i][1] == to)
				{
					close(fd[i][READ]);
					char toSendLetterFull[200];
					char toSendLetterToPC[7];
					char toSendLetterFromPC[7];

					strcpy(toSendLetterFull, name);
					int tempSize = totalSizeString(toSendLetterFull);
					tempSize--;
					toSendLetterFull[tempSize] = ',';
					// strcat(toSendLetterFull, ",");

					sprintf(toSendLetterToPC, "%d", to);
					strcat(toSendLetterFull, toSendLetterToPC);

					strcat(toSendLetterFull, ",");

					sprintf(toSendLetterFromPC, "%d", from);
					strcat(toSendLetterFull, toSendLetterFromPC);

					strcat(toSendLetterFull,",");

					strcat(toSendLetterFull, "SENT: ");
					write(fd[i][WRITE], toSendLetterFull, 100);

					letterPIPEcheck[letterPIPEcheck_ctr] = i;

					letterPIPEcheck_ctr++;

					// printf("Letter Data: %s\n", name);

					//START: Testing Thread Implementation for each letter received
					static struct Letter letter;
					strcpy(letter.nameL, name);

					int tempPosSend = totalSizeString(letter.nameL);
					tempPosSend--;
					memmove(&letter.nameL[tempPosSend], &letter.nameL[tempPosSend + 1], strlen(letter.nameL) - tempPosSend);

					// printf("LETTER.NAME : %s\n",letter.nameL);
					letter.from = from;
					letter.to = to;
					strcpy(letter.status, "SENT: ");
					int err = pthread_create(&threadID, NULL, thread_test_for_eachLetterReceived, (void *)&letter);
					if (err != 0)
						printf("cant create thread: %s\n", strerror(err));
					//END: Testing

					kill(childIDS_PINCODE[i][0], SIGUSR2);
					break;
				}
			}
		}
		break;

	case SIGUSR2:
		if (getpid() != parentPID)
		{
			for (int i = 0; i <= childIDS_PINCODE_ctr; i++)
			{
				if (getpid() == childIDS_PINCODE[i][0])
				{
					static struct Letter letter;

					char buffer[100];
					close(fd[i][WRITE]);
					read(fd[i][READ], buffer, 100);

					char delim[] = ",";
					char *ptr = strtok(buffer, delim);

					strcpy(letter.nameL, ptr);
					ptr = strtok(NULL, delim);

					letter.to = atoi(ptr);
					ptr = strtok(NULL, delim);

					letter.from = atoi(ptr);
					ptr = strtok(NULL, delim);

					strcpy(letter.status, "RECEIVED: ");
					
					int err = pthread_create(&threadID, NULL, thread_test_for_eachLetterReceived, (void *)&letter);
					if (err != 0)
						printf("cant create thread: %s\n", strerror(err));
					//END: Testing

					close(fd_ack[i][READ]);
					write(fd_ack[i][WRITE], "Received", 10);
					printf("WRITTEN THE DATA NOW SENDING SIGNAL : %d\n",i);
					kill(parentPID, SIGUSR2);
					break;
				}
			}
		}
		else
		{
			char ack[10];
			close(fd_ack[letterPIPEcheck[letterPIPEcheck_ctr]][WRITE]);
			int temp = letterPIPEcheck_ctr - 1;
			printf("WAITING TO READ : %d\n",letterPIPEcheck[temp]);
			read(fd_ack[letterPIPEcheck[temp]][READ], ack, 10);
			if (strcmp(ack, "Received") == 0)
			{
				printf("Acknowledgement of letter\n");
			}
			else
			{
				printf("There is some error\n");
			}
		}
		break;
	}
}

int main()
{
	sem_init(&semaphore, 0, 1);
	char structPassedToChild[100];
	oldHandlerSigCHILD = signal(SIGCHLD, newHandlerSigCHILD);

	if (signal(SIGUSR1, sigusr_handler) == SIG_ERR) // both parent and child register for SIGUSR1 handler
	{
		printf("Unable to crete handler for SIGUSR1\n");
		exit(-1);
	}

	if (signal(SIGUSR2, sigusr_handler) == SIG_ERR) // both parent and child register for SIGUSR2 handler
	{
		printf("Unable to crete handler for SIGUSR2\n");
		exit(-1);
	}

	FILE *fp = fopen("dataPOs.txt", "r");
	if (fp == NULL)
	{
		printf("Unable to open the file\n");
		return -1;
	}
	parentPID = getpid();
	// printf("Parent : %d\n", parentPID);
	char line[200];

	while (fgets(line, sizeof(line), fp))
	{
		totalPOs++;
		pipe(fd[totalPOs]);
		pipe(fd_ack[totalPOs]);
		char *token;
		token = strtok(line, ",");

		struct PO childPO;
		strcpy(childPO.name, token);
		// printf("Name : %s\n", childPO.name);

		token = strtok(NULL, ",");
		strcpy(childPO.area, token);
		// printf("Area : %s\n", childPO.area);

		token = strtok(NULL, ",");
		childPO.areaCode = atoi(token);
		// printf("Area Code : %d\n", childPO.areaCode);

		pos[totalPOs] = childPO;

		// printf("\n");
	}

	// printf("Total POs %d\n", totalPOs);

	for (int i = 0; i < totalPOs; i++)
	{
		childPID = fork();
		if (childPID == 0)
		{
			childPO = pos[i];

			// printf("Name : %s || Area : %s || Area Code : %d\n", childPO.name, childPO.area, childPO.areaCode);

			childIDS_PINCODE[childIDS_PINCODE_ctr][0] = getpid();
			childIDS_PINCODE[childIDS_PINCODE_ctr][1] = childPO.areaCode;
			// printf("CHILDIDS_PINCODE_CTR : %d\n", childIDS_PINCODE_ctr);
			// close(fd[i][READ]);
			// read(fd[i][READ], structPassedToChild, 100);
			// printf("Struct information passed to Child : %s\n",structPassedToChild);
			while (1)
				;
		}
		else
		{
			childPO = pos[i];
			childIDS_PINCODE[childIDS_PINCODE_ctr][0] = childPID;
			childIDS_PINCODE[childIDS_PINCODE_ctr][1] = childPO.areaCode;
			childIDS_PINCODE_ctr++;
		}
	}

	if (getpid() == parentPID)
	{
		// printf("Parent's PID : %d & Current PID : %d\n", parentPID, getpid());
		sleep(10);
		int input;

		for(int i=0;i<totalPOs;i++)
		{
			printf("PID : %d Area Code : %d\n",childIDS_PINCODE[i][0],childIDS_PINCODE[i][1]);
		}
		while (1)
		{
			// printf("\e[1;1H\e[2J");
			printf("-------Welcome to Vadodara Postal System---------\n");
			printf("1. Add a new Post Office\n");
			printf("2. Deliver a letter\n");
			printf("3. Check all the letters delivered\n");
			printf("Your input : ");
			scanf("%d", &input);
			clean_stdin();
			printf("\n");

			if (input == 1)
			{
				addNewPO();
			}
			else if (input == 2)
			{
				kill(parentPID, SIGUSR1);
			}
		}
	}
	fclose(fp);
}
