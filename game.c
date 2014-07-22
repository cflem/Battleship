#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "game.h"

int main (int argc, char** argv) {
  struct square*** board = initBoard(10, 10);
  layoutBoard(board);
  shot** targeting = initTargeting(10, 10);
  gameLoop(board, targeting, 10, 10);
}

void gameLoop (struct square*** board, shot** targeting, int cols, int rows) {
  int bet = rand();
  char buffer[256];
  bzero(buffer, 256);
  while (buffer[0] != 'H' && buffer[0] != 'h' && buffer[0] != 'J' && buffer[0] != 'j') {
    printf("Host or Join Game? (H/J) ");
    fgets(buffer, 3, stdin);
  }
  if (buffer[0] == 'H' || buffer[0] == 'h') {
    
  } else {
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;
    sockfd = socket(AF_INET, SOCK_STREAM);
    if (sockfd < 0) {
      fprintf(stderr, "Error opening socket.");
      exit(EXIT_FAILURE);
    }
    server = gethostbyname(buffer);
    if (server == NULL) {
      fprintf(stderr, "Error, no such host.");
      exit(EXIT_SUCCESS);
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(PORTNO);
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
      fprintf(stderr, "Error connecting.");
      exit(EXIT_SUCCESS);
    }
    n = read(sockfd, buffer, 255);
    if (n < 0) {
      fprintf(stderr, "Error reading from socket.");
      exit(EXIT_SUCCESS);
    }
    if (strncmp(buffer, "BET ", sizeof(char)*4) != 0) {
      fprintf(stderr, "Server protocol error.");
      exit(EXIT_SUCCESS);
    }
    int tobeat = atoi(buffer+(sizeof(char)*4));
    bzero(buffer, 256);
    snprintf(buffer, 255, "RAISE %d\n", bet);
    if (bet > tobeat) {

    } else {

    }
  }
}

shot** initTargeting (int cols, int rows) {
  shot** targeting = malloc(sizeof(shot*)*cols);
  int i;
  for (i = 0; i < cols; i++) {
    targeting[i] = malloc(sizeof(shot)*rows);
    int j;
    for (j = 0; j < rows; j++) {
      targeting[i][j] = null;
    }
  }
  return targeting;
}

struct square*** initBoard (int cols, int rows) {
  struct square*** board = malloc(sizeof(struct square**)*cols);
  int i;
  for (i = 0; i < cols; i++) {
    board[i] = malloc(sizeof(struct square*)*rows);
    int j;
    for (j = 0; j < rows; j++) {
      board[i][j] = malloc(sizeof(struct square));
      (*board[i][j]).partof = 0;
    }
  }
  return board;
}

void layoutBoard (struct square*** board) {
  printBoard(board, 10, 10);
  do {
    printf("Input coordinates for your aircraft carrier (5 spaces):\n");
  } while(!addShip(board, 5));
  printBoard(board, 10, 10);

  do {
    printf("Input coordinates for your battleship (4 spaces):\n");
  } while (!addShip(board, 4));
  printBoard(board, 10, 10);

  do {
    printf("Input coordinates for your submarine (3 spaces):\n");
  } while (!addShip(board, 3));
  printBoard(board, 10, 10);

  do {
    printf("Input coordinates for your cruiser (3 spaces):\n");
  } while (!addShip(board, 3));
  printBoard(board, 10, 10);

  do {
    printf("Input coordinates for your destroyer (2 spaces):\n");
  } while (!addShip(board, 2));
  printBoard(board, 10, 10);
}

bool addShip (struct square*** board, int size) {
  char buffer[4] = {0, 0, 0, 0};
  int coordinates[2][2] = {{0, 0}, {0, 0}};
  printf("Start coordinate:\n");
  fgets(buffer, 4, stdin);
  coordinates[0][1] = buffer[0]-'A';
  coordinates[0][0] = buffer[1]-'0';
  printf("End coordinate:\n");
  fgets(buffer, 4, stdin);
  coordinates[1][1] = buffer[0]-'A';
  coordinates[1][0] = buffer[1]-'0';
  if (!(coordinates[0][0] != coordinates[1][0]) != !(coordinates[0][1] != coordinates[1][1])) {
    if (abs(coordinates[0][0]-coordinates[1][0]) == (size-1)) {
      struct ship* partof = malloc(sizeof(struct ship));
      (*partof).size = size;
      (*partof).hits = 0;
      int y = coordinates[0][1];
      int x;
      for (x = least(coordinates[0][0], coordinates[1][0]); x <= most(coordinates[0][0], coordinates[1][0]); x++) {
        setCoordinates(board, x, y, partof);
      }
      return true;
    } else if (abs(coordinates[0][1]-coordinates[1][1]) == (size-1)) {
      struct ship* partof = malloc(sizeof(struct ship));
      (*partof).size = size;
      (*partof).hits = 0;
      int x = coordinates[0][0];
      int y;
      for (y = least(coordinates[0][1], coordinates[1][1]); y <= most(coordinates[0][1], coordinates[1][1]); y++) {
        setCoordinates(board, x, y, partof);
      }
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void setCoordinates (struct square*** board, int x, int y, struct ship* partof) {
  (*board[x][y]).partof = partof;
}

void printGame (struct square*** board, shot** targeting, int cols, int rows) {
  int x, y, r, c = 65;
  printf("\033[0m  \033[31m");
  for (r = 0; r < cols; r++) {
    printf("%d ", r);
  }
  printf("\033[0m          \033[34m");
  for (r = 0; r < cols; r++) {
    printf("%d ", r);
  }
  printf("\033[0m\n");
  for (y = 0; y < rows; y++) {
    printf("\033[31m%c\033[0m ", c);
    for (x = 0; x < cols; x++) {
      if ((*board[x][y]).partof != 0) {
        printf("\033[32mX\033[0m ");
      } else {
        printf("O ");
      }
    }
    printf("\033[0m        ");
    printf("\033[34m%c\033[0m ", c);
    for (x = 0; x < cols; x++) {
      if (targeting[x][y] == hit) {
        printf("\033[31mX\033[0m ");
      } else if (targeting[x][y] == miss) {
        printf("O ");
      } else {
        printf("  ");
      }
    }
    printf("\n\033[0m");
    c++;    
  }
}

void printBoard (struct square*** board, int cols, int rows) {
  int x, y, r, c = 65;
  printf("\033[0m  \033[31m");
  for (r = 0; r < cols; r++) {
    printf("%d ", r);
  }
  printf("\033[0m\n");
  for (y = 0; y < rows; y++) {
    printf("\033[31m%c\033[0m ", c);
    for (x = 0; x < cols; x++) {
      if ((*board[x][y]).partof != 0) {
        printf("\033[32mX\033[0m ");
      } else {
        printf("O ");
      }
    }
    printf("\n\033[0m");
    c++;
  }
}

int least (int a, int b) {
  if (a < b) {
    return a;
  }
  return b;
}

int most (int a, int b) {
  if (a > b) {
    return a;
  }
  return b;
}

int abs (int n) {
  if (n < 0) {
    return 0-n;
  }
  return n;
}
