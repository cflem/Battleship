#include "game.h"

int main (int argc, char** argv) {
  struct square*** board = initBoard(10, 10);
  layoutBoard(board, 10, 10);
  shot** targeting = initTargeting(10, 10);
  gameLoop(board, targeting, 10, 10);
  return 0;
}

void gameLoop (struct square*** board, shot** targeting, int cols, int rows) {
  int bet = rand(), ships = 5, hsockfd, sockfd, portno = 8644, n;
  char buffer[256];
  bool skip = false;
  bool hosting = false;
  bzero(buffer, 256);
  while (buffer[0] != 'H' && buffer[0] != 'h' && buffer[0] != 'J' && buffer[0] != 'j') {
    printf("Host or Join Game? (H/J) ");
    fgets(buffer, 3, stdin);
  }
  if (buffer[0] == 'H' || buffer[0] == 'h') {
    hosting = true;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    hsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
      err("Error opening socket.");
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(hsockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
      err("Error binding socket.");
    printf("Waiting for opponent...\n");
    listen(hsockfd, 5);
    clilen = sizeof(cli_addr);
    sockfd = accept(hsockfd, (struct sockaddr*) &cli_addr, &clilen);
    if (sockfd < 0)
      err("Error accepting client.");
    bzero(buffer, 256);
    snprintf(buffer, 255, "BET %d\n", bet);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
      err("Error writing to socket.");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0)
      err("Error reading from socket.");
    if (strncmp(buffer, "FOLD", 4) == 0) {
      skip = true;
    }
  } else {
    bzero(buffer, 256);
    printf("Input host address:\n");
    fgets(buffer, 255, stdin);
    trim(buffer);
    struct sockaddr_in serv_addr;
    struct hostent* server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
      err("Error opening socket.");
    server = gethostbyname(buffer);
    if (server == NULL)
      err("Error, no such host.");
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
      err("Error connecting.");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0)
      err("Error reading from socket.");
    if (strncmp(buffer, "BET ", sizeof(char)*4) != 0)
      err("Protocol error.");
    int tobeat = atoi(buffer+(sizeof(char)*4));
    bzero(buffer, 256);
    if (bet > tobeat) {
      snprintf(buffer, 255, "RAISE %d\n", bet);
      n = write(sockfd, buffer, strlen(buffer));
      if (n < 0)
        err("Error writing to socket.");
      skip = true;
    } else {
      strncpy(buffer, "FOLD\n", 4);
      n = write(sockfd, buffer, strlen(buffer));
      if (n < 0)
        err("Error writing to socket.");
    }
  }

  while (true) {

    // THEIR TURN
    if (!skip) {
      printf("\nWaiting for opponent...\n");
      bzero(buffer, 256);
      n = read(sockfd, buffer, 255);
      if (n < 0)
        err("Error reading from socket.");
      if (strncmp(buffer, "FIRE ", 5) != 0)
        err("Protocol error.");
      char a = buffer[5], b = buffer[6];
      int x = b - '0', y = a - 'A';
      if ((*board[x][y]).partof != 0) {
        (*board[x][y]).ishit = true;
        if (++(*(*board[x][y]).partof).hits == (*(*board[x][y]).partof).size) {
          if (--ships == 0) {
            n = write(sockfd, "UNCLE\n", 6);
            clear();
            printGame(board, targeting, cols, rows);
            printf("\n\033[1;31mSorry, you lose...\033[0m\n");
            close(sockfd);
            if (hosting)
              close(hsockfd);
            return;
          } else {
            bzero(buffer, 256);
            snprintf(buffer, 255, "SUNK %d\n", (*(*board[x][y]).partof).type);
            n = write(sockfd, buffer, strlen(buffer));
            clear();
            printGame(board, targeting, cols, rows);
            printf("\n\033[1;31mYour %s has been sunk!\033[0m\n", shiptype((*(*board[x][y]).partof).type));
          }
        } else {
          n = write(sockfd, "HIT\n", 4);
          clear();
          printGame(board, targeting, cols, rows);
          printf("\n\033[1;31mYour %s has been hit!\033[0m\n", shiptype((*(*board[x][y]).partof).type));
        }
      } else {
        n = write(sockfd, "MISS\n", 5);
        clear();
        printGame(board, targeting, cols, rows);
        printf("\nYour opponent missed.\n");
      }
      (*board[x][y]).ishit = true;
      if (n < 0)
        err("Error writing to socket.");
    }

    // YOUR TURN
    if (skip) {
      clear();
      printGame(board, targeting, cols, rows);
    }
    char a, b;
    do {
      printf("\n\nFire when ready:\n");
      bzero(buffer, 256);
      fgets(buffer, 4, stdin);
      a = buffer[0];
      b = buffer[1];
    } while ((a-'A') < 0 || (a-'A') >= rows || (b-'0') < 0 || (b-'0') >= cols);
    bzero(buffer, 256);
    snprintf(buffer, 255, "FIRE %c%c\n", a, b);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
      err("Error writing to socket.");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0)
      err("Error reading from socket.");
    int x = b - '0', y = a - 'A';
    if (strncmp(buffer, "HIT", 3) == 0) {
      targeting[x][y] = hit;
      printf("\n\033[1;32mIt's a hit!\033[0m\n");
    } else if (strncmp(buffer, "SUNK ", 5) == 0) {
      targeting[x][y] = hit;
      printf("\n\033[1;32mYou sunk your opponent's %s!\033[0m\n", shiptype(buffer[5]-'0'));
    } else if (strncmp(buffer, "UNCLE", 5) == 0) {
      targeting[x][y] = hit;
      clear();
      printGame(board, targeting, cols, rows);
      printf("\n\033[1;32mCongradulations, you win!\033[0m\n");
      close(sockfd);
      if (hosting)
        close(hsockfd);
      return;
    } else if (strncmp(buffer, "MISS", 4) == 0) {
      targeting[x][y] = miss;
      printf("\nYou missed...\n");
    } else {
      err("Protocol error.");
    }
    skip = false;

  }
}

const char* shiptype (int type) {
  switch (type) {
    case carrier:
      return "aircraft carrier";
    case battleship:
      return "battleship";
    case submarine:
      return "submarine";
    case cruiser:
      return "cruiser";
    case destroyer:
      return "destroyer";
    default:
      return "ship";
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
      (*board[i][j]).ishit = false;
    }
  }
  return board;
}

void layoutBoard (struct square*** board, int cols, int rows) {
  clear();
  printBoard(board, cols, rows);
  do {
    printf("Input coordinates for your aircraft carrier (5 spaces):\n");
  } while(!addShip(board, cols, rows, carrier, 5));

  clear();
  printBoard(board, cols, rows);
  do {
    printf("Input coordinates for your battleship (4 spaces):\n");
  } while (!addShip(board, cols, rows, battleship, 4));

  clear();
  printBoard(board, cols, rows);
  do {
    printf("Input coordinates for your submarine (3 spaces):\n");
  } while (!addShip(board, cols, rows, submarine, 3));

  clear();
  printBoard(board, cols, rows);
  do {
    printf("Input coordinates for your cruiser (3 spaces):\n");
  } while (!addShip(board, cols, rows, cruiser, 3));

  clear();
  printBoard(board, cols, rows);
  do {
    printf("Input coordinates for your destroyer (2 spaces):\n");
  } while (!addShip(board, cols, rows, destroyer, 2));

  clear();
  printBoard(board, cols, rows);
}

bool addShip (struct square*** board, int cols, int rows, int type, int size) {
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

  // Bounds Checking
  if ((!(coordinates[0][0] != coordinates[1][0]) != !(coordinates[0][1] != coordinates[1][1])) && (coordinates[0][0] < cols && coordinates[0][0] >= 0 && coordinates[0][1] < rows && coordinates[0][1] >= 0 && coordinates[1][0] < cols && coordinates[1][0] >= 0 && coordinates[1][1] < rows && coordinates[1][1] >= 0)) {
    if (abs(coordinates[0][0]-coordinates[1][0]) == (size-1)) {
      struct ship* partof = malloc(sizeof(struct ship));
      (*partof).size = size;
      (*partof).hits = 0;
      (*partof).type = type;
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
        if ((*board[x][y]).ishit)
          printf("\033[31mX\033[0m ");
        else
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
        if ((*board[x][y]).ishit)
          printf("\033[31mX\033[0m ");
        else
          printf("\033[32mX\033[0m ");
      } else {
        printf("O ");
      }
    }
    printf("\n\033[0m");
    c++;
  }
}

void err (const char* msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_SUCCESS);
}

void clear () {
  system("clear");
}

char* trim (char* str) {
  char* end;
  while (isspace(*str)) str++;
  if ((*str) == 0)
    return str;
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;
  *(end+1) = 0;
  return str;
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
