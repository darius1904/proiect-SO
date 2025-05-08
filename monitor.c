#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "treasure.h"

volatile sig_atomic_t got_signal = 0;

void handle_sigusr1(int sig) {
    got_signal = 1;
}

void process_command() {
    FILE *fp = fopen("monitor_cmd.txt", "r");
    if (!fp) {
        perror("monitor: Failed to read command file");
        return;
    }

    char command[128];
    if (!fgets(command, sizeof(command), fp)) {
        fclose(fp);
        return;
    }
    fclose(fp);
    command[strcspn(command, "\n")] = 0;

    if (strcmp(command, "list_hunts") == 0) {
        printf("[monitor] Listing hunts:\n");
        DIR *dir = opendir(".");
        if (!dir) {
            perror("monitor: Failed to open current directory");
            return;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && 
                strcmp(entry->d_name, ".") != 0 && 
                strcmp(entry->d_name, "..") != 0) {

                char treasure_path[512];
                snprintf(treasure_path, sizeof(treasure_path), "%s/%s", entry->d_name, TREASURE_FILE);

                int count = 0;
                int fd = open(treasure_path, O_RDONLY);
                if (fd >= 0) {
                    struct Treasure t;
                    while (read(fd, &t, TREASURE_SIZE) == TREASURE_SIZE) {
                        count++;
                    }
                    close(fd);
                }

                printf("Hunt: %s | Treasures: %d\n", entry->d_name, count);
            }
        }
        closedir(dir);

    } else if (strncmp(command, "list_treasures", 14) == 0) {
        char hunt_id[64];
        sscanf(command, "list_treasures %63s", hunt_id);
        list_treasures(hunt_id);

    } else if (strncmp(command, "view_treasure", 13) == 0) {
        char hunt_id[64];
        int tid;
        sscanf(command, "view_treasure %63s %d", hunt_id, &tid);
        view_treasure(hunt_id, tid);

    } else if (strcmp(command, "stop_monitor") == 0) {
        printf("[monitor] Stopping in 3 seconds...\n");
        usleep(3000000); // 3-second delay
        exit(0);
    } else {
        printf("[monitor] Unknown command: %s\n", command);
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("monitor: sigaction failed");
        exit(1);
    }

    printf("[monitor] Ready. Waiting for commands...\n");

    while (1) {
        pause(); // wait for signal
        if (got_signal) {
            got_signal = 0;
            process_command();
        }
    }

    return 0;
}