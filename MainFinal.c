#include "FunctionBody.h"

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
	char line[200];

	while (fgets(line, sizeof(line), fp))
	{
		
		pipe(fd[totalPOs]);
		pipe(fd_ack[totalPOs]);
		char *token;
		token = strtok(line, ",");

		struct PO childPO;
		strcpy(childPO.name, token);

		token = strtok(NULL, ",");
		strcpy(childPO.area, token);

		token = strtok(NULL, ",");
		childPO.areaCode = atoi(token);

		pos[totalPOs] = childPO;
		totalPOs++;
	}

	for (int i = 0; i < totalPOs; i++)
	{
		childPID = fork();
		if (childPID == 0)
		{
			childPO = pos[i];

			childIDS_PINCODE[childIDS_PINCODE_ctr][0] = getpid();
			childIDS_PINCODE[childIDS_PINCODE_ctr][1] = childPO.areaCode;

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
		sleep(3);
		int input;
		while (1)
		{
			printf("\e[1;1H\e[2J");
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
			else if (input == 3)
			{
				lettersDelivered();
			}
			else
			{
				printf("Closing the program\n");
				pthread_exit(0);
			}
			sleep(3);
		}
	}
	fclose(fp);
}
