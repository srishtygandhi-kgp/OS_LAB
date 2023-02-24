#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "macros.h"
int cost[ROWS][COLS], Dist[ROWS], predecessor[ROWS], visited[ROWS];

void dijkstra(int(*array)[COLS], int *consumerSet, int n, int m, int sourcenode, FILE *fp)
{ 
    int k, cnt;
    // printf("arrays made\n");

    // predecessor[] stores the predecessor of each node
    // cnt stores the number of nodes seen so far
    for (int i = 0; i < n; i++){
        for (int j = 1; j < m; j++){
            // printf("%d, %d, array[%d][%d] = %d\n", i, j, i, j, array[i][j]);
            k = consumerSet[i];
            if (array[k][j] == 0){
                cost[k][j-1] = INFINITY;}
            else{
                cost[k][j-1] = 1;}
        }
    }
    // printf("cost matrix done\n");
    // initialize predecessor[],Dist[] and visited[]
    for (int i = 0; i < n; i++)
    {   
        k = consumerSet[i];
        visited[k] = 0;
        Dist[k] = cost[sourcenode][k];
        predecessor[k] = sourcenode;
    }
    Dist[sourcenode] = 0;
    visited[sourcenode] = 1;
    cnt = 1;
    int minDist, nodeNext;
    while (cnt < n - 1)
    {
        minDist = INFINITY;
        // nodeNext gives the node at minimum distance
        for (int i = 0; i < n; i++){
            k = consumerSet[i];
            if (Dist[k] < minDist && !visited[k])
            {
                minDist = Dist[k];
                nodeNext = k;
            }
        }
        // check if a better path exists through nodeNext
        visited[nodeNext] = 1;
        for (int i = 0; i < n; i++){
            k = consumerSet[i];
            if (!visited[k])
                if (minDist + cost[nodeNext][k] < Dist[k])
                {
                    Dist[k] = minDist + cost[nodeNext][k];
                    predecessor[k] = nodeNext;
                }
        }
        cnt++;
    }
    printf("Printing path to file for source node %d\n", sourcenode);
    // print the path and distance of each node
    for (int j = 0; j < n; j++){
        int i = consumerSet[j];
        if (i != sourcenode)
        {   
            fprintf(fp, "Distance of node %d = %d, ", i, Dist[i]);
            fprintf(fp,"Path=%d", i);
            k = i;
            do
            {
                k = predecessor[k];
                fprintf(fp,"<-%d", k);
            } while (k != sourcenode);
            fprintf(fp, "\n");
        }
    }
}

int main()
{
    key_t key;
    key = ftok(FILE_PATH_FOR_KEY, PROJECT_ID);
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }

    int shmid = shmget(key, 0, 0);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }

    int cid = 1; // consumer id
    int(*array)[COLS] = (int(*)[COLS])shmat(shmid, NULL, 0);
    if (array == (int(*)[COLS]) - 1)
    {
        perror("shmat");
        exit(1);
    }

    // Access the shared memory here
    int count = 0;
    for (int i = 0; i < ROWS; i++)
    {
        if (array[i][0] > 0)
            count++;
    }
    printf("total nodes: %d\n", count);

    // map the consumer to its set of nodes
    count /= 10;
    int consumerSet[count + 1];
    int cnt = 0, max_degree=0;
    printf("consumer-%d gets %d nodes\n", cid, count);
    for (int i = 0; i < ROWS && cnt < count; i++)
    {
        if (array[i][0] > 0)
        {   
            // printf("array[%d][%d] = %d", i, 1, array[i][1]);
            consumerSet[cnt++] = i;
            // printf("putting node %d in consumer set\n", i);
            if(array[i][0] > max_degree)
            max_degree = array[i][0];
        }
    }
    char filepath[25];
    sprintf(filepath, "consumer%d.txt", cid);
    // run Djkstra’s shortest path algorithm with all nodes in consumerSet as source node
    FILE *fp = fopen(filepath, "aw");
    for(int i = 0; i < count; i++) {
        // make every element of consumerSet as source node for dijkstra
        dijkstra(array, consumerSet, count, max_degree, consumerSet[i], fp);
    }
    fclose(fp);
    if (shmdt(array) == -1)
    {
        perror("shmdt");
        exit(1);
    }

    return 0;
}