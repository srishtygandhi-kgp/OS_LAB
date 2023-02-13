#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "macros.h"

int main() {
    key_t key;
    key = ftok(FILE_PATH_FOR_KEY, PROJECT_ID);

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
    int count = 0;
    for(int i = 0; i < ROWS; i++){
        if(array[i][0] > 0) 
            count++;
    }
    printf("total nodes: %d\n", count);

    if (shmdt(array) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}