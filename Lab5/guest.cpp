#include <iostream>
#include "main.h"

using namespace std;

int get_rand_inrange(int a, int b)
{
    return a + rand() % (b - a + 1); // generate a random number in the range [a, b]
}

int getLowerPriority(int guestID) {


    // check if any occupied room can be evicted.
    // int leastPriorityRoom = *(occupiedRooms.begin());
    // cout << "leastPriorityRoom " << leastPriorityRoom << endl;

    int currentRoom = -1;
    
    if (pthread_mutex_lock(&changeOccupiedRoom) != 0)
    {
        perror("pthread mutex changeTotalOccupied lock error occured.");
        exit(0);
    }
    
    for (auto itr : occupiedRooms)
    {
        if (guestPriorities[guestID] > guestPriorities[allRooms[itr].currentOccupant])
        {
            currentRoom = itr;
            break;
        }
    }

    if (currentRoom != -1) {
        cout << "Guest :" << guestID  << " Room: " << currentRoom << ", currentPriorityRoom " 
                << guestPriorities[allRooms[currentRoom].currentOccupant] 
                << ", guest priority " << guestPriorities[guestID] << "\n";
        occupiedRooms.erase(occupiedRooms.find(currentRoom));
    }

    if (pthread_mutex_unlock(&changeOccupiedRoom) != 0)
    {
        perror("pthread mutex changeTotalOccupied unlock error occured.");
        exit(0);
    }

    return currentRoom;
}

void allotRoom(int currentRoom, int guestID) {
    
    // modifies allRooms

    if (pthread_mutex_lock(&all_room) != 0)
    {
        perror("pthread mutex all_room lock error occured.");
        exit(0);
    }

    allRooms[currentRoom].available = false;
    allRooms[currentRoom].currentOccupant = guestID;

    // modifying occupiedRoom
    allRooms[currentRoom].pastOccupants++;

    if (pthread_mutex_unlock(&all_room) != 0)
    {
        perror("pthread mutex all_room unlock error occured.");
        exit(0);
    }

    if (pthread_mutex_lock(&changeOccupiedRoom) != 0)
    {
        perror("pthread mutex changeTotalOccupied lock error occured.");
        exit(0);
    }

    occupiedRooms.insert(currentRoom);

    if (pthread_mutex_unlock(&changeOccupiedRoom) != 0)
    {
        perror("pthread mutex changeTotalOccupied unlock error occured.");
        exit(0);
    }    
    
    // modify the global counter
    if (pthread_mutex_lock(&changeTotalOccupied) != 0)
    {
        perror("pthread mutex changeTotalOccupied lock error occured.");
        exit(0);
    }
    totalOccupiedSinceLastClean++;
    cout << "allotRoom totalOcc:" << totalOccupiedSinceLastClean << endl;

    if ( totalOccupiedSinceLastClean == 2 * n ) {
        cout << "ITS TIME" << " unaval rooms: " << unavailableRooms.size() << " aval rooms: " << availableRooms.size() << " occumpied: " << occupiedRooms.size() << endl;
        is_cleaning = 1;
        
        for (int i = 0; i < y; i ++)
            pthread_kill(guest_thread[i], SIGUSR1);
        
        for (int i = 0; i < x; i ++)
            pthread_kill(cleaning_staff[i], SIGUSR1);
        
        cout << "done" << endl;
    }

    if (pthread_mutex_unlock(&changeTotalOccupied) != 0)
    {
        perror("pthread mutex changeTotalOccupied unlock error occured.");
        exit(0);
    }

}

int getRoom(int guestID)
{
    while (1)
    {
        if ( sigsetjmp(guest_env[guestID], 1) == 69 ) {
            cout << "back here" << endl;
        }

        int val;
        sem_getvalue(&roomSemaphore, &val);

        cout << "[LOG] roomSemaphore: " << val << endl;

        int temp = sem_trywait(&roomSemaphore);
        if (temp == 0)
        {
            // got the room
            cout << "getRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << " " << totalOccupiedSinceLastClean << "\n";

            if (pthread_mutex_lock(&aval_room) != 0)
            {
                perror("pthread mutex aval_room lock error occured.");
                exit(0);
            }

            int currentRoom = availableRooms.top();
            
            // modifies availableRooms
            availableRooms.pop();

            if (pthread_mutex_unlock(&aval_room) != 0)
            {
                perror("pthread mutex aval_room unlock error occured.");
                exit(0);
            }

            allotRoom(currentRoom, guestID);

            return currentRoom;
        }
        else if (errno == EAGAIN)
        {
            // all occupied. need to evict
            int _check = 0;

            if (pthread_mutex_lock(&changeOccupiedRoom) != 0)
            {
                perror("pthread mutex changeTotalOccupied lock error occured.");
                exit(0);
            }

            if (occupiedRooms.empty())
                _check = 1;

            if (pthread_mutex_unlock(&changeOccupiedRoom) != 0)
            {
                perror("pthread mutex changeTotalOccupied unlock error occured.");
                exit(0);
            }

            if (_check)
                continue;
            
            int currentRoom = getLowerPriority(guestID);

            // there is no lower priority guest, hence no room
            if(currentRoom == -1) 
                return currentRoom;
            
            // alloting if there is lower priority available
            allotRoom(currentRoom, guestID);

            return currentRoom;
        }
        else
        {
            perror("semaphore wait error occured!");
            exit(0);
        }
    }
}

void vacateRoom(int guestID, int currentRoom)
{
    // check to make sure that an evicted guest does not attempt to vacacte the room 
    if(allRooms[currentRoom].currentOccupant != guestID)
        return;
    // cout << "vacateRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << endl;
    // remove from set
    
    if (pthread_mutex_lock(&changeOccupiedRoom) != 0)
    {
        perror("pthread mutex changeTotalOccupied lock error occured.");
        exit(0);
    }
    
    auto it = occupiedRooms.find(currentRoom);

    // why do we need to check this?
    if (it != occupiedRooms.end())
        occupiedRooms.erase(it);
    
    if (pthread_mutex_unlock(&changeOccupiedRoom) != 0)
    {
        perror("pthread mutex changeTotalOccupied unlock error occured.");
        exit(0);
    }
    // cout << "vacateRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << endl;

    // cout << "vacateRoom: " << currentRoom << " " << allRooms[currentRoom].pastOccupants << "\n";

    int is_usable;

    if (pthread_mutex_lock(&all_room) != 0)
    {
        perror("pthread mutex all_room lock error occured.");
        exit(0);
    }

    allRooms[currentRoom].available = true;
    allRooms[currentRoom].currentOccupant = -1;
    is_usable = (allRooms[currentRoom].pastOccupants != 2);

    if (pthread_mutex_unlock(&all_room) != 0)
    {
        perror("pthread mutex all_room unlock error occured.");
        exit(0);
    }

    if (!is_usable) {

        // unavailableRooms modify

        if (pthread_mutex_lock(&unaval_room) != 0)
        {
            perror("pthread mutex unaval_room lock error occured.");
            exit(0);
        }

        cout << "Room : " << currentRoom << " unavailable\n";

        unavailableRooms.push(currentRoom);
        pthread_cond_broadcast(&cv_unaval);

        if (pthread_mutex_unlock(&unaval_room) != 0)
        {
            perror("pthread mutex unaval_room unlock error occured.");
            exit(0);
        }
    }
    else
    {
        // availableRooms modify
        if (pthread_mutex_lock(&aval_room) != 0)
        {
            perror("pthread mutex aval_room lock error occured.");
            exit(0);
        }

        availableRooms.push(currentRoom);
        if (sem_post(&roomSemaphore) != 0)
        {
            perror("semaphore post error occured!");
            exit(0);
        }

        if (pthread_mutex_unlock(&aval_room) != 0)
        {
            perror("pthread mutex aval_room unlock error occured.");
            exit(0);
        }

    }

    // cout << "vacateRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << " " << totalOccupiedSinceLastClean << "\n";
}

void guest_evict_handler(int signum) {
    char msg[] = "getevicted!\n";
    write(1, msg, strlen(msg));
}

void cleaner_finish_handler(int signum) {
    char msg[] = "Cleaner done\n";
    write(1, msg, strlen(msg));
}

void *guest(void *arg)
{
    int guestID = *(int *)arg;
    signal(SIGUSR1, guest_evict_handler);
    signal(SIGUSR2, cleaner_finish_handler);

    while (1)
    {

        int signum;

        while (is_cleaning) {

            cout << "Guest " << guestID << " waiting on cleaning" << endl;
            pthread_cond_broadcast(&cv_unaval);
            sigwait(&clean_set, &signum);
            cout << "Guest "<< guestID << " cleaning sorted" << endl;

            if (signum != SIGUSR2) {
                cout << "Guest " << guestID 
                    << " signal other than SIGUSR1 recvd" << endl;
            }
        }

        // sleeps for random time first
        int randomSleepTime = get_rand_inrange(RANDOM_SLEEP_TIME_MIN, RANDOM_SLEEP_TIME_MAX);
        cout << "Guest " << guestID << " : "
             << "init sleeping for " << randomSleepTime << " seconds " << "| unaval: " << unavailableRooms.size() << " aval: " << availableRooms.size() << " occ: " << occupiedRooms.size() << "\n";;
        sleep(randomSleepTime);

        int currentRoom = getRoom(guestID);

        if (currentRoom == -1)
            continue;
        
        cout << "Guest " << guestID << " : "
             << "got room " << currentRoom << endl;

        int time_left;

        if (!is_cleaning) {
            int randomStayTime = get_rand_inrange(RANDOM_STAY_TIME_MIN, RANDOM_STAY_TIME_MAX);
            cout << "Guest " << guestID << " : "
                << "staying for " << randomStayTime << " seconds" << "unaval: " << unavailableRooms.size() << " aval: " << availableRooms.size() << " occ: " << occupiedRooms.size() << "\n";;
            time_left = sleep(randomStayTime);

            if (pthread_mutex_lock(&all_room) != 0)
            {
                perror("pthread mutex all_room lock error occured.");
                exit(0);
            }

            allRooms[currentRoom].totalTimeLived += randomStayTime - time_left;

            if (pthread_mutex_unlock(&all_room) != 0)
            {
                perror("pthread mutex all_room unlock error occured.");
                exit(0);
            }

        } else { // basically the 2n'th guest will trigger this

            cout << "Guest " << guestID << " getting evicted!" << endl;
        }

        vacateRoom(guestID, currentRoom);

        if (!time_left) {
            cout << "Guest " << guestID << " : "
                << "vacated the room : " << currentRoom << "unaval: " << unavailableRooms.size() << " aval: " << availableRooms.size() << " occ: " << occupiedRooms.size() << "\n";
            
        } else { // printing for the ones that were sleeping at the time of eviction

            cout << "Guest " << guestID << " getting evicted!" << endl;
        }
    }

    return NULL;
}
