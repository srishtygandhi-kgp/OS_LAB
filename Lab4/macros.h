#ifndef _MACROS_H
#define _MACROS_H

#include <queue>
#include <string>

using namespace std;
#define FILE_PATH "musae_git_edges.csv"
#define MAX_INT 37700
#define FILE_PATH_LOG "sns.log"
#define PROJECT_ID 1
#define PCONSTANT 1
#define ROWS 37700 // an arbitrarily choosen number larger than the initial number of nodes
#define INFINITE 99999

typedef struct actionStruct {
    int user_id;
    int action_id;
    int action_type;
    unsigned int time_stamp;
} Action;

typedef struct node {
    int priority; // 0->priority, 1->chronologial
    int degree;
    int action_id[3]; // action_id[0] for "post", action_id[1] for "comment", action_id[2] for "like"
    queue<Action> WallQueue;
    queue<Action> FeedQueue;
} Node;

vector<Node> nodes(37700);
// vector<Action> action;
// global queue to be monitored by pushUpdate thread
queue <Action> GlbWallQueue;
int actionCnt;
#endif