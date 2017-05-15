/**
  * @file child.h
  * @brief Descrive le operazioni che i processi figli devono eseguire
  *
  * @author Ghignoni Eros VR397407
  * 
  */

#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>


#ifndef CHILD_H
#define CHILD_H

///Comando che indica che il figlio deve eseguire una somma
#define CHILD_SOMMA 1

///Comando che indica che il figlio deve eseguire una moltiplicazione
#define CHILD_MOLTIPLICA 0

//Comando che indica che il figlio ha terminato
#define CHILD_EXIT -1

///Tipo del messaggio (attributo mtype della struct message)
#define MSG_TYPE 1

/**
  * @brief Struttura dati che descrive il formato dei messaggi inviati da child a parent
  */
typedef struct{

	long mtype;  	///indica il tipo di messaggio (MSG_TYPE)
	int pid;	  	///indica il pid del figlio
	int riga;		///riga usata per i calcoli (matrice A per moltiplicazione, C per somma)
	int colonna;	///colonna usata per i calcoli (matrice B)
	int operation;	///operazione eseguita e terminata (vedi costanti)

}message;

/** 
  * @brief Funzione principale del processo figlio.
    Si occupa dello svolgimento delle operazioni sulle matrici richieste dal processo padre
  * @param pipe_fd contiene il file descriptor su cui leggere i comandi inviati dal processo padre
  */
void execute(int pipe_fd);

/**
  * @brief Funzione che calcola la somma della riga della matrice
  * @param matrix matrice
  * @param row riga da sommare
  * @param ordine della matrice
  * @return il risultato della somma
  */
int somma(int * matrix, int row, int ordine);

/**
  * @brief Funzione che effettua la moltiplicazione tra vettori
  * @param a matrice A
  * @param b matrice B
  * @param row_a riga di A
  * @param column_b colonna di B
  * @param ordine ordine delle matrici
  * @return il risultato del prodotto tra vettori 
  */
int moltiplica(int * a, int * b, int row_a, int column_b, int ordine);

/**
  * @brief Funzione che estrae il comando da eseguire dalla stringa inviata dal processo padre
  * @param cmd salva il valore del comando da eseguire
  * @param riga salva il valore della riga su cui eseguire i calcoli
  * @param colonna salva il valore della colonna su cui eseguire i calcoli
  * @param msg stringa inviata dal padre al figlio
  */
void estrai_dati(int * cmd, int * riga, int * colonna, int * ordine, char * msg);

#endif