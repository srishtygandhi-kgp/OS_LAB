#ifndef HISTORY_H
#define HISTORY_H

#include <deque>
#include <string>

#define HISTORY_SIZE 1000

using namespace std;

extern int historyIndex;
extern char* commandBackup;
extern deque<char *> history;

void loadHistory();
void setHistory(char *s);
char* getHistory(int indexBack);
void saveHistory();

#endif