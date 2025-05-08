#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

pid_t monitor_pid = -1;
volatile sig_atomic_t monitor_exited = 0;

void sigchld_handler(int sig) {
    int status;
    waitpid(monitor_pid, &status, 0);
    monitor_exited = 1;
    printf("Monitor exited with status %d\n", WEXITSTATUS(status));
}

void send_command_to_monitor(const char *command) {
    // Write command to a temporary file
    FILE *fp = fopen("monitor_cmd.txt", "w");
    if (!fp) {
        perror("Failed to write command");
        return;
    }
    fprintf(fp, "%s\n", command);
    fclose(fp);

    // Send signal to monitor
    kill(monitor_pid, SIGUSR1);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char input[128];
    while (1) {
        printf("hub> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "start_monitor") == 0) {
            if (monitor_pid > 0 && !monitor_exited) {
                printf("Monitor already running.\n");
                continue;
            }

            monitor_exited = 0;
            monitor_pid = fork();
            if (monitor_pid == 0) {
                execl("./monitor", "monitor", NULL);
                perror("Failed to start monitor");
                exit(1);
            }

        } else if (strcmp(input, "list_hunts") == 0 || 
                   strcmp(input, "list_treasures") == 0 ||
                   strncmp(input, "view_treasure", 13) == 0) {

            if (monitor_pid > 0 && !monitor_exited) {
                send_command_to_monitor(input);
            } else {
                printf("Error: Monitor is not running.\n");
            }

        } else if (strcmp(input, "stop_monitor") == 0) {
            if (monitor_pid > 0 && !monitor_exited) {
                send_command_to_monitor("stop_monitor");
                printf("Sent stop signal. Waiting for monitor to exit...\n");
            } else {
                printf("Monitor not running.\n");
            }

        } else if (strcmp(input, "exit") == 0) {
            if (monitor_pid > 0 && !monitor_exited) {
                printf("Error: Monitor still running. Use stop_monitor first.\n");
            } else {
                break;
            }
        } else {
            printf("Unknown command.\n");
        }

        // If monitor is stopping, block other commands
        if (!monitor_exited && monitor_pid > 0) {
            usleep(100000); // simulate delay
        }
    }

    return 0;
}