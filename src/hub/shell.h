#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_ARGS 10


typedef enum {
  CMD_INVALID,
  CMD_CLEAR,
  CMD_HELP,
  CMD_EXIT,
  CMD_START_MONITOR,
  CMD_STOP_MONITOR,
  CMD_LIST_HUNTS,
  CMD_LIST_TREASURES,
  CMD_VIEW_TREASURE,
  CMD_CALCULATE_SCORES
} shell_command;  //lista de comenzi





typedef struct {
  pid_t pid;
  enum { MONITOR_OFFLINE, MONITOR_ONLINE, MONITOR_SHUTTING_DOWN } state;
  int read_pipe_fd, write_pipe_fd;
} monitor_t;

extern monitor_t monitor;

monitor_t monitor = { .pid = -1,
                     .state = MONITOR_OFFLINE,
                     .read_pipe_fd = -1,
                     .write_pipe_fd = -1 };


int is_monitor_alive(void);
int send_to_monitor(const char* cmd_line);
int cmd_start_monitor(void);
int cmd_stop_monitor(void);
int check_monitor_stopping(void);
