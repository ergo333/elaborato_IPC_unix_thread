#include "child.h"
#include "utils.h"


void execute(void * cmd){

char buff[128];

	comando * c = (comando *) cmd;

	if (c -> cmd == CHILD_MOLTIPLICA){ 
	
		sprintf(buff, "Thread -> MOILTIPLICA [%d][%d]\n", c -> riga, c -> colonna);
		stampa(buff);
		
		int mul = moltiplica(c -> matrix_A, 
							 c -> matrix_B, 
							 c -> riga,
							 c -> colonna,
							 c -> ordine);
		//inserisco il risultato in c
		c -> matrix_C[c -> riga][c -> colonna] = mul;
		//aggiorno lo stato per indicare che ho finito la moltiplicazione
		//ed eventualmente fare in modo che il padre generi nuovi thread
		//per la somma delle righe
		
		pthread_mutex_lock(c -> s_stato);
		*(c -> stato) = 1;				//moltiplicazione terminata
		pthread_mutex_unlock(c -> s_stato);


	}
	if (c -> cmd == CHILD_SOMMA){
		
		sprintf(buff, "Thread -> SOMMA [%d]\n", c -> riga);
		stampa(buff);
		int val = somma(c -> matrix_C, c -> riga, c -> ordine);

		pthread_mutex_lock(c -> sem_somma);
		// P bloccante
		*(c -> somma) += val;

		// V sblocco il semaforo
		pthread_mutex_unlock(c -> sem_somma);
		
	}

	pthread_exit(NULL);
}


int somma(int ** matrix, int row, int ordine){

int j;
int sum = 0;

	for (j = 0; j < ordine; j++){
		sum += matrix[row][j];
	}
	return sum;
}

int moltiplica(int ** a, int ** b, int row_a, int column_b, int ordine){    // riga A * riga B

int result = 0;
int i;

	for (i = 0; i < ordine; i++){
		result += a[row_a][i] * b[i][column_b];
	}
	return result;
}
