#include "utils.h"

void stampa(char* buff){
	write(1, buff, strlen(buff));
}

void read_matrix(int fd, int ordine, int ** matrix){
int i = 0, j = 0;
int n;	 	//num byte letti 
char * buffer = malloc(NUM_BYTE * sizeof(char));


	while((n = read_line(fd, buffer, NUM_BYTE)) > 0){
		buffer[n] = '\0';
		char * token = strtok(buffer, DELIMITER);  //prendo il primo token

		for (j = 0; j < ordine; j++){
			matrix[i][j] = atoi(token);
			token = strtok(NULL, DELIMITER);
		}	

		i++;
	}
	free(buffer);

}

void print_matrix(int ** matrix, int ordine){

int i, j;
char buff[64] = "";

	for (i = 0; i < ordine; i++){
		for (j = 0; j < ordine; j++){
			sprintf(buff, "%d ", matrix[i][j]);
			stampa(buff);
		}
		stampa("\n");
	}
}

int read_line(int fd, char * buff, int max_len){

int i = 0;
char c;
ssize_t n;

	while((n = read(fd, &c, 1)) > 0){		//leggo un carattere alla volta

		if (c == '\n')						//fine riga
			return i; 
		
		if (i == max_len)
			return i;

		buff[i++] = c;
	}
	return i;								//EOF
}

int ** create_matrix(int order){
	
int i;
int ** matrix = calloc(order, sizeof(int *));
	for (i = 0; i < order; i++)
		matrix[i] = calloc(order, sizeof(int));
	return matrix;
}

void free_matrix(int ** matrix, int order){
int i;

	for (i = 0; i < order; i++){
		free(matrix[i]);
	}

	free(matrix);

}

void load_matrix(int ** matrix, int * array2d, int order){

int i, j;
int k=0;

	for (i = 0; i < order; i++)
		for (j = 0; j < order; j++)
			array2d[k++] = matrix[i][j];
}

void set_matrix(int *array2d, int order){
int k;

	for (k = 0; k < order * order; k++)
		array2d[k] = 0;
}

void print_matrix_2d(int * matrix, int ordine){
int i, j;
char buff[64] = "";

	for (i = 0; i < ordine; i++){
		for (j = 0; j < ordine; j++){
			sprintf(buff, "%d ", matrix[(i * ordine) + j]);
			stampa(buff);
		}
		stampa("\n");
	}
}

int position(int * vec, int elem, int len){

int i;
	
	for (i = 0; i < len; i++){
		if (vec[i] == elem)
			return i;
	}
	return -1;
}

void write_matrix_(int fd, int ** matrix, int ord){

int i, j;
char buff[64];

	for (i = 0; i < ord; i++){
		for (j = 0; j < ord - 1; j++){
			sprintf(buff, "%d;", matrix[i][j]);
			write(fd, buff, strlen(buff));
		}

		sprintf(buff, "%d\n", matrix[i][j]);
		write(fd, buff, strlen(buff));
	}
}

int get_next_row(int ** control_matrix, int order){

int i, j;
short found = 0;

	for (i = 0; i < order && !found; i++){

		found = 1;
		for (j = 0; j < order; j++){

			//se la riga non è ancora stata calcolata, o se è già stata sommata
			//considero la riga successiva
			if (control_matrix[i][j] != 1){	
				found = 0;	
				break;
			}
		}
	}
	if (found)
		return i-1;

	return -1; //finish!!
}

void set_row(int ** control_matrix, int order, int row, int value){

int j;
	
	for (j = 0; j < order; j++)
		control_matrix[row][j] = value;
}

void genera_parametri(int riga, int colonna, int ordine, int cmd, char * buff){

	sprintf(buff, "%d;%d;%d;%d", cmd, riga, colonna, ordine);

}
