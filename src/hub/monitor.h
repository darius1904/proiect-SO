#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>


extern volatile sig_atomic_t running;

ssize_t read_line(int fd, char* buf, size_t max_len);

void setup_signal_handlers(void);


