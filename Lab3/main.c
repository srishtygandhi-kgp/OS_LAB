#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>

#include "macros.h"

void calculate_num_nodes(char* file_path, int* graph_size){
    //graph_size[0] --> uniques, integers in the file  which is loaded --> max number of nodes
    // graph_size[1] --> max_degree of a node... max_size of adjacency list
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Unable to open file %s\n", file_path);
        return ;
    }
    char line[20];
    int count[MAX_INT];
    memset(count,0,sizeof(count));
    while (fgets(line, 20, file) ) {
        char *token = strtok(line, " ");
        int first = atoi(token);
        token = strtok(NULL, " ");
        int second = atoi(token);
        count[first]++;
        count[second]++;
    }

    int max_node = MAX_INT;
    int max_degree = 0;
    for(int i = 0; i < MAX_INT; i++){
        if(count[i] > max_degree)
            max_degree = count[i];
        if(count[i] == 0)
            max_node--;
    }

    graph_size[0] = max_node;
    graph_size[1] = max_degree;
}
int ** make_graph(int rows, int columns,int key, int * shmid){
   *shmid = shmget(key, rows * columns * sizeof(int), IPC_CREAT | 0666);
    if (*shmid < 0) {
        perror("shmget");
        exit(1);
    }

    int* array = (int*) shmat(*shmid, NULL, 0);
    if (array == (int*) -1) {
        perror("shmat");
        exit(1);
    }

    // Allocate space for the array of pointers
    int** arr_ptr = (int**) malloc(rows * sizeof(int*));

    // Set each pointer to point to a different row in the shared memory segment
    for (int i = 0; i < rows; i++) {
        arr_ptr[i] = &array[i * columns];
    }

    // Initialize the array with -1
    for (int i = 0; i < rows * columns; i++) {
        array[i] = 0;
    }

    return arr_ptr;
}

void populate_graph(int ** graph_input){
    // reads the file and populates the graph
    // first value of every row tells the degree of the node, i.e. input_graph[i][0] = degree of node i
    // assuming the edges are non directed

    FILE *file = fopen(FILE_PATH, "r");
    if (file == NULL) {
        printf("Unable to open file %s\n", FILE_PATH);
        return ;
    }
    char line[20];
    while (fgets(line, 20, file) ) {
        char *token = strtok(line, " ");
        int first = atoi(token);
        token = strtok(NULL, " ");
        int second = atoi(token);
        // int a = first + 1;
        // int b = second + 1;
        graph_input[first][second+1] = 1;
        graph_input[second][first+1] = 1;
        graph_input[first][0] +=1;
        // printf("%d "arr[])
        graph_input[second][0] += 1;
    }

}


int main(){
    // calculate the size of graph
    // graph_size[0] --> max_num of nodes
    // graph_size[1] --> max_length of nodes
    int graph_size[2];
    calculate_num_nodes(FILE_PATH, graph_size);
    printf("%d %d\n", graph_size[0], graph_size[1]);
    // create a shared memory, --> 2d array of double size, initialised by -1
    
    key_t key ;
    key = ftok(FILE_PATH_FOR_KEY, PROJECT_ID);
    // printf("key --> %d\n",key);
    
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    int ** graph;
    int shmid;
    graph = make_graph(ROWS,COLS,key,&shmid);

    // populate the graph
    populate_graph(graph);

    // // run producer
    // if(fork() == 0) {
    //     char *args[]={"./producer",NULL};
    //     execvp(args[0], args);
    // }

    // // run consumers
    // for(int i = 1; i <= 10; i++) {
    //     char consumerID[3];
    //     sprintf(consumerID, "%d", i);
    //     if(fork() == 0) {
    //         char *args[]={"./consumer", consumerID, NULL};
    //         execvp(args[0], args);
    //     }
    // }
}
