#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "board_library.h"


int linear_conv(int dim_board,int i, int j){
  return j*dim_board+i;
}


void matrix_conv(int dim_board, int count, int *i, int *j){
	*i = count%dim_board;
	*j = count/dim_board;
	return;
}


char * get_board_place_str(board_place *board, int dim, int i, int j){
  return board[linear_conv(dim, i, j)].v;
}


int *shuffled_array(int dim){
	int i, temp, randomIndex;
	int *array = (int *) malloc(dim*sizeof(int));
	
	for(i = 0; i<dim; i++)		// fill array
		array[i] = i;
	
	for(i = 0; i<dim; i++){		// shuffle array
		temp = array[i];
		randomIndex = rand() % dim;

		array[i] = array[randomIndex];
		array[randomIndex] = temp;
	}

	return array;
}
