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

extern int totalOccupiedSinceLastClean;
extern int *guestPriorities;

extern pthread_mutex_t changeTotalOccupied, changeOccupiedRoom;

extern sem_t roomSemaphore;

typedef struct _room {
    bool available;
    int pastOccupants;
    int currentOccupant;
    time_t totalTimeLived;

    bool operator<(const _room& rhs) const {
        return guestPriorities[currentOccupant] > guestPriorities[rhs.currentOccupant];
    }
} Room;

extern Room* allRooms;
extern stack<int> availableRooms, unavailableRooms;
extern set<int> occupiedRooms; // can be evicted

#define PROP_CONST 7
#define RANDOM_SLEEP_TIME_MIN 10
#define RANDOM_SLEEP_TIME_MAX 20
#define RANDOM_STAY_TIME_MIN 10
#define RANDOM_STAY_TIME_MAX 30

// take input x, y and n
extern int x, y, n;

extern sigjmp_buf *guest_env;

extern sigset_t evict_set, clean_set;
extern int is_cleaning;
extern pthread_cond_t cv_unaval;
extern pthread_mutex_t aval_room;
extern pthread_mutex_t unaval_room;
extern pthread_mutex_t all_room;

extern pthread_t *guest_thread;
extern pthread_t *cleaning_staff;

int get_rand_inrange(int a, int b);
int getLowerPriority(int guestID);
void allotRoom(int currentRoom, int guestID);
void allotRoom(int currentRoom, int guestID);
int getRoom(int guestID);
void vacateRoom(int guestID, int currentRoom);
void guest_evict_handler(int signum);
void cleaner_start_handler(int signum);
void cleaner_finish_handler(int signum);
extern void *guest(void *arg);
extern void *cleaner(void *arg);

#endif