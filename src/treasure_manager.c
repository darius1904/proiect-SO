// so.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


#include "treasure.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




Operation get_operation(const char* op) {
    if (strcmp(op, "--add") == 0) return OP_ADD;
    if (strcmp(op, "--list") == 0) return OP_LIST;
    if (strcmp(op, "--view") == 0) return OP_VIEW;
    if (strcmp(op, "--list_hunts") == 0) return OP_LIST_EVERYTHING;
    if (strcmp(op, "--remove_treasure") == 0) return OP_REMOVE_TREASURE;
    if (strcmp(op, "--remove_hunt") == 0) return OP_REMOVE_HUNT;
    return OP_UNKNOWN;
}


int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("\033[31mArgumente insuficiente\033[0m\n");
        interfata();
        return 1;
    }

    const char* operatie = argv[1];
    const char* hunt_id = argv[2];

    Operation op = get_operation(operatie);

    switch (op) {
    case OP_ADD:
        if (argc< 3) {
            printf("\033[31m\nEroare: Nu exita Hunt ID\033[0m\n");
            interfata();
            return 1;
        }
        
        add_treasure(hunt_id);
        break;

    case OP_LIST:
        if (argc < 3) {
            printf("\033[31m\nEroare: Nu sunt indeplinite conditiile\033[0m\n");
            interfata();
            return 1;
        }
        list_treasure(hunt_id);
        break;

    case OP_LIST_EVERYTHING:
        if (argc < 2) {
            printf("\033[31m\nEroare: Nu sunt indeplinite conditiile\033[0m\n");
            interfata();
            return 1;
        }
            everything_list();
        break;

    case OP_VIEW:
        if (argc < 4) {
            printf("\033[31m\nEroare: Nu sunt indeplinite conditiile\033[0m\n");
            interfata();
            return 1;
        }

        
        view(hunt_id, argv[3]);
        break;

    case OP_REMOVE_TREASURE:
        if (argc < 4) {
            printf("\033[31m\nEroare: Nu sunt indeplinite conditiile\033[0m\n");
            interfata();
            return 1;
        }
       
        remove_treasure(hunt_id, argv[3]);
        break;

    case OP_REMOVE_HUNT:
        if (hunt_id == NULL) {
            printf("\033[31m\nEroare: Nu sunt indeplinite conditiile\033[0m\n");
            interfata();
            return 1;
        }
        remove_hunt(hunt_id);
        break;

    case OP_UNKNOWN:
    default:
        perror("\033[31m\nAlte erori\033[0m");
        break;
    }

    return 0;


}


