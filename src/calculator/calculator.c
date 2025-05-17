#include "calculator.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MAX_USERS 100
#define MAX_PATH_LEN 1000

// Read one Treasure_struct from fd into t, returns 1 on success, 0 on EOF or error
int read_treasure_from_file(Treasure_struct *t, int fd) {
    ssize_t bytes = read(fd, t, sizeof(Treasure_struct));
    if (bytes == sizeof(Treasure_struct))
        return 1;
    if (bytes == 0)
        return 0;

    perror("eroare la citire");
    return 0;
}

// Find user index in the array, or -1 if not found
static int find_user_index(const user_score_t *us, int size, const char *username) {
    for (int i = 0; i < size; i++) {
        if (strcmp(us[i].user, username) == 0)
            return i;
    }
    return -1;
}

// Compute the user scores from all treasures in fd, results stored in us[], size updated
void compute_scores(user_score_t *us, int *size, int fd) {
    int count = 0;
    Treasure_struct treasure;

    while (read_treasure_from_file(&treasure, fd)) {
        int idx = find_user_index(us, count, treasure.userName);
        if (idx >= 0) {
            us[idx].total_score += treasure.value;
        } else if (count < MAX_USERS) {
            snprintf(us[count].user, sizeof(us[count].user), "%s", treasure.userName);
            us[count].total_score = treasure.value;
            count++;
        }
    }

    *size = count;
}

// Print all user scores
void print_scores(const user_score_t *us, int size) {
    for (int i = 0; i < size; i++) {
        printf("UserName: %5s            Scorul: %d\n", us[i].user, us[i].total_score);
    }
}

// Extract the target hunt name from a path like "treasure_hunts/hunt_name/treasures.bin"
void get_target_hunt(char *target, const char *path) {
    char path_copy[MAX_PATH_LEN];
    snprintf(path_copy, sizeof(path_copy), "%s", path);

    // Find first slash
    char *first_slash = strchr(path_copy, '/');
    if (!first_slash) {
        target[0] = '\0';
        return;
    }
    first_slash++; // move past first slash

    // Find second slash
    char *second_slash = strchr(first_slash, '/');
    if (!second_slash) {
        snprintf(target, MAX_PATH_LEN, "%s", first_slash);
        return;
    }

    *second_slash = '\0'; // terminate at second slash
    snprintf(target, MAX_PATH_LEN, "%s", first_slash);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Nu sunt bune argumentele la calculator!");
        return 1;
    }

    char target_hunt[MAX_PATH_LEN];
    get_target_hunt(target_hunt, argv[1]);
    printf("\x1b[95mHunt: %s\x1b[0m\n", target_hunt);

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        return 1;
    }

    user_score_t users[MAX_USERS] = {0};
    int size = 0;

    compute_scores(users, &size, fd);
    print_scores(users, size);

    close(fd);
    return 0;
}

