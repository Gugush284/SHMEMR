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
   	struct sembuf enbuf[3] = 
   	{
   		0, 0, 0,
   		2, 0, 0,
   		0, +1, SEM_UNDO,
   	};
	if (semop(semid, enbuf, 3)<0)
	{
		printf("Can\'t wait for condition 1\n");
		shmctl (shmid, IPC_RMID, NULL);
		semctl(semid, 1, IPC_RMID); 
		exit(-1);
	}
	
	//printf ("Making barrier\n");
	struct sembuf chbuf[3] = 
   	{
   		1, -1, 0,
   		1, +1, 0,
   		3, +1, SEM_UNDO
   	};
	if(semop(semid, chbuf, 3) < 0)
	{
		printf("Can\'t wait for condition 2\n");
		shmctl (shmid, IPC_RMID, NULL);
		semctl(semid, 1, IPC_RMID); 
		exit(-1);
	}
	
   	//printf ("ENTERED\n");
   	//printf ("WORKING\n");
	
	int w = 1;
	int i, k, s, b;
	
	assert (argc == 2);
	FILE *file = fopen (argv[1], "r");
	assert (file != NULL);
	
	char sym;
	char *p;
	sbuf.sem_num = 5;
	sbuf.sem_op = 0;
	sbuf.sem_flg = 0;
 	if(semop(semid, mybuf, 1) < 0)
	{
		printf("Can\'t wait for condition 4\n");
		shmctl (shmid, IPC_RMID, NULL);
		semctl(semid, 1, IPC_RMID); 
		exit(-1);
	}	
	while (w)
	{
		//printf ("WORKING. STEP 0\n");
		struct sembuf mainbuf[2] = 
   		{
   			4, 0, 0,
   			4, +1, SEM_UNDO
   		};
		if (semop(semid, mainbuf, 2)< 0)
		{
			printf("Can\'t wait for condition 3\n");
			shmctl (shmid, IPC_RMID, NULL);
			semctl(semid, 1, IPC_RMID); 
			exit(-1);
		}
		
		//printf ("WORKING. STEP 1\n");
		b = semctl(semid, 1, GETVAL);
		if (b == -1)
		{
   			fclose (file);
   			printf ("SEM_ERROR\n\n");
   			exit (0);
   		}
   		if (b == 0)
   		{
   			fclose (file);
   			printf ("\nConsumer is dead\n\n");
   			exit (1); 
   		}
		
		//printf ("WORKING. STEP 2\n");
		if (array->package)
		{
		p = array->buff;
		i = fscanf (file, "%c", &sym);
		if (i == -1)
		{
			array->pack = 0;
			//printf ("PROBLEM\n");
			w = 0;
			array->package = 0;
			array->key = 0;
		}
		else
		{
			*p = sym;
			p++;
			b = 1;
			array->pack = 1;
			array->package = 0;
			array->key = 1;
			
			while (b < 100)
			{
				i = fscanf (file, "%c", &sym);
				if (i == -1)
				{
					*p = '\0';
					//printf ("PROBLEM\n");
					w = 0;
					array->package = 0;
					array->key = 0;
				}
				else
				{
					*p = sym;
					array->pack ++;
					p++;
				}
				b++;
			}
		}
		}
		
		
		//printf ("WORKING. STEP 3\n");
		sbuf.sem_num = 4;
		sbuf.sem_op = -1;
		sbuf.sem_flg = SEM_UNDO;
 		if(semop(semid, mybuf, 1) < 0)
		{
			printf("Can\'t wait for condition 9\n");
			shmctl (shmid, IPC_RMID, NULL);
			semctl(semid, 1, IPC_RMID); 
			exit(-1);
		}
		
		//printf ("WORKING. STEP 4\n");
		if (w == 0)
			break;	
	}
	
 	fclose (file);
 	//printf ("END OF WORK\n");
  	return 0;
}
