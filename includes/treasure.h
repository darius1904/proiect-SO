#pragma once

#include <stdint.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define MAX_NAME 24
#define MAX_CLUE 1024
#define MAX_PATH_LENGTH_DIR 256

// GPS Coordinate struct
typedef struct Coordonata {
    float latitude;
    float longitude;
}Coordonate_comoara;

// the struct for the first requirement
typedef struct Treasure {
    char id[MAX_NAME];
    char userName[MAX_NAME];
    Coordonate_comoara GPSCoordinate;
    char clueText[MAX_CLUE];
    int value;
}Treasure_struct;



Treasure_struct* create_treasure();
int directory_exists(const char* dir_path);
int create_directory(const char* dir_path);

//int FILE_create(const char* file_path);

int create_symlink_for_hunt(const char* hunt_id);


// operations

void add_treasure(const char* hunt_id);
void list_treasure(const char* hunt_id);
//void view(const char* hunt_id, const char* id);
//void remove_treasure(const char* hunt_id, const char* id);
//void remove_hunt(const char* hunt_id);
