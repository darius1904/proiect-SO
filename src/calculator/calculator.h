#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
 

#define MAX_NAME 24
#define MAX_CLUE 1024

typedef struct {
  char user[100];
  int total_score;
} user_score_t;



typedef struct Treasure {
    char id[MAX_NAME];
    char userName[MAX_NAME];
    struct {
        float latitude;
        float longitude;
    }GPSCoordinate;
    char clueText[MAX_CLUE];
    int value;
}Treasure_struct;

