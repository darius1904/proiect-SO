#include "shell.h"

char log_msg[BUFSIZ];






ssize_t read_line(int fd, char* buf, size_t max_len) 
{
    if (!buf || max_len == 0)
        return -1;

    size_t i = 0;
    char ch;
    ssize_t bytes_read;

    while (i < max_len - 1) {
        bytes_read = read(fd, &ch, 1);

        if (bytes_read == 0) {
            // EOF
            break;
        } else if (bytes_read < 0) {
            // If interrupted, try again
            if (errno == EINTR)
                continue;
            return -1; // Other read error
        }

        buf[i++] = ch;

        if (ch == '\n')
            break;
    }

    buf[i] = '\0';
    return i;
}















int is_monitor_alive(void)
{
    return (monitor.pid > 0) && (kill(monitor.pid, 0) == 0);
}

// While monitor is shutting down, intercept commands and report status
int check_monitor_stopping(void) {
    if (monitor.state != MONITOR_SHUTTING_DOWN)
        return 0;

    snprintf(log_msg, BUFSIZ, "Monitorul se opreste, asteptati.\n");
    write(STDOUT_FILENO, log_msg, strlen(log_msg));
    return 1;
}

void read_data_from_pipe(int fd) {
    char buffer[BUFSIZ];
    ssize_t n;
    while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        write(STDOUT_FILENO, buffer, n);
    }
}

void refresh_prompt(void)
{
    const char* status = NULL;

    switch (monitor.state) {
    case MONITOR_ONLINE:
        status = "online";
        break;
    case MONITOR_OFFLINE:
        status = "offline";
        break;
    case MONITOR_SHUTTING_DOWN:
        status = "shutting down";
        break;
    }

    snprintf(log_msg, BUFSIZ, "\nInterfața programului \033[38;5;220mtreasure_hub\033[0m <status monitor: %s> : ", status);
    write(STDOUT_FILENO, log_msg, strlen(log_msg));   //scrie la terminal

}


























int send_to_monitor(const char* cmd_line) 
{

    if (!is_monitor_alive())    //eroare daca monitorul nu este inca pornit
    
    {
        snprintf(log_msg, BUFSIZ,"\033[31mMonitorul nu este încă pornit. Folosește 'start_monitor'.\033[0m\n");
        write(STDOUT_FILENO, log_msg, strlen(log_msg));
        return 0;
    }

   snprintf(log_msg, BUFSIZ, "%s\n", cmd_line);   //new line at the end of cmd_line="--list hunt1"  un exemplu

    write(monitor.write_pipe_fd, log_msg, strlen(log_msg));   //trimite datele monitorului prin pipe

    return 1;
}



int check_argc_valid(shell_command cmd, int argc) 
{
    switch (cmd) {
    case CMD_LIST_HUNTS:
        return argc == 1;
    case CMD_LIST_TREASURES:
        return argc == 2;
    case CMD_VIEW_TREASURE:
        return argc == 3;
    default:
        return argc == 1;
    }
}




int cmd_list(shell_command cmd, char argv[][BUFSIZ], int argc) 

{
    if (!check_argc_valid(cmd, argc))  //verificam daca numarul de argumente este adecvat
        return 0;

    char buf[BUFSIZ];

    if (strcmp(argv[0], "list_hunts") == 0)  //comanda care este bagate in progam cand monitorul este pornit. daca este oprit va fi eroare cand se incearca trimiterea la monitor
    {
        strcpy(argv[0],"--list_hunts");
    }
       
    else if (strcmp(argv[0], "list_treasures") == 0)

    {
        strcpy(argv[0],"--list");
    }
       
    else if (strcmp(argv[0], "view_treasure") == 0)
    {
        strcpy(argv[0], "--view");
    }
    else if (strcmp(argv[0], "calculate_scores") == 0)    
    {
        strcpy(argv[0], "--calculate");
    }

    snprintf(buf, sizeof(buf), "%s", argv[0]);  //se pune si terminatorul \0 automat

    for (int i = 1; i < argc; i++) {
        strcat(buf, " ");
        strcat(buf, argv[i]);
    }

    return send_to_monitor(buf);
}








void execute_calculator(const char* path) 
{
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid < 0) {
        write(STDOUT_FILENO, "\r\033[K", 4);

        sprintf(log_msg, "=Calculator= fork a esuat: %s\n", strerror(errno));
        write(STDERR_FILENO, log_msg, strlen(log_msg));

        return;
    }

    if (pid == 0) {
        // Child
        close(pipefd[0]);               // Close read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipefd[1]);

        execl("./calculator", "calculator", path, NULL);
        perror("execl a esuat la crearea unui proces copil calculator");
        exit(EXIT_FAILURE);
    }
    else {
        // parintele
        close(pipefd[1]);

        // data de la calculatr ce trb citita
        char buffer[BUFSIZ];
        ssize_t n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            write(STDOUT_FILENO, buffer, n); // Output to terminal
        }

        close(pipefd[0]);
        waitpid(pid, NULL, 0); // Wait for child to finish
    }
}

int cmd_calculate_scores() {

    DIR* dir = opendir("treasure_hunts");

    if (!dir) {
        perror("Calculator");
        return 0;
    }

    
    struct dirent* entry;
    //struct stat st;
    char file_path[1000];    //un fork penrtru fiecare director de hunt cu argument calea. 

    while ((entry = readdir(dir)) != NULL) {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ||
            strcmp(entry->d_name, ".DS_Store") == 0)
            continue;

        snprintf(file_path, sizeof(file_path), "%s/%s/%s", "treasure_hunts",
            entry->d_name, "treasure.bin");
        
       /* if (stat(file_path, &st) == -1) {
            perror("Eroare la preluarea dateleor director");
            continue;
        }*/
        
      //  if (S_ISDIR(st.st_mode)) {
            execute_calculator(file_path);
      //  }
    }

    return 1;
}

























































































void cmd_clear_screen()
{

    const char* clear_cmd = "\033[2J";  // Clear entire screen
    const char* home_cmd = "\033[H";    // Move cursor to top-left
    write(STDOUT_FILENO, clear_cmd, strlen(clear_cmd));
    write(STDOUT_FILENO, home_cmd, strlen(home_cmd));

}

int cmd_start_monitor(void) {
    if (is_monitor_alive()) {
        snprintf(log_msg, BUFSIZ, "Monitorul ruleaza deja(PID: %d)\n", monitor.pid);
        write(STDOUT_FILENO, log_msg, strlen(log_msg)); //la output ca monitorul ruleaza
        return 0;
    }

    // main process    write >--pipe--> read    child process

    int shell_to_monitor_pipefd[2];
    if (pipe(shell_to_monitor_pipefd) < 0) {
        perror("pipe");
        return 0;
    }

    int monitor_to_shell_pipefd[2];
    if (pipe(monitor_to_shell_pipefd) < 0) {
        perror("pipe");
        return 0;
    }

    monitor.pid = fork(); 
    if (monitor.pid < 0) 
    {
        perror("Fork a esuat sa creeze procesul copil(Monitor).");
        close(shell_to_monitor_pipefd[0]);
        close(shell_to_monitor_pipefd[1]);
        close(monitor_to_shell_pipefd[0]);
        close(monitor_to_shell_pipefd[1]);
        exit(EXIT_FAILURE);
    }

    if (monitor.pid == 0)  //executat doar de copil se copiaza monitorul si doar procesul copil intra aici
    {               
       
        close(shell_to_monitor_pipefd[1]);     //capetele nefolositse se inchid
        dup2(shell_to_monitor_pipefd[0], STDIN_FILENO);   //folosit de monitor sa citeasca ceea ce e la shell
        close(shell_to_monitor_pipefd[0]);

        close(monitor_to_shell_pipefd[0]);
        dup2(monitor_to_shell_pipefd[1], STDOUT_FILENO);   //folost de monitor ca sa poata sa scrie in sell
        close(monitor_to_shell_pipefd[1]);

        execl("./treasure_monitor", "treasure_monitor", NULL);  //caile pentru programul monior
        perror("execl");
        exit(EXIT_FAILURE);
    }

    //in parinte este invers 
    close(shell_to_monitor_pipefd[0]);
    monitor.write_pipe_fd = shell_to_monitor_pipefd[1];  //folost de shell ca sa poata sa scrie ce e pe monitor
     
  
    close(monitor_to_shell_pipefd[1]);
    monitor.read_pipe_fd = monitor_to_shell_pipefd[0]; //folosit de shell ca sa poata sa citeasca de pe monitor

    monitor.state = MONITOR_ONLINE;

    return 1;
}


int cmd_stop_monitor(void) 
{
    if (!is_monitor_alive()) 
    {
        snprintf(log_msg, BUFSIZ, "Monitorul era oprit înainte de comandă.\n");
        write(STDOUT_FILENO, log_msg, strlen(log_msg));
        return 1;
    }

    monitor.state = MONITOR_SHUTTING_DOWN;
    kill(monitor.pid, SIGTERM);  //sends the SIGTERM signal to the process with PID monitor.pid

    close(monitor.read_pipe_fd);

    // Inform the user we’ve sent SIGTERM; actual exit message comes asynchronously in handle_sigchld().
    snprintf(log_msg, BUFSIZ, "Semnalul de închidere SIGTERM a fost trimis.\n");
    write(STDOUT_FILENO, log_msg, strlen(log_msg));
    return 1; //ca sa nu intre in !err
}


void cmd_exit_shell()
{
    if (is_monitor_alive())
    {
        snprintf(log_msg, BUFSIZ, "Monitorul înca rulează. Folosește 'stop_monitor' mai întâi.\n");
        write(STDOUT_FILENO, log_msg, strlen(log_msg));
        return;
    }
    snprintf(log_msg, BUFSIZ, "Programul treasure_hub se închide\n");
    write(STDOUT_FILENO, log_msg, strlen(log_msg));

    exit(EXIT_SUCCESS);
}


void cmd_print_help()
{

    char* help_msg =
        "\033[1;33mAvailable commands:\033[0m\n"
        "  \033[1;36mstart_monitor\033[0m                - porneste un proces nou\n"
        "  \033[1;36mstop_monitor\033[0m                 - opreste procesul monitor\n"
        "  \033[1;36mlist_hunts\033[0m                   - listeaza toate hunturile si toate comorile\n"
        "  \033[1;36mlist_treasures <hunt_id>\033[0m     - listeaza toate comorile dintr-un hunt\n"
        "  \033[1;36mview_treasure <hunt_id> <id>\033[0m - afiseaza o comoara specifica dintr-un hunt\n"
        "  \033[1;36mcalculate_scores\033[0m             - calculeaza scorurile pentru fiecare hunt\n"
        "  \033[1;36mexit\033[0m                         - opreste programul (sau eroare daca monitorul ruleaza)\n";

    snprintf(log_msg, BUFSIZ, "\n%s", help_msg);

    write(STDOUT_FILENO, log_msg, strlen(log_msg));
}

































int execute_command(shell_command cmd, char argv[][BUFSIZ], int argc) {
    // verifica daca monitorul se inchide
    if (check_monitor_stopping())
        return 1;

    switch (cmd) {
    case CMD_CLEAR:
        cmd_clear_screen();
        break;
    case CMD_START_MONITOR:
        return cmd_start_monitor();
    case CMD_STOP_MONITOR:
        return cmd_stop_monitor();
    case CMD_EXIT:
        cmd_exit_shell();
        break;
    case CMD_LIST_HUNTS:
        return cmd_list(CMD_LIST_HUNTS, argv, argc);
    case CMD_LIST_TREASURES:
        return cmd_list(CMD_LIST_TREASURES, argv, argc);
    case CMD_VIEW_TREASURE:
        return cmd_list(CMD_VIEW_TREASURE, argv, argc);
    case CMD_CALCULATE_SCORES:
         return cmd_calculate_scores();
    default:
        {
            cmd_print_help();
            printf("\n");
            return 0;
        }
    }
    return 1;
}






shell_command which_command(char* string, char args[MAX_ARGS][BUFSIZ], int* argc)
{

    *argc = 0;
    memset(args, 0, MAX_ARGS * BUFSIZ); //argumentele


    char buf[BUFSIZ];
    size_t n = strnlen(string, BUFSIZ - 1);   //o copie a argumentelor a inputului
    memcpy(buf, string, n);
    buf[n] = '\0';


    char* tok = strtok(buf, " \n");  //pregatire de separare  primul token
    if (!tok) {
        return CMD_INVALID;  
    }


    do {
        strncpy(args[(*argc)++], tok, BUFSIZ - 1);   //copiaza fiecare tok in args argumentele
    } while (*argc < MAX_ARGS && (tok = strtok(NULL, " \n")) != NULL);

    //ce ne trebuie returnam un enum
    if (strcmp(args[0], "help") == 0)
        return CMD_HELP;
    else if (strcmp(args[0], "clear") == 0)
        return CMD_CLEAR;
    else if (strcmp(args[0], "start_monitor") == 0)
        return CMD_START_MONITOR;
    else if (strcmp(args[0], "stop_monitor") == 0)
        return CMD_STOP_MONITOR;
    else if (strcmp(args[0], "list_hunts") == 0)
        return CMD_LIST_HUNTS;
    else if (strcmp(args[0], "list_treasures") == 0)
        return CMD_LIST_TREASURES;
    else if (strcmp(args[0], "view_treasure") == 0)
        return CMD_VIEW_TREASURE;
    else if (strcmp(args[0], "calculate_scores") == 0)
        return CMD_CALCULATE_SCORES;
    else if (strcmp(args[0], "exit") == 0)
        return CMD_EXIT;
    else
        return CMD_INVALID;
}




int run_repl(void) {

  char input[BUFSIZ];
  char argv[MAX_ARGS][BUFSIZ]; //argv  These are filled when the user types something at runtime (not when launching the program).
  int argc = 0;

  while (1) 
  {
    refresh_prompt();
    ssize_t bytes_read = read_line(STDIN_FILENO, input, BUFSIZ); //citesc o linie de la terminal
    if (bytes_read < 0) 
    {
      if (errno == EINTR)  //the system call was interupted by a signal before it could complete
        continue; //restart the loop
      else
      {
          perror("\033[31mEroare la citire\033[0m\n");
          return -1;
      }
    }

    shell_command cmd = which_command(input, argv, &argc);

    int err = execute_command(cmd, argv, argc);

    if (!err) {
      snprintf(log_msg, BUFSIZ, "\033[31mCommanda introdusa anterior a fost invalidă\033[0m\n");
      write(STDOUT_FILENO, log_msg, strlen(log_msg));
      continue;
    }

    if (cmd == CMD_LIST_HUNTS || cmd == CMD_LIST_TREASURES || cmd == CMD_VIEW_TREASURE)
      read_data_from_pipe(monitor.read_pipe_fd); //citeste data din pipe si apoi o afisam

    if (cmd == CMD_START_MONITOR)
      usleep(1000000);
  }

  return EXIT_SUCCESS;
}

































void sig_refresh_handler(int sig)
{
    write(STDOUT_FILENO, "\r\033[K", 4);  // sa activam stdin dam refresh
}

void sigchld_handler(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)   //returns the pid of the child. if there is no exited child, wait pid returneaza 0 instead of waiting
    {
        if (pid == monitor.pid && monitor.state == MONITOR_SHUTTING_DOWN) //daca este un copil care a dat exit si monitorol este in comanda de oprire!
        {
            if (WIFEXITED(status))    //copilul sa terminat in mod normal, un macro
            {
                snprintf(log_msg, BUFSIZ, "Monitorul sa închis (status=%d)\n", WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))  //retuns true daca copilul sa terminat printr-un semnal, (in mod anormal)
            {
                snprintf(log_msg, BUFSIZ, "\033[31mMonitorul a murit printr-un semnal. (status=%d)\033[0m\n",
                    WTERMSIG(status));
            }
            write(STDOUT_FILENO, log_msg, strlen(log_msg));

            monitor.state = MONITOR_OFFLINE;  //monitor inchis
            monitor.pid = -1;
            tcflush(STDIN_FILENO, TCIFLUSH); //flush la data from input buffer. flush all data that is received but not read yet (i.e., discard any pending input data waiting in the input queue).
            refresh_prompt();
        }
    }
}

void setup_signal_handlers(void)
{
    struct sigaction sa_child = { .sa_handler = sigchld_handler,
                                 .sa_flags = SA_RESTART | SA_NOCLDSTOP };
    sigaction(SIGCHLD, &sa_child, NULL);

    struct sigaction sa_refresh = { .sa_handler = sig_refresh_handler,
                                   .sa_flags = 0 };
    sigaction(SIGUSR1, &sa_refresh, NULL);
}

















int main(void) 
{
  setup_signal_handlers();
  run_repl();

  return EXIT_SUCCESS;
}
