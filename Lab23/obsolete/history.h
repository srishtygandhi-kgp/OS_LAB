#ifndef HISTORY_H
#define HISTORY_H

#include <deque>
#include <string>
#include <vector>

#define HISTORY_SIZE 1000

using namespace std;

deque<char *> history;

void loadHistory();
void setHistory(char *s);
char* getHistory(int indexBack);
void saveHistory();

#endif