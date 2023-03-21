#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "main.h"

using namespace std;

int get_rand_inrange(int a, int b)
{
    return a + rand() % (b - a + 1); // generate a random number in the range [a, b]
}

int getRoom(int guestID)
{
    while (1)
    {
        int temp = sem_trywait(&roomSemaphore);
        if (temp == 0)
        {
            // got the room
            cout << "getRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << " " << totalOccupiedSinceLastClean << "\n";
            int currentRoom = availableRooms.top();
            availableRooms.pop();
            allRooms[currentRoom].available = false;
            allRooms[currentRoom].currentOccupant = guestID;
            if (allRooms[currentRoom].pastOccupants++ == 0)
                occupiedRooms.insert(currentRoom);

            if (pthread_mutex_lock(&changeTotalOccupied) != 0)
            {
                perror("pthread mutex changeTotalOccupied lock error occured.");
                exit(0);
            }
            totalOccupiedSinceLastClean++;
            if (pthread_mutex_unlock(&changeTotalOccupied) != 0)
            {
                perror("pthread mutex changeTotalOccupied unlock error occured.");
                exit(0);
            }
            return currentRoom;
        }
        else if (errno == EAGAIN)
        {
            // all occupied. need to evict
            if (occupiedRooms.empty())
                continue;

            // check if any occupied room can be evicted.
            // int leastPriorityRoom = *(occupiedRooms.begin());
            // cout << "leastPriorityRoom " << leastPriorityRoom << endl;

            int currentRoom=-1;
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
            if(currentRoom != -1) {
            cout << "Guest id " << guestID << ", currentPriorityRoom " << guestPriorities[allRooms[currentRoom].currentOccupant] << ", guest priority " << guestPriorities[guestID] << "\n";
            occupiedRooms.erase(occupiedRooms.find(currentRoom));
            }
            if (pthread_mutex_unlock(&changeOccupiedRoom) != 0)
            {
                perror("pthread mutex changeTotalOccupied unlock error occured.");
                exit(0);
            }
            if(currentRoom == -1) 
                continue;
            allRooms[currentRoom].available = false;
            allRooms[currentRoom].currentOccupant = guestID;
            allRooms[currentRoom].pastOccupants++;

            if (pthread_mutex_lock(&changeTotalOccupied) != 0)
            {
                perror("pthread mutex changeTotalOccupied lock error occured.");
                exit(0);
            }
            totalOccupiedSinceLastClean++;
            if (pthread_mutex_unlock(&changeTotalOccupied) != 0)
            {
                perror("pthread mutex changeTotalOccupied unlock error occured.");
                exit(0);
            }
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

    // cout << "vacateRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << endl;
    // remove from set
    // if (pthread_mutex_lock(&changeOccupiedRoom) != 0)
    // {
    //     perror("pthread mutex changeTotalOccupied lock error occured.");
    //     exit(0);
    // }
    // auto it = occupiedRooms.find(currentRoom);
    // if (it != occupiedRooms.end())
    //     occupiedRooms.erase(it);
    // if (pthread_mutex_unlock(&changeOccupiedRoom) != 0)
    // {
    //     perror("pthread mutex changeTotalOccupied unlock error occured.");
    //     exit(0);
    // }
    // cout << "vacateRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << endl;

    cout << "vacateRoom: " << currentRoom << " " << allRooms[currentRoom].pastOccupants << "\n";
    allRooms[currentRoom].available = true;
    allRooms[currentRoom].currentOccupant = -1;
    if (allRooms[currentRoom].pastOccupants == 2)
        unavailableRooms.push(currentRoom);
    else
    {
        availableRooms.push(currentRoom);
        if (sem_post(&roomSemaphore) != 0)
        {
            perror("semaphore post error occured!");
            exit(0);
        }
    }

    cout << "vacateRoom: " << availableRooms.size() << " " << occupiedRooms.size() << " " << unavailableRooms.size() << " " << totalOccupiedSinceLastClean << "\n";
}

void *guest(void *arg)
{

    int guestID = *(int *)arg;

    while (1)
    {
        // sleeps for random time first
        int randomSleepTime = get_rand_inrange(10, 20);
        cout << "Guest " << guestID << " : "
             << "init sleeping for " << randomSleepTime << " seconds.\n";
        sleep(randomSleepTime);

        int currentRoom = getRoom(guestID);
        cout << "Guest " << guestID << " : "
             << "got room " << currentRoom << endl;

        int randomStayTime = get_rand_inrange(10, 30);
        cout << "Guest " << guestID << " : "
             << "staying for " << randomStayTime << " seconds.\n";
        sleep(randomStayTime);
        allRooms[currentRoom].totalTimeLived += randomStayTime;

        vacateRoom(guestID, currentRoom);
        cout << "Guest " << guestID << " : "
             << "vacated the room.\n";
    }
}

void *cleaner(void *arg)
{
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
            cout << "\nPlease enter Y, X, N which satisfies Y>N>X>1\n\n";
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
    for (int i = 0; i < y; i++) {
        guestPriorities[i] = 1 + rand() % y;
        cout<<guestPriorities[i]<<" ";
    }
    cout<<"\n";
    totalOccupiedSinceLastClean = 0;

    // initialize mutex for total occupied change
    pthread_mutex_init(&changeTotalOccupied, NULL);

    // initialize mutex for total occupied change
    pthread_mutex_init(&changeOccupiedRoom, NULL);

    // initialize room semaphore
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
    for (int i = 0; i < y; i++)
    {
        pthread_join(guest_thread[i], NULL);
    }

    // wait for cleaning staff threads
    for (int i = 0; i < x; i++)
    {
        pthread_join(cleaning_staff[i], NULL);
    }

    return 0;
}