/**
  * @file child.h
  * @brief Descrive le operazioni che i thread "figl" devono eseguire
  *
  * @author Ghignoni Eros VR397407
  * 
  */

#include <string.h>
#include <pthread.h>

#ifndef CHILD_H
#define CHILD_H

///Comando che indica che il figlio deve eseguire una somma
#define CHILD_SOMMA 1

///Comando che indica che il figlio deve eseguire una moltiplicazione
#define CHILD_MOLTIPLICA 0

typedef struct{

  int riga;           ///Indica la riga su cui effettuare l'operazione
  int colonna;        ///Indica la colonna su cui effettuare l'operazione
  int ordine;         ///Indica l'ordine delle matrici
  int i_thread;       ///Indica l'indice del thread
  int ** matrix_A;    ///Indica la matrice A su cui effettuare la moltiplicazione
  int ** matrix_B;    ///Indica la matrice B su cui effettuare la moltiplicazione
  int ** matrix_C;    ///Indica la matrice C su cui salvare il risultato della moltiplicazione
  int * somma;        ///Puntatore alla variabile su cui salvare la somma delle righe di C
  int cmd;            ///Indica il  comando che il figlio deve eseguire
  pthread_mutex_t * s_stato;    ///Semaforo per modificare lo stato della thread visto dal padre
  int * stato;                  ///Vettore in cui sono salvati gli stati delle thread
  pthread_mutex_t * sem_somma;  ///Semaforo per accedere alla variabile condivisa somma righe

}comando;


void execute(void *cmd);

/**
  * @brief Funzione che calcola la somma della riga della matrice
  * @param matrix matrice
  * @param row riga da sommare
  * @param ordine della matrice
  * @return il risultato della somma
  */
int somma(int ** matrix, int row, int ordine);

/**
  * @brief Funzione che effettua la moltiplicazione tra vettori
  * @param a matrice A
  * @param b matrice B
  * @param row_a riga di A
  * @param column_b colonna di B
  * @param ordine ordine delle matrici
  * @return il risultato del prodotto tra vettori 
  */
int moltiplica(int ** a, int ** b, int row_a, int column_b, int ordine);

/**
  * @brief Funzione che estrae il comando da eseguire dalla stringa inviata dal processo padre
  * @param cmd salva il valore del comando da eseguire
  * @param riga salva il valore della riga su cui eseguire i calcoli
  * @param colonna salva il valore della colonna su cui eseguire i calcoli
  * @param msg stringa inviata dal padre al figlio
  */
void estrai_dati(int * cmd, int * riga, int * colonna, int * ordine, char * msg);

#endif