/**
  * @file utils.h
  * @brief Libreria che contiene funzioni di utilità per il programma
  * 
  * Libreria creata dall'autore
  *
  * @author Ghignoni Eros VR397407
  * 
  */

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef UTILS_H
#define UTILS_H

///Numero massimo di byte per riga file matrice
#define NUM_BYTE 512

///Chiave per memoria condivisa matrix_A
#define SHM_KEY_A 90   

///Chiave per memoria condivisa matrix_B 
#define SHM_KEY_B 91

///Chiave per memoria condivisa matrix_C	
#define SHM_KEY_C 92

///Chiave per memoria condivisa SHM_KEY_SOMMA	
#define SHM_KEY_SOMMA 93

///Chiave per messaggi tra padre e figli
#define MSG_KEY 100	

///Chiave per semaforo mutex condiviso	
#define SEM_KEY 101	

///Delimitatore dei comandi padre -> figli
#define DELIMITER ";"	

/**
  * @brief Funzione che stampa a schermo (usando system call) la stringa come parametro
  * @param buff stringa da stampare a schermo
  */
void stampa(char* buff);

/**
  * @brief Funzione che legge una matrice da file
  * @param fd file descriptor del file csv contenente la matrice
  * @param ordine ordine della matrice
  * @param matrix puntatore (output) alla matrice letta
  */
void read_matrix(int fd, int ordine, int ** matrix);

/**
  * @brief Funzione che legge una riga da file
  * @param fd file descriptor del file
  * @param buff buffer di uscita su cui scrivere la stringa letta
  * @param max_len lunghezza massima del buffer
  * @return numero di caratteri letti
  */
int read_line(int fd, char * buff, int max_len);		//ritorna il numero di caratteri letti

/**
  * @brief Stampa a schermo la matrice passata come parametro
  * @param matrix puntatore alla matrice da stampare
  * @param ordine ordine della matrice 
  */
void print_matrix(int ** matrix, int ordine);

/**
  * @brief Stampa a schermo la matrice 2d passata come parametro
  *        (matrice salvata come vettore unidimensionale)
  * @param matrix puntatore alla matrice
  * @param ordine ordine della matrice
  */
void print_matrix_2d(int * matrix, int ordine);

/**
  * @brief Funzione che alloca la memoria ad una matrice di ordine passato come parametro
  * 	   inizializzata a 0 
  * @param order ordine della matrice
  */
int ** create_matrix(int order);

/**
  * @brief Libera la memoria allocata per la matrice passata come parametro
  * @param matrix contiene il puntatore alla matrice
  * @param order ordine della matrice
  */
void free_matrix(int ** matrix, int order);

/**
  * @brief Carica una matrice in un vettore unidimensionale
  * @param matrix contiene il puntatore della matrice (input)
  * @paam array2d contiene il puntatore all'array su cui salvare la matrice(output)
  * @param order ordine della matrice
  */
void load_matrix(int ** matrix, int * array2d, int order);

/**
  * @brief Inizializza la matrice a 0
  * @param shmem puntatore al vettore matrice
  * @param order ordine della matrice
  */
void set_matrix(int *shmem, int order);

/**
  * @biref Trova la posizione in vec dell'elemento passato come parametro
  * @param vec vettore input
  * @param elem elemento da ricercare in vec
  * @param len lunghezza di vec
  * @return indice di eleme in vec, -1 se non presente
  */
int position(int * vec, int elem, int len);

/**
  * @brief Scrive la matrice in formato csv su file descriptor passato come parametro
  * @param fd file descriptor di scrittura
  * @param matrix matrice da scrivere su file
  * @param ord ordine della matrice
  */
void write_matrix_(int fd, int ** matrix, int ord);

/**
  * @brief Ritorna l'indice di riga della matrice che è gia stata calcolata tutta (moltiplicazione)
  		   La matrice passata per parametro è una matrice di controllo in cui è settato a 1 l'elemento già calcolato
  * @param control_matrix matrice di controllo (1 = elemento calcolato)
  * @param order ordine della matrice
  * @return indice della riga della matrice in cui è già stata calcolata la moltiplicazione 
  */
int get_next_row(int ** control_matrix, int order);

/**
  * @brief Scrive una riga della matrice passata come parametro, con il valore selezionato
  * @param control_matrix matrice
  * @param order ordine della matrice
  * @param row riga da modificare
  * @param valore da memorizzare nella riga
  */
void set_row(int ** control_matrix, int order, int row, int value);

/**
  * @brief Genera una stringa che descrive i parametri che il padre invia al figlio
  * @param riga riga della matrice A
  * @param colonna colonna della matrice B
  * @param ordine ordine delle due matrici
  * @param cmd comando da eseguire (vedi file child.h)
  * @param buff stringa su cui viene salvata la stringa di comando
  */
void genera_parametri(int riga, int colonna, int ordine, int cmd, char * buff);

#endif