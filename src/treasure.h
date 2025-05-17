#pragma once

#include <stdint.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#define MAX_NAME 24
#define MAX_CLUE 1024
#define MAX_PATH_LENGTH_DIR 256

//enum pentru operatii
typedef enum {
    OP_ADD,
    OP_LIST,
    OP_VIEW,
    OP_REMOVE_TREASURE,
    OP_REMOVE_HUNT,
    OP_LIST_EVERYTHING,
    OP_UNKNOWN
} Operation;


//stuctul pentru treasure
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


void add_treasure(const char* hunt_id);
void list_treasure(const char* hunt_id);
void everything_list();
void view(const char* hunt_id, const char* id);
void remove_treasure(const char* hunt_id, const char* id);
void remove_hunt(const char* hunt_id);

void interfata(void);

Treasure_struct* create_treasure();
int directory_exists(const char* dir_path);
int create_directory(const char* dir_path);
int create_symlink_for_hunt(const char* hunt_id);




