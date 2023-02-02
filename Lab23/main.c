#include "main.h"

void printPrompt() {
    printf("\033[0;35m");
    printf("wish -> ");
    printf("\033[0m"); 
}

void exitMessage() {
    printf("\033[0;36m");
    printf("Wish is leaving. Have a nice day!\n");
    printf("\033[0m"); 
}

int main(int argc, char **argv) {

    //temporarily making void for compilation
    (void)argc;
    (void)argv;

    char* input;
    size_t inputSize;

    printf("Welcome to the Wish shell\n");

    while(1) {

        printPrompt();

        getline(&input, &inputSize, stdin);

        printf("You typed: %s", input);

        if(strcmp(input, "clear\n") == 0) system("clear");

        else if(strcmp(input, "exit\n") == 0) break;
    }

    exitMessage();

    return 0;
}