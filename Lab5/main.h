#ifndef _MAIN_H
#define _MAIN_H

typedef struct _room {
    bool available;
    int pastOccupants;
    int currentOccupant;
    time_t totalTimeLived;
} Room;

#endif