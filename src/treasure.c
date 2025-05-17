#include <unistd.h>
#include "treasure.h"



int directory_exists(const char* path) {
    DIR* dp = opendir(path);
    if (dp == NULL) return 0;
    closedir(dp);
    return 1;
}

int create_directory(const char* path) {
    if (directory_exists(path)) return 0;
    return (mkdir(path, 0755) == -1) ? -1 : 1;
}


void interfata(void)
{

    printf("\nAvailable commands:\n");
    printf(" 1.Adaugă o noua comorară la un hunt              --add <hunt_id>\n 2.Listează toate comorile dintr-un hunt          --list <hunt_id>\n 3.Afișează o comoara specifică                   --view <hunt_id> <treasure_id>\n");
    printf(" 4.Șterge o comoara                               --remove_treasure <hunt_id> <treasure_id>\n 5.Șterge un întreg hunt                          --remove_hunt <hunt_id>\n");
    printf(" 6.Afișează toate hunturile                       --list_hunts\n\n");

}



Treasure_struct* create_treasure() {
    Treasure_struct* a_new_treasure = malloc(sizeof(Treasure_struct));

    if (a_new_treasure == NULL) {
        perror("Error allocating memory");   //atentie se pot citi maxim 24 de caractere. cred ca este suficient
        return NULL;
    }
    int ok;

    printf("Ce fel de citire? 0-manuală 1-rapidă\n");
    scanf("%d", &ok);

    if (ok)    //o citire rapida este daca dorim sa introducem toate datele deodata! nu sa stam manual
    {
        if (scanf("%24s %24s %f %f %1024s %d", a_new_treasure->id, a_new_treasure->userName, &a_new_treasure->GPSCoordinate.latitude, &a_new_treasure->GPSCoordinate.longitude, a_new_treasure->clueText, &a_new_treasure->value) != 6) {
            perror("\033[31mO eroare la citire\033[0m\n");
            free(a_new_treasure);
            return NULL;
        }
    }

    else
    {

        printf("Introdu dataele unei comori: \nID: ");

        if (scanf("%24s", a_new_treasure->id) != 1) {
            free(a_new_treasure);
            return NULL;
        }


        printf("Username: ");

        if (scanf("%24s", a_new_treasure->userName) != 1) {
            free(a_new_treasure);
            return NULL;
        }

        printf("Latitude: ");
        scanf("%f", &a_new_treasure->GPSCoordinate.latitude);

        printf("Longitude: ");
        scanf("%f", &a_new_treasure->GPSCoordinate.longitude);
        printf("Indiciul(clue): ");

        getchar(); 
        if (scanf("%1024s", a_new_treasure->clueText) != 1) {        //MAX_CLUE 1024
            free(a_new_treasure);
            return NULL;
        }                                                        //remove the \n from the end

        printf("Value: ");
        scanf("%d", &a_new_treasure->value);
    }

    return a_new_treasure;
}

int create_symlink_for_hunt(const char* hunt_id) {
    char target[MAX_PATH_LENGTH_DIR]; //256    am creat pathul 
    snprintf(target, sizeof(target), "%s/%s/%s", "treasure_hunts", hunt_id, "logged_hunt.txt");

    char symlink_path[MAX_PATH_LENGTH_DIR]; //256   legatura simbolica
    snprintf(symlink_path, sizeof(symlink_path), "%s-<%s>", "logged_hunt", hunt_id);

    struct stat sb;
    if (lstat(symlink_path, &sb) == 0) {
        if (S_ISLNK(sb.st_mode)) {
            printf("Symlink exists: %s\n", symlink_path);
            return 0;
        }
        else {
            fprintf(stderr, "File exists, is not a symlink: %s\n", symlink_path);
            return -1;
        }
    }

    if (symlink(target, symlink_path) == -1) {
        perror("Symlink failed");
        return -1;
    }

    printf("Symlink a fost creata: %s -> %s\n", symlink_path, target);
    return 1;
}

//adauga treasure add treasure

void add_treasure(const char* hunt_id) {


    char log_path[MAX_PATH_LENGTH_DIR];
    snprintf(log_path, sizeof(log_path), "%s/%s/%s", "treasure_hunts", hunt_id, "logged_hunt.txt"); //log

    static char dir_path[MAX_PATH_LENGTH_DIR];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", "treasure_hunts", hunt_id); //directorul

    static char file_path[MAX_PATH_LENGTH_DIR];
    snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, "treasure.bin"); //file .bin




    if (!directory_exists("treasure_hunts")) {                            //directorul mare
        if (create_directory("treasure_hunts") == -1) {
            printf("\033[31mEroare la crearea unui director\033[0m\n");
            return;
        }
    }


    if (!directory_exists(dir_path)) {                                   //directorul de hunt. treasure_hunts/...
        if (create_directory(dir_path) == -1) {
            printf("\033[31mEroare la crearea unui director\033[0m\n");
            return;
        }
    }


    int logf = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);  //crearea fisierului de log sau apenduirea in el daca acesta exita
    if (logf == -1) {
        close(logf);
        perror("Opening/appending hunt log");
        return;
    }

    int binf = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);  //crearea fisierului bin sau apenduirea in el daca acesta exita
    if (binf == -1) {
        close(binf);
        perror("Opening/appending hunt bin");
        return;
    }

    if (create_symlink_for_hunt(hunt_id) == -1) {               //se creeaza o legatura simbolica care va ramane pe parursul restul functiilor
        perror("\033[31mO eroare la creearea unei legaturi simbolice!\033[0m\n");
        close(binf);
        close(logf);
        return;
    }

    Treasure_struct* a_new_treasure = create_treasure();
    if (a_new_treasure == NULL) {
        return;
    }

    if (write(binf, a_new_treasure, sizeof(Treasure_struct)) != sizeof(Treasure_struct)) {
        perror("Eroare la scrierea unei comori");
        free(a_new_treasure);
        close(binf);
        return;
    }

    printf("\033[1;32m\nComoară adaugată cu succes\033[0m\n\n");
    close(binf);


    char log_entry[MAX_PATH_LENGTH_DIR];
    snprintf(log_entry, sizeof(log_entry), "Added treasure with ID: %s - user: %s\n", a_new_treasure->id, a_new_treasure->userName);

    ssize_t bytes_written_log = write(logf, log_entry, strlen(log_entry));

    if (bytes_written_log != strlen(log_entry)) {
        printf("Error writing to log file");
        close(logf);
        return;
    }

    free(a_new_treasure);
    close(logf);


}


void view(const char* hunt_id, const char* id) {
    char log_path[MAX_PATH_LENGTH_DIR];
    snprintf(log_path, sizeof(log_path), "%s/%s/%s", "treasure_hunts", hunt_id, "logged_hunt.txt"); //log

    static char dir_path[MAX_PATH_LENGTH_DIR];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", "treasure_hunts", hunt_id); //directorul

    static char file_path[MAX_PATH_LENGTH_DIR];
    snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, "treasure.bin"); //file .bin

    if (!directory_exists(dir_path)) {                                   //directorul de hunt. treasure_hunts/...
        printf("\033[31mNu exita acest hunt!\033[0m\n");
        return;
    }

    printf("\033[33m\nHunt name: %s\033[0m\n\n", hunt_id);

    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        printf("\033[31mError getting file info\033[0m\n");
        return;
    }


    int file;

    if ((file = open(file_path, O_RDONLY)) == -1) {     // in acest punct fisierul binar ar trebui sa existe, deoarece in el se afla datele despre treasures
        printf("Error opening file!\n");
        return;
    }

    int logf = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);  //crearea fisierului bin sau apenduirea in el daca acesta exita. ar trebui sa fie si el exitent deoarece sa scris in el la adaugarea unei comori
    if (logf == -1) {
        close(logf);
        perror("Opening hunt bin");
        return;
    }

    Treasure_struct treasure;

    ssize_t bytes_read;

    char username_forLOG[MAX_NAME];

    int found = 0;

    printf("Se afișează comoara cu ID-ul: %s\n", id);
    while ((bytes_read = read(file, &treasure, sizeof(treasure))) > 0) {
        if (bytes_read != sizeof(treasure)) {
            printf("Error reading file\n");
            close(file);
            return;
        }

        if (strcmp(treasure.id, id) == 0) {

            printf("+------+------------+----------------+----------------+-------------+--------+\n");
            printf("| ID   | Name       | Latitude       | Longitude      | Indiciul    | Value  |\n");
            printf("+------+------------+----------------+----------------+-------------+--------+\n");

            printf("| %-4s | %-10s | %-14.6f | %-14.6f | %-11s | %-6d |\n",
                treasure.id,
                treasure.userName,
                treasure.GPSCoordinate.latitude,
                treasure.GPSCoordinate.longitude,
                treasure.clueText,
                treasure.value);
            printf("+------+------------+----------------+----------------+-------------+--------+\n");
            printf("\n");

            snprintf(username_forLOG, sizeof(username_forLOG), "%s", treasure.userName);

            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\033[31mNu a exită o comoara cu ID-ul %s în cadrul acestui hunt\033[0m\n", id);
    }

    if (close(file) == -1) {
        perror("Error closing file");
        return;
    }


    char log_entry[MAX_PATH_LENGTH_DIR];

    snprintf(log_entry, sizeof(log_entry), "Viewed treasure with ID %s, user %s in hunt %s\n", id, username_forLOG, hunt_id); //ne trebuie usernameul ca sa il punem in log

    ssize_t bytes_written_log = write(logf, log_entry, strlen(log_entry));

    if (bytes_written_log == -1 || bytes_written_log != strlen(log_entry)) {
        printf("Error writing to log file");
        close(logf);
        return;
    }

    if (close(logf) == -1) {
        printf("Error closing log file");
        return;
    }

}



void list_treasure(const char* hunt_id) {

    char log_path[MAX_PATH_LENGTH_DIR];
    snprintf(log_path, sizeof(log_path), "%s/%s/%s", "treasure_hunts", hunt_id, "logged_hunt.txt"); //log

    static char dir_path[MAX_PATH_LENGTH_DIR];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", "treasure_hunts", hunt_id); //directorul

    static char file_path[MAX_PATH_LENGTH_DIR];
    snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, "treasure.bin"); //file .bin

    if (!directory_exists(dir_path)) {                                   //directorul de hunt. treasure_hunts/...
        printf("\033[31mNu exita acest hunt!\033[0m\n");
        return;
    }

    printf("\033[33m\nHunt name: %s\033[0m\n\n", hunt_id);

    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        printf("\033[31mError getting file info\033[0m\n");
        return;
    }

    printf("Dimensiunea fișierului: %lld bytes\n\n", file_stat.st_size);

    time_t mod_time = file_stat.st_mtime;
    struct tm* time_info = localtime(&mod_time);
    char time_str[100];

    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    printf("Ultima modificare: %s\n\n", time_str);


    int file;

    if ((file = open(file_path, O_RDONLY)) == -1) {     // in acest punct fisierul binar ar trebui sa existe, deoarece in el se afla datele despre treasures
        printf("Error opening file");
        return;
    }

    int logf = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);  //crearea fisierului bin sau apenduirea in el daca acesta exita. ar trebui sa fie si el exitent deoarece sa scris in el la adaugarea unei comori
    if (logf == -1) {
        close(logf);
        perror("Opening hunt bin");
        return;
    }

    Treasure_struct treasure;
    ssize_t bytes_read;
    int ok = 1;
    while ((bytes_read = read(file, &treasure, sizeof(treasure))) > 0) {

        if (bytes_read != sizeof(treasure)) {   //trebuie sa fie la fel
            printf("Error reading file");
            close(file);
            ok = 0;
            return;
        }
        else if (ok)
        {
            printf("+------+------------+----------------+----------------+-------------+--------+\n");
            printf("| ID   | Name       | Latitude       | Longitude      | Indiciul    | Value  |\n");
            printf("+------+------------+----------------+----------------+-------------+--------+\n");
            ok = 0;
        }



        printf("| %-4s | %-10s | %-14.6f | %-14.6f | %-11s | %-6d |\n",
            treasure.id,
            treasure.userName,
            treasure.GPSCoordinate.latitude,
            treasure.GPSCoordinate.longitude,
            treasure.clueText,
            treasure.value);
        printf("+------+------------+----------------+----------------+-------------+--------+\n");
    }
    printf("\n");
    if (close(file) == -1) {
        printf("Error closing file");
        return;
    }

    char log_entry[MAX_PATH_LENGTH_DIR];

    snprintf(log_entry, sizeof(log_entry), "All the treasures have been listed %s\n", hunt_id);

    ssize_t bytes_written_log = write(logf, log_entry, strlen(log_entry));

    if (bytes_written_log == -1 || bytes_written_log != strlen(log_entry)) {
        printf("Error writing to log file");
        close(logf);
        return;
    }

    if (close(logf) == -1) {
        printf("Error closing log file");
        return;
    }


}





void everything_list()
{


    DIR* dir = opendir("treasure_hunts");
    if (dir == NULL) {
        perror("opendir failed");
        return;
    }

    struct dirent* entry;
    struct stat st;
    char full_path[1024];
    //printf("okkkkkk\n");
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Build the full path
         snprintf(full_path, sizeof(full_path), "%s/%s", "treasure_hunts", entry->d_name);

        // Get file status
        if (stat(full_path, &st) == -1) {
            perror("Eroare la director la everything");
            continue;
        }

        // Check if it's a directory
        if (S_ISDIR(st.st_mode)) {
            
           // printf("Directory: %s 214\n", entry->d_name);
            list_treasure(entry->d_name);
            //printf("Directory: %s\n", entry->d_name);
        }
    }

    closedir(dir);

}


void remove_treasure(const char* hunt_id, const char* id) {
    static char dir_path[MAX_PATH_LENGTH_DIR];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", "treasure_hunts", hunt_id); //directorul

    static char file_path[MAX_PATH_LENGTH_DIR];
    snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, "treasure.bin"); //file .bin


    char temp_file_path[MAX_PATH_LENGTH_DIR];
    snprintf(temp_file_path, sizeof(temp_file_path), "%s/copy_%s", dir_path, "treasure.bin");  //auxiliar

    char log_path[MAX_PATH_LENGTH_DIR];
    snprintf(log_path, sizeof(log_path), "%s/%s", dir_path, "logged_hunt.txt"); //log


    if (!directory_exists(dir_path)) {                                   //directorul de hunt. treasure_hunts/...
        printf("\033[31mNu exita acest hunt!\033[0m\n");
        return;
    }

    int file;

    if ((file = open(file_path, O_RDONLY)) == -1) {     // in acest punct fisierul binar ar trebui sa existe, deoarece in el se afla datele despre treasures care ar urma sa fie sterse
        printf("Error opening file");
        return;
    }


    int temp_file = open(temp_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644); //un fisier temporar
    if (temp_file == -1) {
        perror("Error creating temporary file");
        close(file);
        return;
    }

    Treasure_struct a_new_treasure;
    ssize_t bytes_read;
    int found = 0;

    while ((bytes_read = read(file, &a_new_treasure, sizeof(a_new_treasure))) > 0) {     //cirim totul din fisierul binar si scriem in cel tempoar
        if (bytes_read != sizeof(a_new_treasure)) {
            close(file);
            close(temp_file);
            perror("Eroare la citirea fisierului");
            return;
        }

        if (strcmp(a_new_treasure.id, id) != 0) {                                                                   //scriem toate comorile inafara de cea cu ID-ul care vrea sa fie stersa in fisierul temporar != 
            if (write(temp_file, &a_new_treasure, sizeof(a_new_treasure)) != sizeof(a_new_treasure)) {
                close(file);
                close(temp_file);
                perror("Eroare scriere in fisierul temporar");
                return;
            }
        }
        else {
            found = 1;
        }
    }

    if (!found) {
        printf("Eroare: Nu sa gasit o comoara cu ID-ul '%s'\n", id);
        close(file);
        close(temp_file);
        unlink(temp_file_path);    //sterge fisierul de la temp_path_file adica fisierul binar ce contine comoara
        return;
    }

    close(file);
    close(temp_file);

    if (rename(temp_file_path, file_path) == -1) {    //schimba fisierul temporar ii da overwrite
        perror("Eroare la redenumirea fisierului temporar");
        return;
    }

    struct stat new_file_stat;                    //am obtinut informatiile despre fisierul binar
    if (stat(file_path, &new_file_stat) == -1) {
        perror("Eroare la obtinerea informatilor despre fisier");
        return;
    }

    int logf = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);  //crearea fisierului de log sau apenduirea in el daca acesta exita. ar trebui sa fie si el exitent deoarece sa scris in el la adaugarea unei comori
    if (logf == -1) {
        close(logf);
        perror("Opening hunt bin");
        return;
    }

    char log_entry[MAX_PATH_LENGTH_DIR];


    if (new_file_stat.st_size == 0) {    //avand informatii despre fisierul binar putem vedea daca acesta este gol sau nu
        if (unlink(file_path) == -1) {
            perror("Errore la stergerea fisierului gol");
            return;
        }


        snprintf(log_entry, sizeof(log_entry), "Comoara cu ID-ul %s a fost stearsa. Nu mai sunt alte comori in hunt-ul %s asa ca fisierul treasure.bin a fost sters\n", id, hunt_id);
    }
    else {
        snprintf(log_entry, sizeof(log_entry), "Comoara cu ID-ul %s a fost stearsa din hunt-ul %s\n", id, hunt_id);  //scriem in log
    }

    ssize_t bytes_written_log = write(logf, log_entry, strlen(log_entry));

    if (bytes_written_log == -1 || bytes_written_log != strlen(log_entry)) {
        printf("Error writing to log file");
        close(logf);
        return;
    }

    if (close(logf) == -1) {
        printf("Error closing log file");
        return;
    }

    printf("\033[1;32m\nComoara a fost stearsa din hunt cu succes!\033[0m\n\n");
}


void remove_hunt(const char* hunt_id) {      //stergere de hunt
    
    
    static char dir_path[MAX_PATH_LENGTH_DIR];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", "treasure_hunts", hunt_id); //calea directorului

    
    if (!directory_exists(dir_path)) {                                   //directorul de hunt. treasure_hunts/...
        printf("\033[31mNu exita acest hunt!\033[0m\n");
        return;
    }


    DIR* dir = opendir(dir_path);

    struct dirent* entry;
    char file_path[MAX_PATH_LENGTH_DIR];

    while ((entry = readdir(dir)) != NULL) {  //citeste fiecare entry din director pe rand
    
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {    //trebuie ocolit directorul curent si directorul parinte
            continue;
        }

        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name); //luam calea fisierelor din interiorul huntului care ar trebui sterse
        if (unlink(file_path) == -1) {
            printf("Eroare la stergerea fisierelor din interiorul huntului: %s\n", file_path);
            closedir(dir);
            return;
        }
    }

    char symlink_path[MAX_PATH_LENGTH_DIR];
    snprintf(symlink_path, sizeof(symlink_path), "%s-<%s>", "logged_hunt", hunt_id);  //fisierul meu legatura simbolica este de timpul logged_hunt-hunt1

    struct stat sb;
    if (lstat(symlink_path, &sb) == 0)
    {
        if (S_ISLNK(sb.st_mode)) //un macro care verifica daca un fisier e legatura simbolica
        {  
            unlink(symlink_path);
        }
        else 
        {
            printf("%s nu este o legatura simbolica!\n", symlink_path);
        }
    }
    else {
        printf("Symlink %s nu exista\n", symlink_path);
    }

    closedir(dir);
    if (rmdir(dir_path) == -1)
    {                             //stergera folderului/directorului hunt1
            printf("\033[31mEroare la stergerea directorului\033[0m\n");
            return;
    }
    printf("\033[1;32m\nHunt sters cu succes!\033[0m\n\n");
}

