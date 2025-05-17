#include "monitor.h"


char line[10][100];



int split_into_words(char* buf, char line[10][100]) 
{
    int word_index = 0;
    int char_index = 0;

    for (int i = 0; buf[i] != '\0' && word_index < 3; ++i) {
        if (isspace((unsigned char)buf[i])) {
            if (char_index > 0) {
                line[word_index][char_index] = '\0';
                word_index++;
                char_index = 0;
            }
        }
        else {
            if (char_index < 99) {
                line[word_index][char_index++] = buf[i];
            }
        }
    }

    // Handle the last word if buffer didn't end with whitespace
    if (char_index > 0 && word_index < 3) {
        line[word_index][char_index] = '\0';
        word_index++;
    }

    // Remove newline from the last word if present
    if (word_index > 0) {
        int last = word_index - 1;
        size_t len = strlen(line[last]);
        if (len > 0 && line[last][len - 1] == '\n') {
            line[last][len - 1] = '\0';
        }
    }

    return word_index;
}


ssize_t read_line(int fd, char* buf, size_t max_len) {
    size_t i = 0;
    char ch;
    ssize_t n;

    while (i < max_len - 1) {
        n = read(fd, &ch, 1);

        if (n == 0) {
            break; // EOF
        }
        else if (n < 0) {
            return -1; // read error
        }

        buf[i++] = ch;

        if (ch == '\n') {
            break;
        }
    }

    buf[i] = '\0';
    return i;
}













void handle_sigterm(int signum)   //aici se opreste copilul intra aici la SIGTERM
{
    char msg[BUFSIZ];
    write(STDERR_FILENO, "\r\033[K", 4); 
    snprintf(msg, sizeof(msg), "=Monitor(copil)= Am primit semnalul de shutdown. (signal=%d)\n",signum);
    write(STDERR_FILENO, msg, strlen(msg));

    running = 0;
}

void setup_signal_handlers(void) 
{
    struct sigaction sa_term = { 0 };
    struct sigaction sa_child = { 0 };

    // Shutdown signals
    sa_term.sa_handler = handle_sigterm;  //That means when your monitor gets a shutdown signal, it will run handle_sigterm(int signum)
    sigaction(SIGTERM, &sa_term, NULL);  //sent by kill or system shutdown
    sigaction(SIGINT, &sa_term, NULL);   //sent by Ctrl+C
    sigaction(SIGHUP, &sa_term, NULL);  //often sent when terminal is closed

    // Prevent child zombies
    sa_child.sa_handler = SIG_IGN;
    sa_child.sa_flags = SA_NOCLDWAIT; //This part tells the monitor to ignore SIGCHLD, which is sent when a child process finishes copii monitorului de fapt care nu ar trb sa ramana. noi de aici vom rula treasure_hunter.
    sigaction(SIGCHLD, &sa_child, NULL);  
}



















void read_data_from_treasure_manager(int fd) 
{
    char buffer[BUFSIZ];
    ssize_t n;
    while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        write(STDOUT_FILENO, buffer, n); //scrie in pipe data  monitor_to_shell_pipefd[1] (la output monitor)
    }
}





void execute_manager(const char line[10][100], int count) 
{

    char msg[BUFSIZ];

    int pipefd[2];  //pipeurile pentru treasure_hunter/manager
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) 
    {
        write(STDOUT_FILENO, "\r\033[K", 4); //erase from the cursor to the end of the line.

        sprintf(msg, "=Monitor(copil)= \033[31mA esuat fork: %s\033[0m\n", strerror(errno));
        write(STDERR_FILENO, msg, strlen(msg));
        return;
    }

    if (pid == 0)  //copilul
    {
        close(pipefd[0]);               // Close read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipefd[1]);

      /*  if (count == 1)
        {
            //printf("\nprimul argument:%s, al doilea argument %sconi\n",line[0],line[1]);
            execlp("./treasure_hunter", "treasure_hunter", line[0], (char*)NULL);
        } */
        
        if (count == 1)
        {
            //printf("\nprimul argument:%s, al doilea argument %sconi\n",line[0],line[1]);
            execlp("./treasure_hunter", "treasure_hunter", line[0], (char*)NULL);
        }
        else  if (count == 2)
        {
            //printf("\nprimul argument:%s, al doilea argument %sconi\n",line[0],line[1]);
            execlp("./treasure_hunter", "treasure_hunter", line[0], line[1], (char*)NULL);
        }
        else if (count == 3)
        {
            execlp("./treasure_hunter", "treasure_hunter", line[0], line[1], line[2], (char*)NULL);
        }
        else
        {
            perror("ERORR. Daca sa ajuns cu eroarea aici nu este ok!!!");
        }

        //here we only land if the child process did not end successfully
        write(STDERR_FILENO, "\r\033[K", 4);
        sprintf(msg, "=Monitor(copil)= executarea treasure_hunter/manager a esuat: %s\n", strerror(errno));
        write(STDERR_FILENO, msg, strlen(msg));

        exit(EXIT_FAILURE);
    }

    //parintele
    close(pipefd[1]); 

   
    read_data_from_treasure_manager(pipefd[0]); //citeste data din treasure manager suntem in porcesul parinte

   
    waitpid(pid, NULL, 0); //is used to wait for a specific child process to change state (usually to terminate).

  
    kill(getppid(), SIGUSR1); //totul este ok trimitem semnalul SIGUSR1 Sends the signal SIGUSR1 to the process with process ID pid.
    //getppid(): Returns the parent process ID of the current process.

   
}

volatile sig_atomic_t running = 1;


void monitor_loop(void) 
{

    char buf[BUFSIZ];
    int count;
    while (running)
    {
        ssize_t n = read_line(STDIN_FILENO, buf, BUFSIZ); //citeste data de la pipe care la randul ei ia data de la stdin
        
        
        
        if (n > 0)
        {
            buf[n] = '\0'; // null-terminate the buffer
            count = split_into_words(buf, line);
            
            //  printf("Total words: %d\n", count);
            /*  for (int i = 0; i < count; ++i) {
             printf("Word %d: %s\n", i + 1, line[i]);
             }*/
        }
        
        if (n <= 0)
            break;
        //printf("\nokkkkkkkkkk\n");
        write(STDERR_FILENO, "\r\033[K", 4);
        
        execute_manager(line, count);
    }
}






int main(void) 
{

  char msg[BUFSIZ];

  setup_signal_handlers();

  snprintf(msg, BUFSIZ, "=Monitor(copil)= Porneste (PID: %d)\n", getpid()); //pornirea
  write(STDERR_FILENO, msg, strlen(msg));

  monitor_loop();

  sleep(2);

  snprintf(msg, BUFSIZ, "=Monitor(copil)= Se inchide.\n"); //inchiderea
  write(STDERR_FILENO, msg, strlen(msg));
  return EXIT_SUCCESS;
}
