#define false 0
#define true 1
typedef int bool;

struct square {
  struct ship* partof;
};
struct ship {
  int size;
  int hits;
};

int main (int, char**);
struct square*** initBoard (int, int);
void layoutBoard (struct square***);
bool addShip (struct square***, int);
void setCoordinates (struct square***, int, int, struct ship*);
void printBoard (struct square***, int, int);
int least (int, int);
int most (int, int);
int abs (int);
