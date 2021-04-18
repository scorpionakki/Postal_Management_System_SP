//LOCKS ARE PENDING
//IDENTIFY PARENT HAS WRITTEN TO WHICH PIPE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

#define READ 0	/* The index of the “read” end of the pipe */
#define WRITE 1 /* The index of the “write” end of the pipe */

int parentPID, childPID; //For determining parent or child
int fd[100][2]; //pipes
int totalPOs = -1; //total POs added. This will keep a count
int childIDS_PINCODE[100][2];
int childIDS_PINCODE_ctr = 0;
int letterPIPEcheck[100]; // For checking on which pipe the letter is going
int letterPIPEcheck_ctr = 0;

struct PO
{
	char name[100];
	char area[100];
	int areaCode;
};

struct PO childPO;
struct PO pos[100];

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
int totalSizeString(const char* str)
{
	char* c = &str[0];
	int size = 0;
	while(*c != '\0')
	{
		c++;
		size++;
	}
	return size;
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
	tempArr[tempSize]=',';

	strcat(tempArr, tempNew->area);
	tempSize = totalSizeString(tempArr);
	tempSize--;
	tempArr[tempSize]=',';
	
	sprintf(tempAreaCode, "%d", tempNew->areaCode);
	strcat(tempArr, tempAreaCode);

	write(copy_desc, "\n", 1);
	write(copy_desc, tempArr, totalSizeString(tempArr));

	pos[totalPOs] = *tempNew;

	if(getpid() == parentPID)
	{
		if (fork() == 0)
		{
			
			childPO = pos[totalPOs];

			printf("Name : %s || Area : %s || Area Code : %d\n", childPO.name, childPO.area, childPO.areaCode);

			childIDS_PINCODE[childIDS_PINCODE_ctr][0] = getpid();
			childIDS_PINCODE[childIDS_PINCODE_ctr][1] = childPO.areaCode;
			childIDS_PINCODE_ctr++;
			// close(fd[totalPOs][WRITE]);
			// read(fd[totalPOs][READ], letter, 100);
			// printf("Letter information passed to Child : %s\n",letter);
			while(1);
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
	switch(signum)
	{
		case SIGUSR1:
			if(getpid() == parentPID) //Parent Block
			{
				int to,from;
				char name[100];
				printf("Please provide your name: ");
				scanf("%s",name);

				printf("Please enter to-pincode: ");
				scanf("%d",&to);

				printf("Please enter from-pincode: ");
				scanf("%d",&from);

				printf("%s %d %d %d\n",name,to,from,childIDS_PINCODE_ctr);
				for(int i=0;i<childIDS_PINCODE_ctr;i++)
				{
					printf("Inside for LOOP\n");
					printf("%d %d\n",childIDS_PINCODE[i][0], childIDS_PINCODE[i][1]);
					if(childIDS_PINCODE[i][1] == to)
					{
						close(fd[i][READ]);
						write(fd[i][WRITE], name, 100);
						letterPIPEcheck[letterPIPEcheck_ctr++] = i;
						printf("Letter Data: %s\n",name);
						kill(childIDS_PINCODE[i][0], SIGUSR2);
						break;
					}
					else
					{
						printf("Condition Checking\n");
					}
				}
			}
			break;
		
		case SIGUSR2:
			if(getpid() != parentPID)
			{
				char buffer[100];
				read(letterPIPEcheck[letterPIPEcheck_ctr], buffer, 100);
				printf("Content read : %s\n", buffer);
			}
	}
		
}

int main()
{
	
	char structPassedToChild[100];
	oldHandlerSigCHILD = signal(SIGCHLD, newHandlerSigCHILD);
	
	if (signal(SIGUSR1, sigusr_handler) == SIG_ERR)	// both parent and child register for SIGUSR1 handler
	{
		printf("Unable to crete handler for SIGUSR1\n");
		exit(-1);
	}

	if (signal(SIGUSR2, sigusr_handler) == SIG_ERR)	// both parent and child register for SIGUSR2 handler
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
	printf("Parent : %d\n", parentPID);
	char line[200];

	while (fgets(line, sizeof(line), fp))
	{
		totalPOs++;
		pipe(fd[totalPOs]);
		char *token;
		token = strtok(line, ",");

		struct PO childPO;
		strcpy(childPO.name, token);
		printf("Name : %s\n", childPO.name);

		token = strtok(NULL, ",");
		strcpy(childPO.area, token);
		printf("Area : %s\n", childPO.area);

		token = strtok(NULL, ",");
		childPO.areaCode = atoi(token);
		printf("Area Code : %d\n", childPO.areaCode);

		pos[totalPOs] = childPO;

		printf("\n");
	}

	printf("Total POs %d\n", totalPOs);

	for (int i = 0; i < totalPOs; i++)
	{
		childPID = fork();
		if (childPID == 0)
		{
			childPO = pos[i];

			printf("Name : %s || Area : %s || Area Code : %d\n", childPO.name, childPO.area, childPO.areaCode);

			childIDS_PINCODE[childIDS_PINCODE_ctr][0] = getpid();
			childIDS_PINCODE[childIDS_PINCODE_ctr][1] = childPO.areaCode;
			printf("CHILDIDS_PINCODE_CTR : %d\n",childIDS_PINCODE_ctr);
			// close(fd[i][READ]);
			// read(fd[i][READ], structPassedToChild, 100);
			// printf("Struct information passed to Child : %s\n",structPassedToChild);
			while(1);
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
		printf("Parent's PID : %d & Current PID : %d\n", parentPID, getpid());
		sleep(10);
		int input;

		while (1)
		{
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
			else if(input == 2)
			{
				kill(parentPID,SIGUSR1);
			}
		}
	}
}
