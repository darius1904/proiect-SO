#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include "treasure.h"

#define MAX_USERNAME 32
#define MAX_CLUE 128
#define TREASURE_FILE "treasures.dat"
#define LOG_FILE "logged_hunt"
#define TREASURE_SIZE sizeof(struct Treasure)


int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s --<command> <args>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "--add") == 0 && argc == 3) {
        add_treasure(argv[2]);
    } else if (strcmp(argv[1], "--list") == 0 && argc == 3) {
        list_treasures(argv[2]);
    } else if (strcmp(argv[1], "--view") == 0 && argc == 4) {
        view_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "--remove_treasure") == 0 && argc == 4) {
        remove_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "--remove") == 0 && argc == 3) {
        remove_hunt(argv[2]);
    } else {
        fprintf(stderr, "Unknown command or wrong number of arguments.\n");
    }

    return 0;
}
