#include "Config.h"

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
	struct Letter *letterLocalSend = letter;

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
	int fdPtr = open("dataPOs.txt", O_WRONLY | O_APPEND);
	int copy_desc = dup(fdPtr);
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

	pipe(fd[totalPOs]);
	pipe(fd_ack[totalPOs]);

	if (getpid() == parentPID)
	{
        int childPIDTemp = fork();
		if (childPIDTemp == 0)
		{

			childPO = pos[totalPOs];
			childIDS_PINCODE[childIDS_PINCODE_ctr][0] = getpid();
			childIDS_PINCODE[childIDS_PINCODE_ctr][1] = childPO.areaCode;
			childIDS_PINCODE_ctr++;

			while (1)
				;
		}
		else
		{
            sleep(2);
			childPO = pos[totalPOs];
			childIDS_PINCODE[childIDS_PINCODE_ctr][0] = childPIDTemp;
			childIDS_PINCODE[childIDS_PINCODE_ctr][1] = childPO.areaCode;
			childIDS_PINCODE_ctr++;
			totalPOs++;

		}
	}
    
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

					sprintf(toSendLetterToPC, "%d", to);
					strcat(toSendLetterFull, toSendLetterToPC);

					strcat(toSendLetterFull, ",");

					sprintf(toSendLetterFromPC, "%d", from);
					strcat(toSendLetterFull, toSendLetterFromPC);

					strcat(toSendLetterFull, ",");

					strcat(toSendLetterFull, "SENT: ");
					write(fd[i][WRITE], toSendLetterFull, 200);

					letterPIPEcheck[letterPIPEcheck_ctr] = i;

					letterPIPEcheck_ctr++;
					//START: Thread Implementation for each letter received
					static struct Letter letter;
					strcpy(letter.nameL, name);
                    
					int tempPosSend = totalSizeString(letter.nameL);
					tempPosSend--;
					memmove(&letter.nameL[tempPosSend], &letter.nameL[tempPosSend + 1], strlen(letter.nameL) - tempPosSend);
                    
					letter.from = from;
					letter.to = to;
					strcpy(letter.status, "SENT: ");
                   
					int err = pthread_create(&threadID, NULL, thread_test_for_eachLetterReceived, (void *)&letter);
					if (err != 0)
						printf("cant create thread: %s\n", strerror(err));
					//END: Thread Implementation for each letter received
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

					char buffer[200];
					close(fd[i][WRITE]);
					read(fd[i][READ], buffer, 200);
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

					close(fd_ack[i][READ]);
					write(fd_ack[i][WRITE], "Received", 10);
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

void lettersDelivered()
{
	int inputAreaCode;
	printf("Please provide the area code : ");
	scanf("%d", &inputAreaCode);
	clean_stdin();

	char fullfileName[50]="./Letter_Files/";
	char fileName[11];
	sprintf(fileName, "%d", inputAreaCode);
	strcat(fileName, ".txt");

	strcat(fullfileName, fileName);

	FILE *fileDataRead = fopen(fullfileName, "r");
	if (fileDataRead == NULL)
	{
		printf("Unable to open/read the file. Please try again\n");
		return;
	}

	char operation;
	printf("Please provide an input for Received or Sent or Both : ");
	scanf("%c", &operation);

	char line[200];

	printf("---------------------------------------------------------------------\n");
	while (fgets(line, sizeof(line), fileDataRead))
	{
		char *token;
		token = strtok(line, ":");

		if (operation == 'R')
		{
			if (strcmp(token, "RECEIVED") == 0)
			{
				printf("Mode: %s || ", token);
			}
			else
			{
				continue;
			}
		}
		else if (operation == 'S')
		{
			if (strcmp(token, "SENT") == 0)
			{
				printf("Mode: %s || ", token);
			}
			else
			{
				continue;
			}
		}
		else
		{
			printf("Mode: %s || ", token);
		}

		token = strtok(NULL, ",");
		printf("Name: %s || ", token);
		token = strtok(NULL, ",");
		printf("To: %s || ", token);
		token = strtok(NULL, ",");
		printf("From: %s",token);
	}
	printf("--------------------------------------------------------------------\n");
}