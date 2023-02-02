#include "main.h"

void printPrompt() {
    printf("wish -> ");
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

        if(strcmp(input, "exit\n") == 0) break;
    }

    printf("Wish is leaving. Have a nice day!\n");

    return 0;
}