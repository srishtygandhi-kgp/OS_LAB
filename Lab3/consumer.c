#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "macros.h"
#include "queue.h"

int **shortestDists, **isAdded, **parent;

void printPath(int *parent, int currNode, FILE *fp) {
    if(currNode == -1) return;
    printPath(parent, parent[currNode], fp);
    fprintf(fp, "%d ", currNode);
}

void Dijkstra(int(*Graph)[COLS], int sourceNode, FILE *fp, int nodeIndex) {

    // printf("source: %d\n", sourceNode);
    for(int i=0; i<ROWS; i++){
        shortestDists[nodeIndex][i] = INFINITE;
        isAdded[nodeIndex][i] = 0;
        parent[nodeIndex][i] = -1;
    }

    shortestDists[nodeIndex][sourceNode] = 0;
    for(int i=1; i < ROWS; i++) {
        int nearestNode = -1;
        int shortestDist = INFINITE;

        for(int j = 0; j < ROWS; j++) {
            if(!isAdded[nodeIndex][j] && shortestDists[nodeIndex][j] < shortestDist) {
                nearestNode = j;
                shortestDist = shortestDists[nodeIndex][j];
            }
        }

        if(nearestNode == -1) continue;

        isAdded[nodeIndex][nearestNode] = 1;

        for(int j = 0; j < ROWS; j++) {
            int weight = Graph[nearestNode][j+1];

            if(weight > 0 && ((shortestDist+weight)<shortestDists[nodeIndex][j])) {
                parent[nodeIndex][j] = nearestNode;
                shortestDists[nodeIndex][j] = shortestDist+weight;
            }
        }
        
    }
    
    for(int i=0; i < ROWS; i++) {
        if( i != sourceNode && shortestDists[nodeIndex][i] != INFINITE && Graph[i][0]>0) {

            fprintf(fp, "Distance of node %d from source node %d = %d, Path is: ", i, sourceNode, shortestDists[nodeIndex][i]);
            printPath(parent[nodeIndex], i, fp);
            fprintf(fp, "\n");
        }
    }
    fprintf(fp, "\n");

}

void appendDij(int(*Graph)[COLS], int existingNode, int existingNodeIndex, int *newNodeSet, int newNodes, FILE *fp) {

    for(int i = 0; i < newNodes; i++) {
        enqueue(newNodeSet[i]);
    }

    while(!isQueueEmpty()) {
        int currentNewNode = dequeue();

        if(Graph[currentNewNode][0] == 0) continue;

        int nearestNode = -1;
        int shortestDist = INFINITE;

        for(int j = 0; j < ROWS; j++) {
            int weight = Graph[currentNewNode][j+1];
            if(weight > 0 && shortestDists[existingNodeIndex][j] + weight < shortestDist) {
                nearestNode = j;
                shortestDist = shortestDists[existingNodeIndex][j] + weight;
            }
        }

        if(nearestNode == -1) continue;

        parent[existingNodeIndex][currentNewNode] = nearestNode;
        shortestDists[existingNodeIndex][currentNewNode] = shortestDist;
        isAdded[existingNodeIndex][currentNewNode] = 1;

        for(int j = 0; j < ROWS; j++) {
            int weight = Graph[currentNewNode][j+1];

            if(weight > 0 && ((shortestDist+weight)<shortestDists[existingNodeIndex][j])) {
                parent[existingNodeIndex][j] = currentNewNode;
                shortestDists[existingNodeIndex][j] = shortestDist+weight;

                enqueue(j);
            }
        }
    }

    for(int i=0; i < ROWS; i++) {
        if( i != existingNode && shortestDists[existingNodeIndex][i] != INFINITE && Graph[i][0]>0) {

            fprintf(fp, "Distance of node %d from source node %d = %d, Path is: ", i, existingNode, shortestDists[existingNodeIndex][i]);
            printPath(parent[existingNodeIndex], i, fp);
            fprintf(fp, "\n");
        }
    }
    fprintf(fp, "\n");
}

int main(int argc, char* argv[]) {
    int consumerID = atoi(argv[1]);
    int optimize = atoi(argv[2]);

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

    int(*array)[COLS] = (int(*)[COLS])shmat(shmid, NULL, 0);
    if (array == (int(*)[COLS]) - 1)
    {
        perror("shmat");
        exit(1);
    }

    shortestDists = (int **)malloc(sizeof(int *)*(ROWS/10));
    isAdded = (int **)malloc(sizeof(int *)*(ROWS/10));
    parent = (int **)malloc(sizeof(int *)*(ROWS/10));

    for(int i = 0; i < ROWS/10; i++) {
        shortestDists[i] = (int *)malloc(sizeof(int)*ROWS);
        isAdded[i] = (int *)malloc(sizeof(int)*ROWS);
        parent[i] = (int *)malloc(sizeof(int)*ROWS);
    }

    int totalNodesDone = 0;
    int consumerSet[ROWS/10], newNodeSet[ROWS/10];

    int myConsumerNodeCount = 0;
    for(int i = 0; i < ROWS; i++) {
        if (array[i][0] > 0 && i%10 == consumerID-1) consumerSet[myConsumerNodeCount++] = i;
    }

    while(1) {
        // Access the shared memory here
        int count = 0, newNodes = 0;
        for (int i = 0; i < ROWS; i++)
        {
            if (array[i][0] > 0) {
                count++;
            }
        }

        char filepath[25];
        sprintf(filepath, "consumer%d.txt", consumerID);
        FILE *fp = fopen(filepath, "aw");

        if(optimize && totalNodesDone != 0) {
            // optimized run

            for(int i = totalNodesDone; i < ROWS; i++) {
                if (array[i][0] > 0) {
                    newNodeSet[newNodes++] = i;
                }
            }

            // fix previously computed paths
            for(int i = 0; i < myConsumerNodeCount; i++) {
                appendDij(array, consumerSet[i], i, newNodeSet, newNodes, fp);
                fprintf(fp, "\n\n");
            }

            // run Djkstra’s shortest path algorithm with all new nodes in consumerSet as source node
            for(int i = 0; i < newNodes; i++) {

                if(newNodeSet[i] % 10 == consumerID-1) {
                    Dijkstra(array, newNodeSet[i], fp, myConsumerNodeCount);
                    fprintf(fp, "\n\n");

                    consumerSet[myConsumerNodeCount++] = newNodeSet[i];
                }
            }
        }
        else {
            // trivial run

            // run Djkstra’s shortest path algorithm with all nodes in consumerSet as source node
            for(int i = 0; i < myConsumerNodeCount; i++) {

                // make every element of consumerSet as source node for dijkstra
                Dijkstra(array, consumerSet[i], fp, i);
                fprintf(fp, "\n\n");
            }
        }

        totalNodesDone = count;

        fclose(fp);

        sleep(30);
    }

    if (shmdt(array) == -1)
    {
        perror("shmdt");
        exit(1);
    }

    return 0;
}