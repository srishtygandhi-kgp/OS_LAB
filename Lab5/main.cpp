#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <setjmp.h>
#include "main.h"

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
        
        cout << "doneeeeeeee" << endl;
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

void cleaner_start_handler(int signum) {
    char msg[] = "Cleaner starting\n";
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

void *cleaner(void *arg)
{

    int cleanerID = *(int *)arg;
    signal(SIGUSR1, cleaner_start_handler);
    while (1) {

        int signum, did_clean = 0;

        // we wait here to be woken up by the signal
        while (!is_cleaning) {
            cout << "Cleaner " << cleanerID << " waiting to get dirty" << endl;
            sigwait(&evict_set, &signum);
            cout << "Cleaner " << cleanerID << " starting" << endl;
            
            if (signum != SIGUSR1) {
                cout << "Cleaner " << cleanerID 
                    << " signal other than SIGUSR1 recvd" << endl;
            }
        }

        //while (1) {
            // getting the room to clean first
            //cout << "Cleaner " << cleanerID << " gonna clean" << endl;
            
            int val, currentRoom = -1;
            sem_getvalue(&roomSemaphore, &val);

            //cout << "Cleaner " << cleanerID << " the number of rooms to be cleaned at loop start " << n - val << endl;

            /*
            if (val == n) {
                cout<< "Cleaner " << cleanerID << " sending signal when val = n";
                pthread_cond_broadcast(&cv_unaval);
                break;
            }
            */

            if (pthread_mutex_lock(&unaval_room) != 0)
            {
                perror("pthread mutex unaval_room lock error occured.");
                exit(0);
            }


            if ( !unavailableRooms.empty() ) {
                currentRoom = unavailableRooms.top();
                unavailableRooms.pop();
            }


            if (pthread_mutex_unlock(&unaval_room) != 0)
            {
                perror("pthread mutex unaval_room unlock error occured.");
                exit(0);
            }

            // looking into the allrooms array and changing for
            // the given room

            //cout << "Cleaner " << cleanerID << " right out the lock"<< endl;

            if ( currentRoom != -1 ) {

                //cout << "Cleaner " << cleanerID << " changing"<< endl;

                if (pthread_mutex_lock(&all_room) != 0)
                {
                    perror("pthread mutex all_room lock error occured.");
                    exit(0);
                }

                allRooms[currentRoom].available = true;
                allRooms[currentRoom].currentOccupant = -1;
                allRooms[currentRoom].pastOccupants = 0;
                time_t lived = allRooms[currentRoom].totalTimeLived;
                allRooms[currentRoom].totalTimeLived = 0;

                if (pthread_mutex_unlock(&all_room) != 0)
                {
                    perror("pthread mutex all_room unlock error occured.");
                    exit(0);
                }

                //cout << "Cleaner " << cleanerID << " done with allRooms"<< endl;

                // availableRooms modify
                if (pthread_mutex_lock(&aval_room) != 0)
                {
                    perror("pthread mutex aval_room lock error occured.");
                    exit(0);
                }

                availableRooms.push(currentRoom);

                if (pthread_mutex_unlock(&aval_room) != 0)
                {
                    perror("pthread mutex aval_room unlock error occured.");
                    exit(0);
                }

                cout << "Cleaner " << cleanerID << " : modified " << lived << "|"<< currentRoom << endl;

                sleep((int) lived / PROP_CONST);

                if (sem_post(&roomSemaphore) != 0)
                {
                    perror("semaphore post error occured!");
                    exit(0);
                }

                int val;
                sem_getvalue(&roomSemaphore, &val);

                cout << "Cleaner " << cleanerID << " the number of rooms to be cleaned actually " << n - val << endl;
                did_clean += 1;

                /*
                if (val >= n) {
                    cout << "Cleaner " << cleanerID << "unaval: " << unavailableRooms.size() << " aval: " << availableRooms.size() << " occ: " << occupiedRooms.size() << "\n";
                    pthread_cond_broadcast(&cv_unaval);
                    break;
                }
                */

            } else {

                cout << "Cleaner " << cleanerID << " [LOG] unaval is empty" << endl;


                if (pthread_mutex_lock(&changeOccupiedRoom) != 0)
                {
                    perror("pthread mutex changeOccupiedRoom lock error occured.");
                    exit(0);
                }

                cout << "Cleaner " << cleanerID << " has mutex" << endl;

                if ( occupiedRooms.empty() ) {
                    cout << "BREAKING OUT" << endl;
                    //break;

                    if (pthread_mutex_lock(&changeTotalOccupied) != 0)
                    {
                        perror("pthread mutex changeTotalOccupied lock error occured.");
                        exit(0);
                    }

                    cout << "Cleaner " << cleanerID << " exiting\n";

                    is_cleaning = 0;
                    totalOccupiedSinceLastClean = 0;

                    for (int i = 0; i < y; i ++)
                        pthread_kill(guest_thread[i], SIGUSR2);
                        
                    cout << "Cleaner " << cleanerID << "Signals sent\n";

                    if (pthread_mutex_unlock(&changeTotalOccupied) != 0)
                    {
                        perror("pthread mutex changeTotalOccupied unlock error occured.");
                        exit(0);
                    }

                } else {
                    cout << "Work left to do" << endl;
                }

                if (pthread_mutex_unlock(&changeOccupiedRoom) != 0)
                {
                    perror("pthread mutex changeOccupiedRoom unlock error occured.");
                    exit(0);
                }

            }
        //}

        //cout << "Cleaner " << cleanerID << " out\n";

        /*
        if (pthread_mutex_lock(&changeTotalOccupied) != 0)
        {
            perror("pthread mutex changeTotalOccupied lock error occured.");
            exit(0);
        }

        cout << "Cleaner " << cleanerID << " has mutex\n";

        if (totalOccupiedSinceLastClean == 2 * n)
            totalOccupiedSinceLastClean = 0;

        // using totalOcc.. as a counter
        if (did_clean) {
            totalOccupiedSinceLastClean += did_clean;
            cout << "Cleaner " << cleanerID << " out" << " totalOcc : " << totalOccupiedSinceLastClean << "\n";
        }

        if (totalOccupiedSinceLastClean == n) {
            is_cleaning = 0;
            totalOccupiedSinceLastClean = 0;

            cout << "Cleaner " << cleanerID << "Last one out\n";

            for (int i = 0; i < y; i ++)
                pthread_kill(guest_thread[i], SIGUSR2);
            
            cout << "Cleaner " << cleanerID << "Signals sent\n";
            
        }
        if (!is_cleaning)
            continue;

        if (pthread_mutex_unlock(&changeTotalOccupied) != 0)
        {
            perror("pthread mutex changeTotalOccupied unlock error occured.");
            exit(0);
        }
        */
    }
    return NULL;
}

int main()
{
    // init random seed
    srand(time(NULL));
    is_cleaning = 0;
    
    sigemptyset(&evict_set);
    sigaddset(&evict_set, SIGUSR1);

    sigemptyset(&clean_set);
    sigaddset(&clean_set, SIGUSR2);

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

    // init jmp_bufs
    guest_env = (sigjmp_buf *) malloc ( y * sizeof (sigjmp_buf) );
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
    guest_thread = (pthread_t *)malloc(y * sizeof(pthread_t));
    for (int i = 0; i < y; i++)
    {
        guest_id[i] = i;
        pthread_create(&guest_thread[i], &attr, guest, &guest_id[i]);
    }

    // create cleaning staff thread
    int *cleaning_staff_id = (int *)malloc(x * sizeof(int));
    cleaning_staff = (pthread_t *)malloc(x * sizeof(pthread_t));
    for (int i = 0; i < x; i++)
    {
        cleaning_staff_id[i] = i;
        pthread_create(&cleaning_staff[i], &attr, cleaner, &cleaning_staff_id[i]);
    }

    sigset_t old_set, main_set;
    sigemptyset(&main_set);
    sigaddset(&main_set, SIGUSR1);
    sigaddset(&main_set, SIGUSR2);

    pthread_sigmask(SIG_BLOCK, &main_set, &old_set);

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
