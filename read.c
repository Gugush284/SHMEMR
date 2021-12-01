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

int main(int argc, char *argv[])
{
	struct message
    	{ 
        	int package;
        	int pack;
        	char buff[50];
        	int key;
    	} *array;
    	
    	union semun {
      	int val;                  /* value for SETVAL */
      	struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
      	unsigned short *array;    /* array for GETALL, SETALL */
      	struct seminfo *__buf;    /* buffer for IPC_INFO */
	} arg;
    	
    	struct sembuf sbuf;
    	sbuf.sem_flg = 0;
    	struct sembuf * mybuf = &sbuf;  
    	
    	int shmid;        
    	char path[] = "Sender.c"; 
    	char pathname[] = "read.c";
    	key_t key;     
    	int semid; 
    	
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
    	(shmid = shmget(key, sizeof(struct message), 0666|IPC_CREAT|IPC_EXCL))
    	 < 0)
    	{
        	if(errno != EEXIST)
        	{
        	    printf("Can\'t create shared memory\n");
        	    exit(-1);
        	} 
       		else 
       	 	{	
            		if((shmid = shmget(key, sizeof(struct message), 0)) < 0)
           		{
               			printf("Can\'t find shared memory\n");
                		exit(-1);
            		}
        	}
    	}

    	if(
    	(array = (struct message *)shmat(shmid, NULL, 0)) == 
    	(struct message *)(-1))
    	{
        printf("Can't attach shared memory\n");
        exit(-1);
   	}
   	
   	int f = 0;
   	int f2 = 0;
   	int a, c, d;
   	
   	printf ("TRYING TO ENTER\n");
   	while (f2 != 1)
   	{
   		if(((a = semctl(semid, 1, GETVAL)) == 0) && ((c = semctl(semid, 2, GETVAL)) != 1))
   		{
   			d = semctl(semid, 4, GETVAL);
   			if (d  == -1)
			{
				printf ("SEM_ERROR\n\n");
				exit (0);
			}
   			if (d == 0)
   			{
   				sbuf.sem_num = 4;
				sbuf.sem_op = 1;
				sbuf.sem_flg = 0;
 				if(semop(semid, mybuf, 1) < 0)
				{
					printf("Can\'t wait for condition 9\n");
					shmctl (shmid, IPC_RMID, NULL);
					semctl(semid, 1, IPC_RMID); 
					exit(-1);
				}
   			}
   			
   			sbuf.sem_num = 1;
			sbuf.sem_op = 1;
			sbuf.sem_flg = SEM_UNDO;
 			if(semop(semid, mybuf, 1) < 0)
			{
				printf("Can\'t wait for condition 9\n");
				shmctl (shmid, IPC_RMID, NULL);
				semctl(semid, 1, IPC_RMID); 
				exit(-1);
			}
			f = 1;	
   		}
   		else if ((a == -1) || (c == -1))
   			exit (0);
   		
   		if((a = semctl(semid, 1, GETVAL)) > 1)
   		{
   			sbuf.sem_num = 1;
			sbuf.sem_op = -1;
			sbuf.sem_flg = SEM_UNDO;
 			if(semop(semid, mybuf, 1) < 0)
			{
				printf("Can\'t wait for condition 9\n");
				shmctl (shmid, IPC_RMID, NULL);
				semctl(semid, 1, IPC_RMID); 
				exit(-1);
			}
			f = 0;		
   		}
   		else if (a == -1)
   			exit (0);
   		
   		a = semctl(semid, 0, GETVAL);
   		if (a == -1)
   			exit (0);	
   		if ((f == 1) && (a==1))
   		{
   			sbuf.sem_num = 3;
			sbuf.sem_op = 1;
			sbuf.sem_flg = SEM_UNDO;
 			if(semop(semid, mybuf, 1) < 0)
			{
				printf("Can\'t wait for condition 9\n");
				shmctl (shmid, IPC_RMID, NULL);
				semctl(semid, 1, IPC_RMID); 
				exit(-1);
			}
			
			printf ("ok\n");
			f2 = 1;
   		}
   	}
   	
   	printf ("ENTERED\n");
   		
   	while ((a = semctl(semid, 4, GETVAL)) > 1)
   	{
   		sbuf.sem_num = 4;
		sbuf.sem_op = -1;
		sbuf.sem_flg = 0;
 		if(semop(semid, mybuf, 1) < 0)
		{
			printf("Can\'t wait for condition 9\n");
			shmctl (shmid, IPC_RMID, NULL);
			semctl(semid, 1, IPC_RMID); 
			exit(-1);
		}	
   	}
   	if (a  == -1)
	{
		printf ("SEM_ERROR\n\n");
		exit (0);
	}				
   	
	printf ("WORKING\n");
	int w = 1;
	int b;
	array->pack = 0;
	array->package = 1;
	array->key = 1;	
	while (w)
	{
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
		
		if(array->pack)
		{
			printf ("%s", array->buff);
		}
		if (array->key == 0)
			w = 0;
		array->pack = 0;
		array->package = 1;
		
		sbuf.sem_num = 4;
		sbuf.sem_op = 1;
		sbuf.sem_flg = SEM_UNDO;
 		if(semop(semid, mybuf, 1) < 0)
		{
			printf("Can\'t wait for condition 9\n");
			shmctl (shmid, IPC_RMID, NULL);
			semctl(semid, 1, IPC_RMID); 
			exit(-1);
		}	
	}
	
 	printf ("END OF WORK\n");
   	
  	return 0;
}
