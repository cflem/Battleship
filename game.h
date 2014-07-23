#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define carrier 5
#define battleship 4
#define submarine 3
#define cruiser 2
#define destroyer 1

#define null 0
#define miss 1
#define hit 2
typedef int shot;

#define false 0
#define true 1
typedef int bool;

struct square {
  struct ship* partof;
  bool ishit;
};
struct ship {
  int size;
  int hits;
  int type;
};

int main (int, char**);
void gameLoop (struct square*** board, shot** targeting, int cols, int rows);
const char* shiptype (int);
shot** initTargeting (int, int);
struct square*** initBoard (int, int);
void layoutBoard (struct square***);
bool addShip (struct square***, int, int);
void setCoordinates (struct square***, int, int, struct ship*);
void printGame (struct square***, shot**, int, int);
void printTargeting (shot**, int, int);
void printBoard (struct square***, int, int);
void err (const char*);
void clear ();
char* trim (char*);
int least (int, int);
int most (int, int);
int abs (int);
