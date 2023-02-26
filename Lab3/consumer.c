#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "macros.h"

int shortestDists[ROWS], isAdded[ROWS], parent[ROWS];


void printPath(int *parent, int currNode, FILE *fp) {
    if(currNode == -1) return;
    printPath(parent, parent[currNode], fp);
    fprintf(fp, "%d ", currNode);
}

void tryDij(int(*Graph)[COLS], int sourceNode, FILE *fp) {


    for(int i=0; i<ROWS; i++){
        shortestDists[i] = INFINITE;
        isAdded[i] = 0;
        parent[i] = -1;
    }

    shortestDists[sourceNode] = 0;
    for(int i=1; i < ROWS; i++) {
        int nearestNode = -1;
        int shortestDist = INFINITE;

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
    
    for(int i=0; i < ROWS; i++) {
        if( i != sourceNode && shortestDists[i] != INFINITE) {

            fprintf(fp, "Distance of node %d from source node %d = %d, ", i, sourceNode, shortestDists[i]);
            fprintf(fp,"Path is: ");
            printPath(parent, i, fp);
            fprintf(fp, "\n");
        }
    }
    fprintf(fp, "\n");

}


int main(int argc, char* argv[]) {
    int consumerID = atoi(argv[1]);

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

    while(1) {
        // Access the shared memory here
        int count = 0;
        for (int i = 0; i < ROWS; i++)
        {
            if (array[i][0] > 0)
                count++;
        }

        // map the consumer to its set of nodes
        int nodeShare = (int)(ceil((double)count / 10));
        int prevNodeShare = (consumerID-1)*nodeShare;
        int consumerSet[nodeShare];
        int cnt = 0;
        
        int currIndex = 0;
        for(int i = 0; i < ROWS; i++, currIndex++) {
            if(array[i][0] > 0 && prevNodeShare > 0) prevNodeShare--;
            if(prevNodeShare == 0) break;
        }

        for (int i = currIndex+1; i < ROWS && cnt < nodeShare; i++)
        {
            if (array[i][0] > 0)
            {   
                // printf("array[%d][%d] = %d", i, 1, array[i][1]);
                consumerSet[cnt++] = i;
                // printf("putting node %d in consumer set\n", i);
            }
        }
        char filepath[25];
        sprintf(filepath, "consumer%d.txt", consumerID);
        // run Djkstra’s shortest path algorithm with all nodes in consumerSet as source node
        FILE *fp = fopen(filepath, "aw");
        for(int i = 0; i < count; i++) {
            // make every element of consumerSet as source node for dijkstra
            tryDij(array, consumerSet[i], fp);
            fprintf(fp, "\n\n");
        }
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