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
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>

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
    	
    	//printf ("CONNECTING\n");
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
   	
   	//printf ("TRYING TO ENTER\n");
   	struct sembuf enbuf[4] = 
   	{
   		1, 0, 0,
   		3, 0, 0,
   		5, +1, SEM_UNDO,
   		1, +1, SEM_UNDO
   	};
	if(semop(semid, enbuf, 4)< 0)
	{
		printf("Can\'t wait for condition 1\n");
		shmctl (shmid, IPC_RMID, NULL);
		semctl(semid, 1, IPC_RMID); 
		exit(-1);
	}
	
	//printf ("Making barrier\n");
	struct sembuf chbuf[3] = 
   	{
   		0, -1, 0,
   		0, 1, 0,
   		2, +1, SEM_UNDO
   	};
   	
	if(semop(semid, chbuf, 3)<0)
	{
		printf("Can\'t wait for condition 2\n");
		shmctl (shmid, IPC_RMID, NULL);
		semctl(semid, 1, IPC_RMID); 
		exit(-1);
	}
	
   	//printf ("WORKING\n");
	int w = 1;
	int b;
	char *p;
	char sym;
	int f = 0;
	array->pack = 0;
	array->package = 1;
	array->key = 1;
	
	sbuf.sem_num = 5;
	sbuf.sem_op = -1;
	sbuf.sem_flg = 0;
 	if(semop(semid, mybuf, 1) < 0)
	{
		printf("Can\'t wait for condition 5\n");
		shmctl (shmid, IPC_RMID, NULL);
		semctl(semid, 1, IPC_RMID); 
		exit(-1);
	}	
	while (w)
	{
		//printf ("WORKING. STEP 1\n");
		if (f == 0)
		{
			struct sembuf mainbuf1[2] = 
   			{
   				4, 0, 0,
   				4, +1, SEM_UNDO
   			};
			if (semop(semid, mainbuf1, 2)< 0)
			{
				printf("Can\'t wait for condition 3\n");
				//scanf ("%c", &sym);
				shmctl (shmid, IPC_RMID, NULL);
				semctl(semid, 1, IPC_RMID); 
				exit(-1);
			}
			f++;
		}
		else
		{
			struct sembuf mainbuf2[4] = 
   			{
   				4, 0, 0,
   				4, +1, 0,
   				4, -1, SEM_UNDO,
   				4, +1, SEM_UNDO
   			};
			if (semop(semid, mainbuf2, 4)< 0)
			{
				printf("Can\'t wait for condition 3\n");
				//scanf ("%c", &sym);
				shmctl (shmid, IPC_RMID, NULL);
				semctl(semid, 1, IPC_RMID); 
				exit(-1);
			}
		}
		
		//printf ("WORKING. STEP 3 - %d\n", array->key);
		p = array->buff;
		while(array->pack)
		{
			b = write (STDOUT_FILENO, p , array->pack);
			p = array->buff + b;
			array->pack = array->pack - b;
		}
		if (array->key == 0)
			w = 0;
		array->pack = 0;
		array->package = 1;
		
		if (w != 0)
		{
			//printf ("WORKING. STEP 2\n");
			b = semctl(semid, 0, GETVAL);
			if (b == -1)
			{	
   				printf ("SEM_ERROR\n\n");
   				exit (0);
   			}
   			if (b == 0)
   			{
   				printf ("\nSENDER is dead\n\n");
   				exit (1); 
   			}
   		}
		
		//printf ("WORKING. STEP 4\n");
		sbuf.sem_num = 5;
		sbuf.sem_op = -1;
		sbuf.sem_flg = IPC_NOWAIT;
 		if(semop(semid, mybuf, 1) < 0)
		{
			b = semctl(semid, 1, GETVAL);
			if (b == -1)
			{
   				printf ("SEM_ERROR\n\n");
   				exit (0);
   			}
   			if (b == 0 && w != 0)
   			{
   				printf ("\nConsumer is dead2\n\n");
   				exit (1); 
   			}
		}
	}
	
 	//printf ("END OF WORK\n");
  	return 0;
}
