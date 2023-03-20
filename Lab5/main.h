#ifndef _MAIN_H
#define _MAIN_H

#include <stack>
#include <queue>

int totalOccupiedSinceLastClean;
int *guestPriorities;

pthread_mutex_t changeTotalOccupied;

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

stack<Room> availableRooms, unavailableRooms;
priority_queue<Room> occupiedRooms; // can be evicted

#endif