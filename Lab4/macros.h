#ifndef _MACROS_H
#define _MACROS_H

#include <queue>
#include <string>

using namespace std;
#define FILE_PATH "musae_git_edges.csv"
#define MAX_INT 37700
#define FILE_PATH_LOG "sns.log"
#define PROJECT_ID 1
#define PCONSTANT 10
#define ROWS 37700  // initial number of nodes
#define INFINITE 99999
#define NUM_READ_THREADS 10

typedef struct actionStruct {
    int priority_type;              // these are case dependent
    int priority_val;               // used for ordering per-case basis
    int user_id;
    int action_id;
    int action_type;
    unsigned int time_stamp;

    // creating a priority queue
    bool operator<(const actionStruct& rhs) const {
        return (priority_type) ? (time_stamp < rhs.time_stamp) : (priority_val < rhs.priority_val);
    }
} Action;

typedef struct node {
    int priority; // 0->priority, 1->chronologial
    int degree;
    int action_id[3]; // action_id[0] for "post", action_id[1] for "comment", action_id[2] for "like"
    queue<Action> WallQueue;
    priority_queue<Action> FeedQueue;
} Node;

vector<Node> nodes(37700);

using Graph = vector<vector<int> >;
Graph graph(ROWS);

vector<vector<int> > priority_score(ROWS);

// vector<Action> action;
// global queue to be monitored by pushUpdate thread
queue <Action> GlbWallQueue;
int actionCnt;

queue<int> feed_queue[NUM_READ_THREADS];

pthread_mutex_t lock_wallq;
pthread_mutex_t lock_feedq[NUM_READ_THREADS];

#endif