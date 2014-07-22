#include "game.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv) {
  struct square*** board = initBoard(10, 10);
  layoutBoard(board);
}

struct square*** initBoard (int rows, int cols) {
  struct square*** board = malloc(sizeof(struct square**)*rows);
  int i;
  for (i = 0; i < rows; i++) {
    board[i] = malloc(sizeof(struct square*)*cols);
    int j;
    for (j = 0; j < cols; j++) {
      board[i][j] = malloc(sizeof(struct square));
      (*board[i][j]).partof = 0;
    }
  }
}

void layoutBoard (struct square*** board) {
  do {
    printf("Input coordinates for your aircraft carrier (5 spaces):\n");
  } while(!addShip(5));
  do {
    printf("Input coordinates for your battleship (4 spaces):\n");
  } while (!addShip(4));
  do {
    printf("Input coordinates for your submarine (3 spaces):\n");
  } while (!addShip(3));
  do {
    printf("Input coordinates for your cruiser (3 spaces):\n");
  } while (!addShip(3));
  do {
    printf("Input coordinates for your destroyer (2 spaces):\n");
  } while (!addShip(2));
}

bool addShip (int size) {
  char buffer[2];
  int coordinates[2][2] = {{0, 0}, {0, 0}};
  fgets(buffer, 2, stdin);
  coordinates[0][0] = buffer[0]-'A';
  coordinates[0][1] = buffer[1]-'1';
  fgets(buffer, 2, stdin);
  coordinates[1][0] = buffer[0]-'A';
  coordinates[1][1] = buffer[0]-'1';
  if (!(coordinates[0][0] != coordinates[1][0]) != !(coordinates[0][1] != coordinates[1][1])) {
    if (abs(coordinates[0][0]-coordinates[1][0]) == size) {
      struct ship* partof = malloc(sizeof(struct ship));
      (*partof).size = size;
      (*partof).hits = 0;
      
      return true;
    } else if (abs(coordinates[0][1]-coordinates[1][1]) == size) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void setCoordinates (struct square*** board, int x, int y, struct ship* partof) {
  
}

int abs (int n) {
  if (n < 0) {
    return 0-n;
  }
  return n;
}
