#ifndef _MACROS_H
#define _MACROS_H

#include <queue>

using namespace std;
#define FILE_PATH "musae_git_edges.csv"
#define MAX_INT 5000
#define KEY 1234
#define FILE_PATH_FOR_KEY "main.cpp"
#define PROJECT_ID 1
#define ROWS 37700 // an arbitrarily choosen number larger than the initial number of nodes
#define INFINITE 99999

typedef struct node {
    int id;
    int priority;
    int degree;
    queue<int> WallQueue;
    queue<int> FeedQueue;
} Node;

typedef struct actionStruct {
    int user_id;
    int action_id;
    int action_type;
    string time_stamp;
} Action;

vector<Node> nodes(37700);
vector<Action> action;
#endif