#ifndef _MAIN_H
#define _MAIN_H

#include <pthread.h>
#include <semaphore.h>
#include <stack>
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

#endif