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

#include "macros.cxx"
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
    }

}

int main(){
    
    vector<vector<int>> graph(ROWS);

    // populate the graph
    populate_graph(graph);
    printf("Number of nodes -> %ld\n", graph.size());

    
}