#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


#define FILE_PATH "facebook_combined.txt"
#define MAX_INT 5000
#define KEY 1234
#define FILE_PATH_FOR_KEY "main.c"
#define PROJECT_ID 1
#define ROWS 8000 // double the number of initial rows
#define COLS 8000 // 4 times the number of initial rows

int get_rand_inrange(int a, int b) {
  srand(time(0));  // seed the random number generator with the current time

  return a + rand() % (b - a + 1);  // generate a random number in the range [a, b]
}

int select_new_node(int ** input_graph){
    for(int i = 0; i < ROWS; i++){
        if(input_graph[i][0] == 0){
            printf("%d -- new_node\n", i);
            return i;}
    }
    return -1;
}

int select_node(int ** input_graph){
    long long int total_degree = 0;
    for (int i = 0; i < ROWS; i++) {
        total_degree += input_graph[i][0];  // sum up all the degrees
    }
    int randomNumber = get_rand_inrange(1, total_degree);
    int sum = 0;
    int k = 0;
    for (int i = 0; i < ROWS; i++) {
        sum += input_graph[i][0];  // add the degree of each node to the sum
        if (randomNumber <= sum) {  // when the random number is less than or equal to the sum, we've found the node
            k = i;
            break;
        }
    }
    printf("%d -- existing node",k);
    return k;
}

void add_node_to_graph(int first, int second, int ** input_graph){
    if(input_graph[first][second+1] == 0){
        input_graph[first][second+1] = 1;
        input_graph[second][first+1] = 1;
        input_graph[first][0] +=1;
        input_graph[second][0] += 1;
        printf("added new edge\n");
        return;
    }
    printf("edge already exists\n");
}

void update_graph(int ** input_graph){
    int m = get_rand_inrange(10,30);
    printf("%d --m\n",m);
    for(int i = 0; i < m; i++){
        printf("i -- %d", i);
        int new_node = select_new_node(input_graph);
        int k = get_rand_inrange(1,20);
        printf("%d --k\n",k);
        for(int j = 0; j < k; j++){
            // select existing node
            int existing_node = select_node(input_graph);
            add_node_to_graph(new_node,existing_node, input_graph);
        }
    }
}

int main() {
    key_t key;
    key = ftok(FILE_PATH_FOR_KEY, PROJECT_ID);
    // printf("%d",key);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    int shmid = shmget(key, 0, 0);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    int (*array)[COLS] = (int (*)[COLS]) shmat(shmid, NULL, 0);
    if (array == (int (*)[COLS]) -1) {
        perror("shmat");
        exit(1);
    }

    // Access the shared memory here
//     for(int i = 0; i < 30; i++){
//         for(int j = 0; j < 10; j++){
//             printf("%d ",array[i][j] );
//         }
//         printf("\n");
//     }
  
  while(1){
        // update_graph(array);
        int m = get_rand_inrange(10,30);
        // printf("\n m -- %d\n",m);
        for(int i = 0; i < m; i++){
            // printf("%d ",i);
            int new_node;
            for(int a = 0; a<ROWS; a++){
                if(array[a][0] == 0){
                    new_node = a;
                    // printf("new_node -- %d", new_node);
                    break;
                }
            }

            int k = get_rand_inrange(1,20);
            for(int a = 0; a<k; a++){
                int existing_node;
                long long int total_degree = 0;
                for(int b = 0; b < ROWS; b++){
                    total_degree += array[b][0];
                }
                // printf("%d -- total degree\n",total_degree);
                int randomNumber = get_rand_inrange(1,total_degree);
                int sum = 0;
                // int c = 0;
                for(int b = 0; b < ROWS; b++){
                    sum += array[b][0];
                    if(randomNumber <= sum){
                        existing_node = b;
                        break;
                    }
                }
                // existing_node = c;
                // printf("%d -- existing node \n",existing_node);
                // add edge to graph
                if(array[new_node][existing_node+1] == 0){
                    array[new_node][existing_node+1] = 1;
                    array[existing_node][new_node+1] = 1;
                    array[new_node][0] +=1;
                    array[existing_node][0] += 1;
                    // printf("added new edge\n");
                }
            }

            

        }
        // printf("\n");
        sleep(50);
    }

    if (shmdt(array) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;

}
