#ifndef _MAIN_H
#define _MAIN_H

#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stack>
#include <signal.h>
#include <setjmp.h>
#include <set>

using namespace std;

int totalOccupiedSinceLastClean;
int *guestPriorities;

pthread_mutex_t changeTotalOccupied, changeOccupiedRoom;

sem_t roomSemaphore;

typedef struct _room {
    bool available;
    int pastOccupants;
    int currentOccupant;
    time_t totalTimeLived;

    bool operator<(const _room& rhs) const {
        return guestPriorities[currentOccupant] > guestPriorities[rhs.currentOccupant];
    }
} Room;

Room* allRooms;
stack<int> availableRooms, unavailableRooms;
set<int> occupiedRooms; // can be evicted

#define PROP_CONST 7
#define RANDOM_SLEEP_TIME_MIN 10
#define RANDOM_SLEEP_TIME_MAX 20
#define RANDOM_STAY_TIME_MIN 10
#define RANDOM_STAY_TIME_MAX 30

// take input x, y and n
int x, y, n;

sigjmp_buf *guest_env;

sigset_t evict_set, clean_set;
int is_cleaning;
pthread_cond_t cv_unaval = PTHREAD_COND_INITIALIZER;
pthread_mutex_t aval_room = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t unaval_room = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t all_room = PTHREAD_MUTEX_INITIALIZER;

pthread_t *guest_thread;
pthread_t *cleaning_staff;

int get_rand_inrange(int a, int b);
int getLowerPriority(int guestID);
void allotRoom(int currentRoom, int guestID);
int getRoom(int guestID);
void vacateRoom(int guestID, int currentRoom);
void guest_evict_handler(int signum);
void cleaner_start_handler(int signum);
void cleaner_finish_handler(int signum);
void *guest(void *arg);
void *cleaner(void *arg);

#endif