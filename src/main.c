/**
  * @file main.c
  * @brief Funzione principale del programma
  * 
  * Contiene il codice che gestisce il processo padre che si occupa
  * della sincronizzazione di tutti i processi figli per il calcolo 
  * del prodotto tra matrici quadrate dello stesso ordine
  *
  * @author Ghignoni Eros VR397407
  * 
  */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/sem.h>

//my libraries
#include "mylib.h"
#include "utils.h"
#include "child.h"

///Definisce il numero di parametri necessario all'esecuzione del programma
#define NUM_PARAM 5

/**
	
	Funzione main

	Si occupa di:\n
		- apertura dei file contenenti le matrici quadrate A e B\n
		- creazione del file contenente la matrice risultato C\n
		- alloca tutte le risorse necessario per la comunicazione padre <-> figli (pipe, 
		  memoria condivisa)\n
		- alloca un semaforo mutex per consentire la sincronizzazione dei figli che eseguono la
		  somma delle righe della matrice C\n
		- gestisce gli errori derivanti da una non corretta allocazione delle risorse\n
		- partiziona il lavoro nel modo più bilanciato possibile\n
		- scrive a schermo la matrice risultante C e la somma delle righe\n
		- dealloca tutte le risorse al termine del programma

  */

int main(int argc, char * argv[]){

int fd_matA;		//Contiene il file descriptor della matrice A
int fd_matB;		//Contiene il file descriptor della matrice B
int fd_matC;		//Contiene il file descriptor della matrice 
int ordine;			//Ordine delle matrici quadrate A, B, C
int num_processi;	
int err_flag = 0;
int ** matrix_A;
int ** matrix_B;
int i, j;	
int res;
char * std_buff = malloc(sizeof(char) * 256);					//buffer di scrittura su schermo
int shmem_A, shmem_B, shmem_C, shmem_somma;  //contengono gli id delle memorie condivise
int * pids;					//vettore che contiene i PID dei processi figli creati 
int num_working;
int msgid;
message messaggio;
char buff_param[64];
int ** control_matrix;
int sem_id;

	
	if (std_buff == NULL){
		stampa("ERROR");
		return 0;
	}


	if (argc != NUM_PARAM + 1){     // + nome del programma

		strcpy(std_buff, "Errore parametri: matA matB matC ordine processi\n");
		stampa(std_buff);

		free(std_buff);
		return 0;

	} else {

		fd_matA = open(argv[1], O_RDONLY, S_IRUSR);    
		fd_matB = open(argv[2], O_RDONLY, S_IRUSR);		
		fd_matC = creat(argv[3], O_RDWR | O_TRUNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);  //controllo se il file esiste già

	
		ordine = atoi(argv[4]);

		num_processi = atoi(argv[5]);

		if (fd_matA < 0){
			strcpy(std_buff,"Errore apertura file matA\n");
			stampa(std_buff);
			err_flag = 1;
		}
		if (fd_matB < 0){
			strcpy(std_buff,"Errore apertura file matB\n");
			stampa(std_buff);
			err_flag = 1;
		}
		if (fd_matC < 0){
			strcpy(std_buff,"Errore apertura file matC\n");
			stampa(std_buff);
			err_flag = 1;
		}

		if (ordine <= 0){
			strcpy(std_buff,"Errore: ordine deve essere > 0\n");
			stampa(std_buff);
			err_flag = 1;
		}

		if (num_processi <= 0){
			strcpy(std_buff,"Errore: numero processi deve essere > 0\n");
			stampa(std_buff);
			err_flag = 1;
		}
		if (err_flag){				//se ci sono stati errori, esco dal programma
			close(fd_matA);
			close(fd_matB);
			close(fd_matC);
			free(std_buff);
			return 0;
		}

	}

	matrix_A = create_matrix(ordine);
	matrix_B = create_matrix(ordine);
	control_matrix = create_matrix(ordine);

	for (i = 0; i < ordine; i++)
		for (j = 0; j < ordine; j++)
			control_matrix[i][j] = 0;

	read_matrix(fd_matA, ordine, matrix_A);
	read_matrix(fd_matB, ordine, matrix_B);

	//Alloco lo spazio di memoria condivisa in cui salvare matrix_A
	shmem_A = shmget(SHM_KEY_A, sizeof(int[ordine][ordine]), (0666 | IPC_CREAT | IPC_EXCL)); 
	//crea memoria condivisa con permessi di lettura e scrittura (per tutti), solo se non esiste gia' un'altra mem condivisa
	//associata alla SHM_KEY_num_working

	//Alloco lo spazio di memoria condivisa in cui salvare matrix_B
	shmem_B = shmget(SHM_KEY_B, sizeof(int[ordine][ordine]), (0666 | IPC_CREAT | IPC_EXCL));

	//Alloco lo spazio di memoria condivisa in cui salvare matrix_C
	shmem_C = shmget(SHM_KEY_C, sizeof(int[ordine][ordine]), (0666 | IPC_CREAT | IPC_EXCL));

	shmem_somma = shmget(SHM_KEY_SOMMA, sizeof(int), (0666 | IPC_CREAT | IPC_EXCL));


	if (shmem_A == -1 || shmem_B == -1 || shmem_C == -1 || shmem_somma == -1){
		stampa("Errore durante l'allocazione di memoria condivisa per matrice A\n");
		return 0;
	}


	int * shmatrix_A = shmat(shmem_A, NULL, 0);   //array che emula una matrice
	if (shmatrix_A == (void *)-1){
		stampa("Errore attach matrice A\n");
		return 0;
	}

	//carico la matrice A condivisa
	load_matrix(matrix_A, shmatrix_A, ordine);

	int * shmatrix_B = shmat(shmem_B, NULL, 0);   //array che emula una matrice
	if (shmatrix_B == (void *)-1){
		stampa("Errore attach matrice B\n");
		return 0;
	}

	//carico la matrice B condivisa
	load_matrix(matrix_B, shmatrix_B, ordine);

	int * shmatrix_C = shmat(shmem_C, NULL, 0);   //array che emula una matrice
	if (shmatrix_C == (void *)-1){
		stampa("Errore attach matrice C\n");
		return 0;
	}

	sem_id = semget(SEM_KEY, 1, 0666 | IPC_CREAT | IPC_EXCL);
	if (sem_id < 0){
		stampa("Errore nell'allocazione del semaforo\n");
		return 0;
	}

	struct sembuf operation;
	operation.sem_num = 0;		//primo ed unico semaforo
	operation.sem_op = 1;		//inizializzo a 1 -> mutex
	operation.sem_flg = 0;		//nessuna flag settata

	semop(sem_id, &operation, 1);		//esecuzione dell'operazione

	int * shmsomma = shmat(shmem_somma, NULL, 0);   //array che emula una matrice
	if (shmsomma == (void *)-1){
		stampa("Errore attach somma\n");
		return 0;
	}

	*shmsomma = 0;

	//creazione pipe per comunicazione parametri ai processi figli
	//param_pipe = malloc(sizeof(int*) * num_processi);
	int param_pipe[num_processi][2];

	pids = malloc(sizeof(int) * num_processi);
	
	if (pids == NULL){
		stampa("Errore nella creazione processi\n");
		return 0;
	}

	msgid = msgget(MSG_KEY, 0666 | IPC_CREAT | IPC_EXCL);
	if (msgid == -1){
		stampa("Errore nella creazione della coda messaggi\n");
		return 0;
	}

	//genero i processi richiesti dall'utente
	for (num_working = 0; num_working < num_processi; num_working++){
		//creo la pipe per comunicare col processo i-esimo
		
		if (pipe(param_pipe[num_working]) == -1){
			stampa("Errore creazione pipe\n");
			return 0;
		}

		pids[num_working] = fork();			//salvo nel vettore pids tutti i pid dei processi figli creati 
		if (pids[i] == -1){
			stampa("Errore nella creazione processi figli\n");
			return 0;
		}
		if (pids[num_working] == 0){			//figlio
			if (close(param_pipe[num_working][1]) == -1){
				stampa("Errore chiusura pipe figlio\n");
				exit(1);
			}
			execute(param_pipe[num_working][0]);
			exit(0);
		}
		else{					//padre
			close(param_pipe[num_working][0]);
		}
	}
	num_working = 0;

	//caso particolare
	if (ordine == 1){
		genera_parametri(0, 0, ordine, CHILD_MOLTIPLICA, buff_param);
		write(param_pipe[num_working][1], buff_param, strlen(buff_param));

		msgrcv(msgid, &messaggio, sizeof(message) - sizeof(long), MSG_TYPE, 0);	
		int p_index = position(pids, messaggio.pid, num_processi);
		control_matrix[messaggio.riga][messaggio.colonna] = 1;

		genera_parametri(0, 0, ordine, CHILD_MOLTIPLICA, buff_param);
		write(param_pipe[(p_index + 1) % num_processi][1], buff_param, strlen(buff_param));
		num_working++;

	} else{

		for (i = 0; i < ordine * ordine; i++){         //per semplicità

			for (; num_working < num_processi && num_working < ordine * ordine; num_working++, i++){

				//posso "allocare" del nuovo lavoro per il processo disponibile
				genera_parametri(i/ordine, i%ordine, ordine, CHILD_MOLTIPLICA, buff_param);
				write(param_pipe[num_working][1], buff_param, strlen(buff_param));
			}

			//attendo che un processo termini per potergli dare del nuovo lavoro da fare
			msgrcv(msgid, &messaggio, sizeof(message) - sizeof(long), MSG_TYPE, 0);	

			if (messaggio.operation == CHILD_MOLTIPLICA){
				control_matrix[messaggio.riga][messaggio.colonna] = 1;	//calcolo effettuato
			}
			//trovo l'indice del processo che ha terminato -> lo uso per mandare sulla sua pipe
			//il prossimo comando da eseguire
			int p_index = position(pids, messaggio.pid, num_processi);
				
			num_working--;

			//se ho ancora delle moltiplicazioni da fare... 
			if (i < ordine * ordine){
				genera_parametri(i/ordine, i%ordine, ordine, CHILD_MOLTIPLICA, buff_param);
				write(param_pipe[p_index][1], buff_param, strlen(buff_param));
				
				num_working++;
			}

		}
	}
	
	i = 0;		//indica quanti processi posso allocare ancora (se numProcessi > ordine * ordine)
	int r;

	while(num_working > 0){
		msgrcv(msgid, &messaggio, sizeof(message) - sizeof(long), MSG_TYPE, 0);	
		int p_index = position(pids, messaggio.pid, num_processi);

		num_working--;


		if (messaggio.operation == CHILD_MOLTIPLICA){
			control_matrix[messaggio.riga][messaggio.colonna] = 1;
		}
		if (messaggio.operation == CHILD_SOMMA){
			set_row(control_matrix, ordine, messaggio.riga, 2);
		}

		//Ci sono ancora righe da sommare
		if (i < ordine){
			//cerco la prossima riga
			r = get_next_row(control_matrix, ordine);

			//ho trovato la riga da sommare... altrimenti aspetto un nuovo processo
			if (r != -1){
				i++;
				genera_parametri(r, 0, ordine, CHILD_SOMMA, buff_param);
				write(param_pipe[p_index][1], buff_param, strlen(buff_param));
				set_row(control_matrix, ordine, r, 2);
				num_working++;
			}
		}
	}

	stampa("\nLa matrice risultato C :\n");

	print_matrix_2d(shmatrix_C, ordine);
	write_matrix_(fd_matC, shmatrix_C, ordine);

	stampa("Attendo la terminazione di tutti i processi allocati\n");
	
	//attendo tutti i processi figli
	genera_parametri(0, 0, 0, CHILD_EXIT, buff_param);
	for (num_working = 0; num_working < num_processi; num_working++){
		write(param_pipe[num_working][1], buff_param, strlen(buff_param));	
	}
	
	sprintf(std_buff, "\n\nLa somma totale delle righe corrisponde a : %d\n", *shmsomma);
	stampa(std_buff);

	//prima di deallocare le risorse, attendo la terminazione di tutti i processi figli
	while (wait(NULL) > 0);

	if (msgctl(msgid, IPC_RMID, NULL) == -1){
		stampa("Errore cancellazione coda messaggi\n");
		return 0;
	}

	res = shmdt(shmatrix_A);
	if (res < 0){
		stampa("Errore detatch mem A\n");
	}

	res = shmctl(shmem_A, IPC_RMID, NULL);
	if (res < 0){
		stampa("Errore cancellazione A\n");
	}

	res = shmdt(shmatrix_B);
	if (res < 0){
		stampa("Errore detatch mem B\n");
	}

	res = shmctl(shmem_B, IPC_RMID, NULL);
	if (res < 0){
		stampa("Errore cancellazione B\n");
	}

	res = shmdt(shmatrix_C);
	if (res < 0){
		stampa("Errore detatch mem C\n");
	}

	res = shmctl(shmem_C, IPC_RMID, NULL);
	if (res < 0){
		stampa("Errore cancellazione C\n");
	}

	res = shmdt(shmsomma);
	if (res < 0){
		stampa("Errore detatch mem somma\n");
	}

	res = shmctl(shmem_somma, IPC_RMID, NULL);
	if (res < 0){
		stampa("Errore cancellazione somma\n");
	}

	res = semctl(sem_id, 0, IPC_RMID);
	if (res < 0){
		stampa("Errore deallocazione semaforo\n");
	}

	free(pids);
	free_matrix(matrix_A, ordine);
	free_matrix(matrix_B, ordine);
	free(std_buff);
	close(fd_matA);
	close(fd_matB);
	close(fd_matC);

	return 0;
}

