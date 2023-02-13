#include <string.h>
#include "myhistory.h"

using namespace std;

int historyIndex;
deque<char *> history;
char* commandBackup;

void loadHistory() {
    history.clear();
    historyIndex = -1;
    commandBackup = (char *)malloc(300*sizeof(char));

    FILE *fp = fopen("/tmp/temp_shell_history.txt", "r");
    if(!fp) return;

    size_t max_size = 300;
    char *buffer = (char *)malloc(max_size*sizeof(char));

    while(1) {
        size_t size = getline(&buffer, &max_size, fp);
        if((int)size == -1) break;
        buffer[size-1] = '\0';
        char *copy = (char *)malloc(max_size*sizeof(char));
        strcpy(copy, buffer);
        history.push_back(copy);
    }
    free(buffer);
    fclose(fp);
}


void setHistory(char *s) {
    char *copy = (char *)malloc(300*sizeof(char));
    strcpy(copy, s);
    if (history.size() == HISTORY_SIZE) {
        history.pop_front();
    }
    history.push_back(copy);
}


char* getHistory(int indexBack) {
    if(history.empty()) return NULL;
    else if(indexBack >= (int)history.size()) return NULL;
    else return history.at(history.size()-1-indexBack);
}

void saveHistory() {
    FILE *fp = fopen("/tmp/temp_shell_history.txt", "w");
    if(!fp) return;

    for(auto it: history) {
        fprintf(fp, "%s\n", it);
    }

    fclose(fp);
}