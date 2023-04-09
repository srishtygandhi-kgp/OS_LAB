#include <iostream>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <unistd.h>
#include <bits/stdc++.h>

#include "macros.h"
using namespace std;
FILE *fp;

#define LOG_MSG(FP,MSG)                     \
    do {                                    \
        printf(MSG);                        \
        fprintf(FP, MSG);                   \
    } while (0)

int num_pushes[25], num_pops[NUM_READ_THREADS];

pthread_cond_t cv_empty;
pthread_cond_t feed_empty[NUM_READ_THREADS];

void calculate_priority(vector<vector<int>> &graph, vector<vector<int>> &priority_score){
    for(int i = 0; i <ROWS; i++){

        vector<int> v1 = graph[i];
        for(int id: graph[i]){
            vector<int> v2 = graph[id];
            
            vector<int> v3(v1.size() + v2.size());
            vector<int>::iterator end;

            end = set_intersection(
                v1.begin(), v1.end(),
                v2.begin(), v2.end(),
                v3.begin());
            //if(i <10 && j < 10)
            //cout<<i<<" "<<j<< " "<< distance(v3.begin(), end)<<endl;

            // jth element inserted.
            priority_score[i].push_back(distance(v3.begin(), end));
        }
        // if(i%100 == 0)
        //cout << i << endl;
    }
    cout << "[LOG] Graph priority populated" << endl;
}

void populate_graph(vector<vector<int>> &graph_input)
{
    // reads the file and populates the graph
    // assuming the edges are non directed

    FILE *file = fopen(FILE_PATH, "r");
    if (file == NULL)
    {
        printf("Unable to open file %s\n", FILE_PATH);
        return;
    }
    char line[20];
    fgets(line, 20, file);
    cout << line;

    while (fgets(line, 20, file))
    {
        // printf("READIND: %s\n", line);
        char *token = strtok(line, ",");
        int first = atoi(token);
        token = strtok(NULL, ",");
        int second = atoi(token);

        graph_input[first].push_back(second);
        graph_input[second].push_back(first);

        nodes[first].degree++;
        nodes[second].degree++;

        // printf("ITEREND\n");
    }

    cout << "[LOG] Graph populated" << endl;
}

void *simulateUserAction(void *arg)
{
    FILE *fp = (FILE *)arg;
    int nodeId, n, degree, num_action, action_type, cnt;
    while (1)
    {   
        // select 100 random nodes

        cout << "[LOG] simulateUserAction starting" << endl;

        for (int i = 0; i < ITER; i++)
        {
            // select a random node id
            nodeId = rand() % ROWS;
            degree = nodes[nodeId].degree;
            degree = 1 + floor(log2(degree));
            num_action = PCONSTANT * degree;

            if (pthread_mutex_lock(&lock_wallq))
            {
                perror("wall queue lock failed\n");
                exit(1);
            }
            else {
                fprintf(fp, "[userSimulator] thread locked the wall queue\n");
                printf("[userSimulator] thread locked the wall queue\n");
            }
            
            fprintf(fp, "\nNode id selected by userSimulator: %d\nNo of Actions generated: %d\nDegree of node id %d: %d\n", nodeId, num_action, nodeId, nodes[nodeId].degree);
            printf("\nNode id selected by userSimulator: %d\nNo of Actions generated: %d\nDegree of node id %d: %d\n", nodeId, num_action, nodeId, nodes[nodeId].degree);
            for (int j = 0; j < num_action; j++)
            {
                Action newAction;
                newAction.user_id = nodeId;
                action_type = rand() % 3;
                newAction.action_type = action_type;

                // action_id is a countervariable associated with each node and specific to each action
                cnt = ++nodes[nodeId].action_id[action_type];
                newAction.action_id = cnt;

                time_t curr_time = time(NULL);
                // cout << curr_time << "\n";
                newAction.time_stamp = curr_time;

                newAction.priority_type = -1;
                newAction.priority_val = -1;

                // push the action to the WallQueue of user node
                nodes[nodeId].WallQueue.push(newAction);

                // push the action to a queue monitored by pushUpdatethreads
                GlbWallQueue.push(newAction);

                // print the details of new action to sns.log
                fprintf(fp, "----Action----\nAction id: %d\nAction type: %d\nTimestamp: %d\n", newAction.action_id, newAction.action_type, newAction.time_stamp);
                printf("----Action----\nAction id: %d\nAction type: %d\nTimestamp: %d\n", newAction.action_id, newAction.action_type, newAction.time_stamp);

                pthread_cond_broadcast(&cv_empty);
                fprintf(fp, "[simulateUserAction] userSimulator thread sent condition signal.\n");
                printf("[simulateUserAction] userSimulator thread sent condition signal.\n");
            }

            if (pthread_mutex_unlock(&lock_wallq))
            {
                perror("wall queue unlock failed\n");
                exit(1);
            }
            else {
                fprintf(fp, "[simulateUserAction] userSimulator thread unlocked the wall queue\n");
                printf("[simulateUserAction] userSimulator thread unlocked the wall queue\n");
            }
        }
        // sleep for 2 minutes after pushing all actions
        // sleep(120);

        for (int i = 0; i < 12; i ++) {
            sleep(10);
            cout << "[SLEPT] " << ( i + 1 ) * 10 << endl;
        }
    }
    pthread_exit(NULL);
}

void *pushUpdate(void *arg)
{

    int id = *(int *)arg;
    bool got_action = false;
    Action newAction;

    // worker threads come here access the queue (critical) and dequeue "Action". 
    // If the queue is empty they wait on the condition using pthread_cond_wait 
    // variable else the thread deques an element from queue and signals after checking again for the condition
    // After that the thread pushes the actions to neighours feed

    while (1)
    {
        if (pthread_mutex_lock(&lock_wallq))
        {
            perror("wall queue lock failed\n");
            exit(1);
        }
        else {
            fprintf(fp, "[pushUpdate] Thread id %d locked the wall queue\n", id);
            printf("[pushUpdate] Thread id %d locked the wall queue\n", id);
        }

        if (!GlbWallQueue.empty())
        {
            newAction = GlbWallQueue.front();
            got_action = true;
            GlbWallQueue.pop();

            // if condition is reached then send signal
            if (!GlbWallQueue.empty())
            {
                pthread_cond_broadcast(&cv_empty);
                fprintf(fp, "[pushUpdate] Thread id %d sent condition signal.\n", id);
                printf("[pushUpdate] Thread id %d sent condition signal.\n", id);
            }
        }
        else
        {
            // wait for some actions to be added to queue
            while (GlbWallQueue.empty())
            {
                fprintf(fp, "[pushUpdate] Thread id %d going into wait for actions to be added in queue...\n", id);
                printf("[pushUpdate] Thread id %d going into wait for actions to be added in queue...\n", id);
                pthread_cond_wait(&cv_empty, &lock_wallq);
            }
        }

        if (pthread_mutex_unlock(&lock_wallq))
        {
            perror("wall queue unlock failed\n");
            exit(1);
        }
        else {
            fprintf(fp, "[pushUpdate] Thread id %d unlocked the wall queue\n", id);
            printf("[pushUpdate] Thread id %d unlocked the wall queue\n", id);
        }

        // do the work
        if (got_action)
        {
            // add element to feed queue of each neighbour
            for (int i = 0; i < graph[newAction.user_id].size(); i++)
            {
                int neighbourId = graph[newAction.user_id][i];
                int priority_val = priority_score[newAction.user_id][i];

                int _id = neighbourId % NUM_READ_THREADS;

                if ( pthread_mutex_lock(&lock_feedq[_id]) ) {
                    perror("feed queue lock failed");
                    exit(1);
                } 
                else {
                    fprintf(fp, "[pushUpdate] Thread id %d locked the feed queue\n", id);
                    printf("[pushUpdate] Thread id %d locked the feed queue\n", id);
                }

                // adding the detail dependent
                newAction.priority_type = nodes[neighbourId].priority;
                newAction.priority_val = priority_val;

                // TODO: add a mutex to protect the feed-queue
                nodes[neighbourId].FeedQueue.push(newAction);                
                feed_queue[_id].push(neighbourId);

                // adding a pthread_cond_signal/broadcast to wake up
                // stuck readPost threads
                fprintf(fp, "[pushUpdate] Thread id %d added action of Node id %d to feed of Node id %d\n", id, newAction.user_id, neighbourId);
                printf("[pushUpdate] Thread id %d added action of Node id %d to feed of Node id %d\n", id, newAction.user_id, neighbourId);

                pthread_cond_broadcast(&feed_empty[_id]);
                fprintf(fp, "[pushUpdate] This thread %d sent the broadcast\n", id);
                printf("[pushUpdate] This thread %d sent the broadcast\n", id);

                if ( pthread_mutex_unlock(&lock_feedq[_id]) )
                {
                    perror("wall queue unlock failed\n");
                    exit(1);
                }

                num_pushes[id - 1] ++;

                /*
                int _cnt = 0;
                for (int i = 0; i < 25; i ++)
                    _cnt += num_pushes[i];
                if ( (_cnt % 10000) == 0 )
                    cout << "PUSHES: " << id << " :" << _cnt << endl;
                */

            }
            got_action = false;
        }
    }
    pthread_exit(NULL);
}

void log_action(Action action, int thread_id, int node_id) {

    switch(action.action_type) {

        case 1: // POST
            fprintf(fp, "[readPost] Thread-%d, read a POST from Node-%d's feed-queue\n"
            , thread_id
            , node_id);
            printf("[readPost] Thread-%d, read a POST from Node-%d's feed-queue\n"
            , thread_id
            , node_id);
            break;
        case 2: // COMMENT
            fprintf(fp, "[readPost] Thread-%d, read a COMMENT from Node-%d's feed-queue\n"
            , thread_id
            , node_id);
            printf("[readPost] Thread-%d, read a COMMENT from Node-%d's feed-queue\n"
            , thread_id
            , node_id);
            break;
        default: // LIKE
            fprintf(fp, "[readPost] Thread-%d, read a LIKE from Node-%d's feed-queue\n"
            , thread_id
            , node_id);
            printf("[readPost] Thread-%d, read a LIKE from Node-%d's feed-queue\n"
            , thread_id
            , node_id);

    }

    return;
}

void *readPost(void *arg) {
    int id = *(int *)arg;

    /**
     * We're basically dividing the nodes up into 10 classes
     * and making the threads take up one class each.
     * 
     * For each of these classes, there is a mutex defined,
     * when we access a particular node's feed-queue, we will acquire
     * the lock, and update it, and then free the lock
    */
    
    // getting the lock to wait on
    //pthread_mutex_t feed_lock = &lock_feedq[id - 1];

    while (1) {
    
        // waiting on its feed-queue's lock
        if ( pthread_mutex_lock(&lock_feedq[id - 1]) ) {
            perror("feed queue lock failed");
            exit(1);
        } 
        else {
            fprintf(fp, "[readPost] Thread id %d locked the feed queue\n", id);
            printf("[readPost] Thread id %d locked the feed queue\n", id);
        }

        while (!feed_queue[id - 1].empty()) {

            int node = feed_queue[id - 1].front();
            feed_queue[id - 1].pop();

            Action _action = nodes[node].FeedQueue.top();
            nodes[node].FeedQueue.pop();

            log_action(_action, id, node);

            // num_pops[id - 1] ++;

            fprintf(fp, "[readPost] Thread id %d popped %d node's Feed\n", id, node);
            printf("[readPost] Thread id %d popped %d node's Feed\n", id, node);
        }

        while (feed_queue[id - 1].empty())
        {
            fprintf(fp, "[readPost] Thread id %d going into wait for actions to be added in queue...\n", id);
            printf("[readPost] Thread id %d going into wait for actions to be added in queue...\n", id);
            pthread_cond_wait(&feed_empty[id - 1], &lock_feedq[id - 1]);
        }

        while (!feed_queue[id - 1].empty()) {

            int node = feed_queue[id - 1].front();
            feed_queue[id - 1].pop();

            Action _action = nodes[node].FeedQueue.top();
            nodes[node].FeedQueue.pop();

            log_action(_action, id, node);

            // num_pops[id - 1] ++;

            fprintf(fp, "[readPost] Thread id %d popped %d node's Feed\n", id, node);
            printf("[readPost] Thread id %d popped %d node's Feed\n", id, node);
        }

        // unlocking its feed-queue's lock
        if ( pthread_mutex_unlock(&lock_feedq[id - 1]) ) {
            perror("feed queue lock failed");
            exit(1);
        } 
        else {
            fprintf(fp, "[readPost] Thread id %d unlocked the feed queue\n", id);
            printf("[readPost] Thread id %d unlocked the feed queue\n", id);
        }

        /*
        int _cnt = 0;
        for (int i = 0; i < NUM_READ_THREADS; i ++)
            _cnt += num_pops[i];
        
        if ( (_cnt % 10000) == 0 )
            cout << "POPS: " << id << " " << _cnt << endl;
        */
    }


    pthread_exit(NULL);
}

int main()
{

    srand(time(0)); // seed the random number generator with the current time

    for (int i = 0; i < ROWS; i++)
    {
        int randInt = rand() % 2;
        nodes[i].priority = randInt;
        nodes[i].degree = 0;
        // No of "post", "comment", "like" set to 0 for all nodes
        nodes[i].action_id[0] = 0;
        nodes[i].action_id[1] = 0;
        nodes[i].action_id[2] = 0;
    }

    // populate the graph
    populate_graph(graph);

    cout << "[PARAM] GRAPH POP DONE" << endl;

    calculate_priority(graph, priority_score);

    int max_degree = -1;
    for (int i = 0; i < ROWS; i++)
    {
        max_degree = max(max_degree, nodes[i].degree);
    }
    printf("Number of nodes -> %ld\n", graph.size());
    cout << "Maximum degree of node: " << max_degree << "\n";

    int return_value;
    pthread_t userSimulator;
    pthread_attr_t attr;

    // initialize pthread mutex protecting GlbWallQueue and condition variable objects
    pthread_mutex_init(&lock_wallq, NULL);
    pthread_cond_init(&cv_empty, NULL);
    for (int i = 0; i < 10; i ++)
        pthread_cond_init(&feed_empty[i], NULL);

    // explicitly creating threads in a joinable state 
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    fp = fopen(FILE_PATH_LOG, "aw");
    if ((return_value = pthread_create(&userSimulator, &attr, simulateUserAction, (void *)fp)))
    {
        perror("pthread_create\n");
        exit(0);
    }

    cout << "[LOG] User Simulator running" << endl;
    
    // create 10 readPost 25 pushUpdate thread
    pthread_t readPost_pool[10];
    pthread_t pushUpdate_pool[25];

    for (int i = 0; i < 25; i++)
    {
        int tid = i + 1;
        pthread_create(&pushUpdate_pool[i], &attr, pushUpdate, &tid);
    }
    
    cout << "[LOG] pushUpdate threads created" << endl;
    
    int thread_ids[NUM_READ_THREADS];
    for(int i=0; i < 10; i++) {
        
        thread_ids[i] = i + 1;
        pthread_create(&readPost_pool[i], &attr, readPost, &thread_ids[i]);
    }
    
    cout << "[LOG] readPost threads created" << endl;
    
    // block until the thread completes
    pthread_join(userSimulator, NULL);
    for (int i = 0; i < 25; ++i)
    {
        pthread_join(pushUpdate_pool[i], NULL);
    }

    for (int i = 0; i < NUM_READ_THREADS; ++i)
    {
        pthread_join(readPost_pool[i], NULL);
    }

    fclose(fp);
    // Clean up and exit 
    pthread_mutex_destroy(&lock_wallq);
    for (int i = 0; i < NUM_READ_THREADS; i ++)
        pthread_mutex_destroy(&lock_feedq[i]);
    pthread_cond_destroy(&cv_empty);
    for (int i = 0; i < NUM_READ_THREADS; i ++)
        pthread_cond_destroy(&feed_empty[i]);
    // pthread_exit(NULL);
    return 0;
}