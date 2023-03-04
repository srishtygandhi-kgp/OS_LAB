#include <iostream>
#include <bits/stdc++.h>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <wait.h>

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

int main(){
    
    srand(time(0));  // seed the random number generator with the current time
    vector<vector<int>> graph(ROWS);

    for(int i=0; i<ROWS; i++) {
        nodes[i].id = i;
        int randInt = rand()%2;
        nodes[i].priority = randInt;
        nodes[i].degree = 0;
    }

    // populate the graph
    populate_graph(graph);
    printf("Number of nodes -> %ld\n", graph.size());

    
}