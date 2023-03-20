#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "main.h"

using namespace std;

int get_rand_inrange(int a, int b) {
  return a + rand() % (b - a + 1);  // generate a random number in the range [a, b]
}

int getRoom(int guestID) {
    while(1) {
        int temp = sem_trywait(&roomSemaphore);
        if(temp == 0) {
            // got the room
            cout << "getRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << endl;
            int currentRoom = availableRooms.top();
            availableRooms.pop();
            allRooms[currentRoom].available = false;
            allRooms[currentRoom].currentOccupant = guestID;
            if(allRooms[currentRoom].pastOccupants++ == 0) occupiedRooms.insert(currentRoom);
            return currentRoom;
        }
        else if(errno == EAGAIN) {
            // all occupied. need to evict
            if(occupiedRooms.empty()) continue;

            // check if any occupied room can be evicted.
            int leastPriorityRoom = *(occupiedRooms.begin());
            // cout << "leastPriorityRoom " << leastPriorityRoom << endl;
            if(guestPriorities[allRooms[leastPriorityRoom].currentOccupant] < guestPriorities[guestID]) {
                // evict @sarita
                // occupiedRooms.erase(leastPriorityRoom);
            }
        }
        else {
            perror("semaphore wait error occured!");
            exit(0);
        }
    }
}

void vacateRoom(int guestID, int currentRoom) {

    // cout << "vacateRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << endl;
    //remove from priority queue
    auto it = occupiedRooms.find(currentRoom);
    if(it != occupiedRooms.end()) occupiedRooms.erase(it);
    // cout << "vacateRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << endl;

    cout << "vacateRoom: " << currentRoom << " " << allRooms[currentRoom].pastOccupants << endl;
    allRooms[currentRoom].available = true;
    allRooms[currentRoom].currentOccupant = -1;
    if(allRooms[currentRoom].pastOccupants == 2) unavailableRooms.push(currentRoom);
    else {
        availableRooms.push(currentRoom);
        if(sem_post(&roomSemaphore) != 0) {
            perror("semaphore post error occured!");
            exit(0);
        }
    }

    cout << "vacateRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << endl;
}

void *guest(void *arg) {

    int guestID = *(int *)arg;

    while(1) {
        // sleeps for random time first
        int randomSleepTime = get_rand_inrange(10, 20);
        cout << "Guest " << guestID << " : " << "init sleeping for " << randomSleepTime << " seconds." << endl;
        sleep(randomSleepTime);

        int currentRoom = getRoom(guestID);
        cout << "Guest " << guestID << " : " << "got room " << currentRoom << endl;

        int randomStayTime = get_rand_inrange(10, 30);
        cout << "Guest " << guestID << " : " << "staying for " << randomStayTime << " seconds." << endl;
        sleep(randomStayTime);
        allRooms[currentRoom].totalTimeLived += randomStayTime;

        vacateRoom(guestID, currentRoom);
        cout << "Guest " << guestID << " : " << "vacated the room." << endl;
    }
}


void *cleaner(void *arg) {

}

int main()
{
    // init random seed
    srand(time(NULL));

    // take input x, y and n
    int x, y, n;
    do
    {
        cout << "Enter the number of guest thread(Y): ";
        cin >> y;
        cout << "Enter the number of cleaning staff thread(X): ";
        cin >> x;
        cout << "Enter the number of rooms(N): ";
        cin >> n;
        if (!(y > n && n > x && x > 1))
        {
            cout << "Please enter Y, X, N which satisfies Y>N>X>1\n";
        }
        else
            break;
    } while (1);

    // Init rooms
    allRooms = (Room *)malloc(n * sizeof(Room));
    for (int i = 0; i < n; i++)
    {
        allRooms[i] = {
            .available = true,
            .pastOccupants = 0,
            .currentOccupant = -1,
            .totalTimeLived = 0};
        availableRooms.push(i);
    }

    // randomly assign priority to guests and store it in array
    guestPriorities = (int *)malloc(y * sizeof(int));
    for (int i = 0; i < y; i++)
        guestPriorities[i] = 1 + rand() % y;

    totalOccupiedSinceLastClean = 0;

    // initialize mutex for total occupied change
    pthread_mutex_init(&changeTotalOccupied, NULL);

    //initialize room semaphore
    sem_init(&roomSemaphore, 0, n);

    // explicitly creating threads in a joinable state 
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // create guest thread
    int *guest_id = (int *)malloc(y * sizeof(int));
    pthread_t *guest_thread = (pthread_t *)malloc(y * sizeof(pthread_t));
    for (int i = 0; i < y; i++)
    {
        guest_id[i] = i;
        pthread_create(&guest_thread[i], &attr, guest, &guest_id[i]);
    }

    // create cleaning staff thread
    int *cleaning_staff_id = (int *)malloc(x * sizeof(int));
    pthread_t *cleaning_staff = (pthread_t *)malloc(x * sizeof(pthread_t));
    for (int i = 0; i < x; i++)
    {
        cleaning_staff_id[i] = i;
        pthread_create(&cleaning_staff[i], &attr, cleaner, &cleaning_staff_id[i]);
    }

    // wait for guest threads
    for(int i = 0; i < y; i++) {
        pthread_join(guest_thread[i], NULL);
    }

    // wait for cleaning staff threads
    for(int i = 0; i < x; i++) {
        pthread_join(cleaning_staff[i], NULL);
    }

    return 0;
}