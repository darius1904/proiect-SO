//
//  treasure.h
//  so
//
//  Created by darius on 13.04.2025.
//

#ifndef treasure_h
#define treasure_h


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

#define MAX_USERNAME 32
#define MAX_CLUE 128
#define TREASURE_FILE "treasures.dat"
#define LOG_FILE "logged_hunt"
#define TREASURE_SIZE sizeof(struct Treasure)

struct Treasure {
    int id;
    char username[MAX_USERNAME];
    float latitude;
    float longitude;
    char clue[MAX_CLUE];
    int value;
};

void log_operation(const char *hunt_path, const char *operation) {
    char log_path[512];
    snprintf(log_path, sizeof(log_path), "%s/%s", hunt_path, LOG_FILE);

    int fd = open(log_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("Log open failed");
        return;
    }

    time_t now = time(NULL);
    char log_entry[256];
    snprintf(log_entry, sizeof(log_entry), "[%s] %s\n", ctime(&now), operation);
    write(fd, log_entry, strlen(log_entry));
    close(fd);
}

void create_symlink(const char *hunt_id) {
    char link_name[256];
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id);

    char target_path[256];
    snprintf(target_path, sizeof(target_path), "%s/%s", hunt_id, LOG_FILE);

    symlink(target_path, link_name);  // Errors ignored if it already exists
}

void add_treasure(const char *hunt_id) {
    mkdir(hunt_id, 0755);

    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    struct Treasure t;
    printf("Treasure ID: ");
    scanf("%d", &t.id);
    printf("Username: ");
    scanf("%s", t.username);
    printf("Latitude: ");
    scanf("%f", &t.latitude);
    printf("Longitude: ");
    scanf("%f", &t.longitude);
    printf("Clue: ");
    getchar(); // consume newline
    fgets(t.clue, MAX_CLUE, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;
    printf("Value: ");
    scanf("%d", &t.value);

    int fd = open(file_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }
    write(fd, &t, TREASURE_SIZE);
    close(fd);

    log_operation(hunt_id, "Added a treasure.");
    create_symlink(hunt_id);
}

void list_treasures(const char *hunt_id) {
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    struct stat st;
    if (stat(file_path, &st) != 0) {
        perror("Stat failed");
        return;
    }

    printf("Hunt: %s\n", hunt_id);
    printf("File size: %ld bytes\n", st.st_size);
    printf("Last modified: %s", ctime(&st.st_mtime));

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    struct Treasure t;
    while (read(fd, &t, TREASURE_SIZE) == TREASURE_SIZE) {
        printf("ID: %d | User: %s | Location: (%.2f, %.2f) | Value: %d\n",
               t.id, t.username, t.latitude, t.longitude, t.value);
    }
    close(fd);
    log_operation(hunt_id, "Listed treasures.");
}

void view_treasure(const char *hunt_id, int id) {
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    struct Treasure t;
    while (read(fd, &t, TREASURE_SIZE) == TREASURE_SIZE) {
        if (t.id == id) {
            printf("ID: %d\nUser: %s\nLocation: %.2f, %.2f\nClue: %s\nValue: %d\n",
                   t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
            close(fd);
            log_operation(hunt_id, "Viewed a treasure.");
            return;
        }
    }
    printf("Treasure not found.\n");
    close(fd);
}

void remove_treasure(const char *hunt_id, int id) {
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    int temp_fd = open("temp.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    struct Treasure t;
    while (read(fd, &t, TREASURE_SIZE) == TREASURE_SIZE) {
        if (t.id != id) {
            write(temp_fd, &t, TREASURE_SIZE);
        }
    }
    close(fd);
    close(temp_fd);

    rename("temp.dat", file_path);
    log_operation(hunt_id, "Removed a treasure.");
}

void remove_hunt(const char *hunt_id) {
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    unlink(file_path);
    unlink(hunt_id); // in case it's a symlink

    char log_path[512];
    snprintf(log_path, sizeof(log_path), "%s/%s", hunt_id, LOG_FILE);
    unlink(log_path);

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);

    rmdir(hunt_id);
    printf("Hunt %s removed.\n", hunt_id);
}

#endif /* treasure_h */
