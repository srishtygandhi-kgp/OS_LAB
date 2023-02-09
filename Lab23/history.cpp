#include "history.h"

using namespace std;

deque<char *> history;

void loadHistory() {
    history.clear();

    FILE *fp = fopen("/tmp/temp_shell_hitory.txt", "r");
    if(!fp) return;

    size_t size = 300;
    char *buffer = (char *)malloc(size*sizeof(char));

    while(1) {
        size_t size = getline(&buffer, &size, fp);
        if(size == -1) break;
        buffer[size-1] = '\0';
        history.push_back(buffer);
    }
    free(buffer);
    fclose(fp);
}


void setHistory(char *s) {
    if (history.size() == HISTORY_SIZE) {
        history.pop_front();
    }
    history.push_back(s);
}


char* getHistory(int indexBack) {
    if(history.empty()) return NULL;
    else if(indexBack >= history.size()) return NULL;
    else return history.at(history.size()-1-indexBack);
}

void saveHistory() {
    FILE *fp = fopen("/tmp/temp_shell_hitory.txt", "w");
    if(!fp) return;

    for(auto it: history) {
        fprintf(fp, "%s\n", it);
    }

    fclose(fp);
}