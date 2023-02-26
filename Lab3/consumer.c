#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "macros.h"
// int cost[ROWS][COLS], Dist[ROWS], predecessor[ROWS], visited[ROWS];
int shortestDists[ROWS], isAdded[ROWS], parent[ROWS];

// void dijkstra(int(*array)[COLS], int *consumerSet, int n, int m, int sourcenode, FILE *fp)
// { 
//     int cnt;
//     // printf("arrays made\n");

//     // predecessor[] stores the predecessor of each node
//     // cnt stores the number of nodes seen so far
//     // for (int i = 0; i < n; i++){
//     //     if(array[i][0] <= 0) continue;
//     //     for (int j = 1; j < m; j++){
//     //         // printf("%d, %d, array[%d][%d] = %d\n", i, j, i, j, array[i][j]);
//     //         if (array[i][j] == 0){
//     //             cost[i][j-1] = INFINITY;}
//     //         else{
//     //             cost[i][j-1] = 1;}
//     //     }
//     // }
//     // printf("cost matrix done\n");
//     // initialize predecessor[],Dist[] and visited[]
//     for (int i = 0; i < n; i++)
//     {   
//         // if(array[i][0] <= 0) continue;
//         visited[i] = 0;
//         Dist[i] = array[sourcenode][i];
//         predecessor[i] = sourcenode;
//     }
//     Dist[sourcenode] = 0;
//     visited[sourcenode] = 1;
//     cnt = 1;
//     int minDist, nodeNext;
//     while (cnt < n - 1)
//     {
//         minDist = INFINITY;
//         // nodeNext gives the node at minimum distance
//         for (int i = 0; i < n; i++){
//             if (Dist[i] < minDist && !visited[i])
//             {
//                 minDist = Dist[i];
//                 nodeNext = i;
//             }
//         }
//         // check if a better path exists through nodeNext
//         visited[nodeNext] = 1;
//         for (int i = 0; i < n; i++){
//             if (!visited[i])
//                 if (minDist + cost[nodeNext][i] < Dist[i])
//                 {
//                     Dist[i] = minDist + cost[nodeNext][i];
//                     predecessor[i] = nodeNext;
//                 }
//         }
//         cnt++;
//     }
//     printf("Printing path to file for source node %d\n", sourcenode);
//     // print the path and distance of each node
//     for (int i = 0; i < n; i++){
//         if (i != sourcenode)
//         {   
//             fprintf(fp, "Distance of node %d = %d, ", i, Dist[i]);
//             fprintf(fp,"Path=%d", i);
//             do
//             {
//                 i = predecessor[i];
//                 fprintf(fp,"<-%d", i);
//             } while (i != sourcenode);
//             fprintf(fp, "\n");
//         }
//     }
// }

void printPath(int *parent, int currNode, FILE *fp) {
    if(currNode == -1) return;
    printPath(parent, parent[currNode], fp);
    fprintf(fp, "%d ", currNode);
}

void tryDij(int(*Graph)[COLS], int sourceNode, FILE *fp) {


    for(int i=0; i<ROWS; i++){
        shortestDists[i] = INFINITY;
        isAdded[i] = 0;
        parent[i] = -1;
    }

    shortestDists[sourceNode] = 0;
    for(int i=1; i < ROWS; i++) {
        int nearestNode = -1;
        int shortestDist = INFINITY;

        for(int j = 0; j < ROWS; j++) {
            if(!isAdded[j] && shortestDists[j] < shortestDist) {
                nearestNode = j;
                shortestDist = shortestDists[j];
            }
        }

        if(nearestNode == -1) continue;

        isAdded[nearestNode] = 1;

        for(int j = 0; j < ROWS; j++) {
            int weight = Graph[nearestNode][j+1];

            if(weight > 0 && ((shortestDist+weight)<shortestDists[j])) {
                parent[j] = nearestNode;
                shortestDists[j] = shortestDist+weight;
            }
        }
        
    }
    
    printf("Printing path to file for source node %d\n", sourceNode);
    for(int i=0; i < ROWS; i++) {
        if( i != sourceNode && shortestDists[i] != INFINITY) {

            fprintf(fp, "Distance of node %d from source node %d = %d, ", i, sourceNode, shortestDists[i]);
            fprintf(fp,"Path = ");
            printPath(parent, i, fp);
            fprintf(fp, "\n");
        }
    }
    fprintf(fp, "\n");

}


int main() {
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
        tryDij(array, consumerSet[i], fp);
        fprintf(fp, "\n\n");
    }
    fclose(fp);
    if (shmdt(array) == -1)
    {
        perror("shmdt");
        exit(1);
    }

    return 0;
}