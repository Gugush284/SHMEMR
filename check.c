#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/sem.h>
#include <string.h>

int main(int argc, char *argv[])
{
	struct message
    	{ 
        	int package;
        	int pack;
        	char buff[100];
        	int key;
    	} *array;
    	
    	union semun {
      	int val;                  /* value for SETVAL */
      	struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
      	unsigned short *array;    /* array for GETALL, SETALL */
      	struct seminfo *__buf;    /* buffer for IPC_INFO */
	} arg; 
    	 
	struct sembuf sbuf;
    	struct sembuf * mybuf = &sbuf;     	
    	
    	int shmid;        
    	char path[] = "Sender.c"; 
    	char pathname[] = "read.c";
    	key_t key;     
    	int semid; 
    	
    	printf ("CONNECTING\n");
    	if((key = ftok(path,0)) < 0)
    	{
        	printf("Can\'t generate key\n");
        	exit(-1);
    	}
    	
    	if(
    	(semid = semget(key, 9, 0666 | IPC_CREAT))
    	 < 0)
    	{
		printf("Can\'t get semid\n");
		exit(-1);
	}
    	 
    	if((key = ftok(pathname,0)) < 0)
    	{
        	printf("Can\'t generate key\n");
        	exit(-1);
    	}
    
    	if(
    	(shmid = shmget(key, sizeof(struct message), 0666|IPC_CREAT))
    	 < 0)
    	{
        	if(errno != EEXIST)
        	{
        	    printf("Can\'t create shared memory\n");
        	    exit(-1);
        	} 
    	}

    	if(
    	(array = (struct message *)shmat(shmid, NULL, 0)) == 
    	(struct message *)(-1))
    	{
        printf("Can't attach shared memory\n");
        exit(-1);
   	}
   	
   	int flag = 1;
	int check, i;
	i = 0;
	while (flag)
	{
		check = semctl(semid, i, GETVAL);
		printf ("%d\n", check);
		
   		if (check  == -1)
		{
			printf ("SEM_ERROR\n\n");
			exit (0);
		}
		
		i++;
		if (i == 9) break;
	}
   	
  	return 0;
}
