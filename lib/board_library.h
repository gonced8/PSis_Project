#ifndef BOARD_LIBRARY_H
#define BOARD_LIBRARY_H

#include <stdlib.h>

typedef struct board_place{
  char v[3];
} board_place;

typedef struct play_response{
  int code; // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -2 2nd - diffrent
  int play1[2];
  int play2[2];
  int play1_int[2];
  int timer5sec_init;
  char str_play1[3], str_play2[3];
} play_response;

int linear_conv(int dim_board,int i, int j);
void matrix_conv(int dim_board, int count, int *i, int *j);
char *get_board_place_str(board_place *board, int dim, int i, int j);
int *shuffled_array(int dim);
// void board_play (int x, int y, play_response *rsp);

#endif /* BOARD_LIBRARY_H */
