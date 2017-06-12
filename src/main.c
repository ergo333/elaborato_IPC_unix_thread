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
#include <pthread.h>

//my libraries
#include "mylib.h"
#include "utils.h"
#include "child.h"

///Definisce il numero di parametri necessario all'esecuzione del programma
#define NUM_PARAM 4

/**
  * @brief
  * Funzione principale del programma\n
  * Si occupa di allocare dinamicamente il numero di thread necessarie a svolgere nel modo 
    più efficiente e veloce possibile, il calcolo del prodotto tra le matrici A e B e per calcolare
    la somma delle righe della matrice prodotto

  * @param argc numero di argomenti passati da riga di comando
  * @param argv vettore che contiene i parametri da riga di comando 
  */

int main(int argc, char * argv[]){

int fd_matA;		//Contiene il file descriptor della matrice A
int fd_matB;		//Contiene il file descriptor della matrice B
int fd_matC;		//Contiene il file descriptor della matrice 
int ordine;			//Ordine delle matrici quadrate A, B, C
int num_thread;	
int ** matrix_A;
int ** matrix_B;
int ** matrix_C;
int somma = 0;
int i, j;
pthread_t * threads;
int * stato;			//memorizzo gli stati dei thread che eseguono la moltiplicazione per sapere se finiscono
						//e se posso procedere con la somma delle righe
pthread_t * sum_threads;
pthread_mutex_t * s_stato;
pthread_mutex_t sem_somma;
int err_flag = 0;
char * std_buff = malloc(sizeof(char) * 256);					//buffer di scrittura su schermo

	
	if (std_buff == NULL){
		stampa("ERROR");
		return 0;
	}

	if (argc != NUM_PARAM + 1){     // + nome del programma

		strcpy(std_buff, "Errore parametri: matA matB matC ordine\n");
		stampa(std_buff);

		free(std_buff);
		return 0;

	} else {

		fd_matA = open(argv[1], O_RDONLY, S_IRUSR);    
		fd_matB = open(argv[2], O_RDONLY, S_IRUSR);		
		fd_matC = creat(argv[3], O_RDWR | O_TRUNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);  //controllo se il file esiste già

	
		ordine = atoi(argv[4]);

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
	matrix_C = create_matrix(ordine);

	read_matrix(fd_matA, ordine, matrix_A);
	read_matrix(fd_matB, ordine, matrix_B);


	//genero un numero di thread pari all'ordine della matrice
	//ogni thread può così effettuare la moltiplicazione indipendentemente
	threads = (pthread_t *) malloc(sizeof(pthread_t) * ordine * ordine);

	stato = (int *) calloc(ordine * ordine, sizeof(int));

	s_stato = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * ordine * ordine);

	if (threads == NULL || stato == NULL || s_stato == NULL){
		stampa("Errore malloc threads\n");
		close(fd_matA);
		close(fd_matC);
		close(fd_matB);
		free(threads);
		free(stato);
		free(s_stato);
		free(std_buff);
	}

	//inizializzazione dei semafori per lo stato delle thread
	for (i = 0; i < ordine * ordine; i++){
		pthread_mutex_init(&s_stato[i], NULL);	//default
	}


	comando * comandi = malloc(sizeof(comando) * ordine * ordine);
	if (comandi == NULL){
		stampa("Errore allocazione memoria\n");
		return 0;
	} 

	for (i = 0; i < ordine * ordine; i++){
		comandi[i].riga = i/ordine;
		comandi[i].colonna = i%ordine;
		comandi[i].ordine = ordine;
		comandi[i].i_thread = i;
		comandi[i].cmd = CHILD_MOLTIPLICA;
		comandi[i].matrix_A = matrix_A;
		comandi[i].matrix_B = matrix_B;
		comandi[i].matrix_C = matrix_C;
		comandi[i].somma = NULL;
		comandi[i].s_stato = &s_stato[i];
		comandi[i].stato = &stato[i];
		comandi[i].sem_somma = NULL;
		
		if (pthread_create(&threads[i], NULL, &execute, &comandi[i])){
			stampa("Errore creazione thread\n");
			free(threads);
			free(comandi);
			free(matrix_A);
			free(matrix_B);
			free(matrix_C);
			free(std_buff);	
			return 0;
		}

	
	}
	int conta_righe = 0;

	sum_threads = malloc(sizeof(pthread_t) * ordine);
	if (sum_threads == NULL){
		stampa("Errore creazione thread di somma\n");
		return 0;
	}

	comando * comandi_somma = malloc(sizeof(comando) * ordine);

	if (comandi_somma == NULL){
		stampa("Errore  creazione comando per threadn\n");
		free(threads);
		free(comandi);
		free(sum_threads);
		free(matrix_A);
		free(matrix_B);
		free(matrix_C);
		free(std_buff);
		return 0;
	}	

	pthread_mutex_init(&sem_somma, NULL);

	int conta_di_seguito;
	while (conta_righe < ordine){
		
		for (i = 0; i < ordine; i++){
			conta_di_seguito = 0;

			for (j = 0; j < ordine; j++){		//controllo se tutti i thread che sono incaricati di effettuare
												//la moltiplicazione della i esima riga hanno terminato
				pthread_mutex_lock(&s_stato[i * ordine + j]);
				if (stato[i*ordine + j] == 1){
					conta_di_seguito++;
					pthread_mutex_unlock(&s_stato[i * ordine + j]);
				}
				else{
					pthread_mutex_unlock(&s_stato[i * ordine + j]);
					break;
				}

			}
			if (conta_di_seguito == ordine)
				break;
		}

		if (conta_di_seguito == ordine){	//tutte le thread hanno completato la moltiplicazione della riga di C
			
    		comandi_somma[i].riga = i;
			comandi_somma[i].colonna = 0;
			comandi_somma[i].ordine = ordine;
			comandi_somma[i].i_thread = i + ordine*ordine;
			comandi_somma[i].cmd = CHILD_SOMMA;
			comandi_somma[i].matrix_A = NULL;
			comandi_somma[i].matrix_B = NULL;
			comandi_somma[i].matrix_C = matrix_C;
			comandi_somma[i].somma = &somma;
			comandi_somma[i].s_stato = NULL;
			comandi_somma[i].stato = NULL;
			comandi_somma[i].sem_somma = &sem_somma;

			conta_righe++;

			if (pthread_create(&sum_threads[i], NULL, &execute, &comandi_somma[i])){
				stampa("Errore creazione thread\n");
				free(threads);
				free(comandi);
				free(comandi_somma);
				free(sum_threads);
				free(matrix_A);
				free(matrix_B);
				free(matrix_C);
				free(std_buff);
				return 0;
			}

			for (j = 0; j < ordine; j++)
				stato[i * ordine + j] = 2;	//somma sulla riga già in esecuzione

		}		

	}

	stampa("Attendo la terminazione di tutti i processi allocati\n");
	//attendo la terminazione dei thread
	for (i = 0; i < ordine * ordine; i++){
		pthread_join(threads[i], NULL);
	}

	for (i = 0; i < ordine; i++){
		pthread_join(sum_threads[i], NULL);
	}
	
	stampa("\nLa matrice risultato C :\n");

	print_matrix(matrix_C, ordine);
	write_matrix_(fd_matC, matrix_C, ordine);	
	sprintf(std_buff, "\n\nLa somma totale delle righe corrisponde a : %d\n", somma);
	stampa(std_buff);
	
	pthread_mutex_destroy(&sem_somma);
	for (i = 0; i < ordine * ordine; i++){
		pthread_mutex_destroy(&s_stato[i]);
	}

	free(threads);
	free(comandi);
	free(comandi_somma);
	free(sum_threads);
	free(matrix_A);
	free(matrix_B);
	free(matrix_C);
	free(std_buff);
	close(fd_matA);
	close(fd_matB);
	close(fd_matC);

	return 0;
}

