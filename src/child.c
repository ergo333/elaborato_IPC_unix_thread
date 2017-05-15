#include "child.h"
#include "utils.h"

void execute(int pipe_fd){
short termina = 0;

int shmem_A, shmem_B, shmem_C, shmem_somma;
int sem;
int riga, colonna, ordine, cmd;
int res;
int msgid;
char c[64];

	res = msgget(MSG_KEY, 0666);
	if (res == -1){
		stampa("Errore nella creazione della coda messaggi\n");
		exit(1);
	}

	shmem_A = shmget(SHM_KEY_A, sizeof(int[ordine][ordine]), 0666); 	
	shmem_B = shmget(SHM_KEY_B, sizeof(int[ordine][ordine]), 0666); 
	shmem_C = shmget(SHM_KEY_C, sizeof(int[ordine][ordine]), 0666);
	shmem_somma = shmget(SHM_KEY_SOMMA, sizeof(int), 0666);
	sem = semget(SEM_KEY, 1, 0666);

	if (shmem_A == -1){
		stampa("Errore durante l'allocazione di memoria condivisa per matrice A (child)\n");
		exit(1);
	}
	if (shmem_B == -1){
		stampa("Errore durante l'allocazione di memoria condivisa per matrice B (child)\n");
		exit(1);
	}
	if (shmem_C == -1){
		stampa("Errore durante l'allocazione di memoria condivisa per matrice C (child)\n");
		exit(1);
	}
	if (sem == -1){
		stampa("Errore durante semget (child)\n");
		exit(1);
	}

	int * mat_A = shmat(shmem_A, NULL, 0);
	if ((void*)mat_A == -1){
		stampa("Errore attach memoria A (child)\n");
		exit(1);
	}
	int * mat_B = shmat(shmem_B, NULL, 0);
	if ((void*)mat_B == -1){
		stampa("Errore attach memoria B (child)\n");
		exit(1);
	}
	int * mat_C = shmat(shmem_C, NULL, 0);
	if ((void*)mat_C == -1){
		stampa("Errore attach memoria C (child)\n");
		exit(1);
	}

	int * sum = shmat(shmem_somma, NULL, 0);
	if ((void*)sum == -1){
		stampa("Errore attach memoria somma (child)\n");
		exit(1);
	}

	msgid = msgget(MSG_KEY, 0666);
	if (msgid == -1){
		stampa("Errore msgget child\n");
		exit(1);
	}

	while(!termina){

		int n = read(pipe_fd, c, 64);
		c[n] = '\0';


		estrai_dati(&cmd, &riga, &colonna, &ordine, c);

		if (cmd == CHILD_MOLTIPLICA){ 
	
			int mul = moltiplica(mat_A, mat_B, riga, colonna, ordine);

			//inserisco il risultato in c
			mat_C[riga*ordine + colonna] = mul;

			message msg;
			msg.mtype = MSG_TYPE;
			msg.pid = getpid();
			msg.riga = riga;
			msg.colonna = colonna;
			msg.operation = CHILD_MOLTIPLICA;

			msgsnd(msgid, &msg, sizeof(message) - sizeof(long), MSG_TYPE);

		}
		if (cmd == CHILD_SOMMA){

			struct sembuf operation;

			//operation init

			operation.sem_num = 0;		//primo ed unico semaforo
			operation.sem_op = -1;		//P
			operation.sem_flg = 0;		//nessuna flag settata

			int val = somma(mat_C, riga, ordine);

			semop(sem, &operation, 1);		//esecuzione dell'operazione
			// P bloccante
			*sum += val;
			operation.sem_op = 1;
			semop(sem, &operation, 1);	//sblocco il semaforo per altre eventuali somme

			message msg;
			msg.mtype = MSG_TYPE;
			msg.pid = getpid();
			msg.riga = riga;
			msg.colonna = 0;
			msg.operation = CHILD_SOMMA;

			msgsnd(msgid, &msg, sizeof(message) - sizeof(long), MSG_TYPE);
			
		}
		if (cmd == CHILD_EXIT)
			termina = 1;
	}

	shmdt(shmem_A);	
	shmdt(shmem_B);	
	shmdt(shmem_C);
	shmdt(sum);

}

int somma(int * matrix, int row, int ordine){

int j;
int sum = 0;

	for (j = 0; j < ordine; j++){
		sum += matrix[(row * ordine) + j];
	}
	return sum;
}

int moltiplica(int * a, int * b, int row_a, int column_b, int ordine){    // riga A * riga B

int result = 0;
int i;

	for (i = 0; i < ordine; i++){
		result += a[(ordine * row_a) + i] * b[(i * ordine) + column_b];
	}
	return result;
}

void estrai_dati(int * cmd, int * riga, int * colonna, int * ordine, char * msg){

	*cmd = atoi(strtok(msg, ";"));
	*riga = atoi(strtok(NULL, ";"));
	*colonna = atoi(strtok(NULL, ";"));
	*ordine = atoi(strtok(NULL, ";"));

}