#include <iostream>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <unistd.h>

#include "macros.h"
using namespace std;

void populate_graph(vector<vector<int>> &graph_input){
    // reads the file and populates the graph
    // assuming the edges are non directed

    FILE *file = fopen(FILE_PATH, "r");
    if (file == NULL) {
        printf("Unable to open file %s\n", FILE_PATH);
        return ;
    }
    char line[20];
    fgets(line, 20, file);
    cout<<line<<"\n"; 

    while (fgets(line, 20, file) ) {
        char *token = strtok(line, ",");
        int first = atoi(token);
        token = strtok(NULL, ",");
        int second = atoi(token);

        graph_input[first].push_back(second);
        graph_input[second].push_back(first);

        nodes[first].degree++;
        nodes[second].degree++;

    }

}

void *simulateUserAction(void *arg) {
    FILE *fp = (FILE *)arg;
    int nodeId, n, degree, num_action, action_type, cnt;
    while(1) {    
        // select 100 random nodes
        for(int i=0; i <100; i++) {
            // select a random node id
            nodeId = rand()%37700;
            degree = nodes[nodeId].degree;
            degree = floor(log2(degree));
            num_action = PCONSTANT*degree;

            fprintf(fp, "\nNode id selected by userSimulator: %d\nNo of Actions generated: %d\nDegree of node id %d: %d\n", nodeId, num_action, nodeId, nodes[nodeId].degree);
            for(int j=0; j < num_action; j++) {
                Action newAction;
                newAction.user_id = nodeId;
                action_type = rand()%3;
                newAction.action_type = action_type;

                // action_id is a countervariable associated with each node and specific to each action
                cnt = ++nodes[nodeId].action_id[action_type];
                newAction.action_id = cnt++;

                time_t curr_time = time(NULL);
                // cout << curr_time << "\n";
                newAction.time_stamp = curr_time;

                // push the action to the WallQueue of user node
                nodes[nodeId].WallQueue.push(newAction);

                // push the action to a queue monitored by pushUpdatethreads
                GlbWallQueue.push(newAction);

                // print the details of new action to sns.log
                fprintf(fp, "----Action----\nAction id: %d\nAction type: %d\nTimestamp: %d\n", newAction.action_id,newAction.action_type,newAction.time_stamp);
            }
        }
        // sleep for 2 minutes after pushing all actions
        sleep(120);
    }
}

void *pushUpdate(void *arg) {
    int id = *(int *)arg;

    // worker threads come here access the queue (critical)and
    // dequeue "Action". Later push the actions to neighours feed
    // need to use pthread_cond_signal to check if any change in the shared queue

}

void *readPost(void *arg) {
    int id = *(int *)arg;


}

int main(){
    
    srand(time(0));  // seed the random number generator with the current time
    vector<vector<int>> graph(ROWS);

    for(int i=0; i<ROWS; i++) {
        int randInt = rand()%2;
        nodes[i].priority = randInt;
        nodes[i].degree = 0;
        // No of "post", "comment", "like" set to 0 for all nodes
        nodes[i].action_id[0] = 0;
        nodes[i].action_id[1] = 0;
        nodes[i].action_id[2] = 0;
    }

    // populate the graph
    populate_graph(graph);

    int max_degree = -1;
    for(int i=0; i<ROWS; i++) {        
        max_degree = max(max_degree, nodes[i].degree);
    }
    printf("Number of nodes -> %ld\n", graph.size());
    cout<< "Maximum degree of node: " << max_degree<<"\n";

    actionCnt = 0;
    int return_value;
    pthread_t userSimulator;
    FILE *fp = fopen(FILE_PATH_LOG, "aw");
    if ((return_value = pthread_create(&userSimulator, NULL, simulateUserAction, fp))) {
      perror("pthread_create\n");
      exit(0);
    }
    // block until the thread completes
    pthread_join(userSimulator, NULL);

    // create 10 readPost 25 pushUpdate thread
    pthread_t readPost_pool[10];
    pthread_t pushUpdate_pool[25];

    // initialize pthread mutex protecting GlbWallQueue
    pthread_mutex_init(&lock_wallq, NULL);
    for(int i=0; i < 25; i++) {
        int *id = new int(i);
        pthread_create(&pushUpdate_pool[i], NULL, pushUpdate, id);
    }

    for(int i=0; i < 10; i++) {
        int *id = new int(i);
        pthread_create(&pushUpdate_pool[i], NULL, readPost, id);
    }

    fclose(fp);
    return 0;
    
}