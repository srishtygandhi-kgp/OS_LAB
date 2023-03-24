#include <iostream>
#include "main.h"

using namespace std;

void cleaner_start_handler(int signum) {
    char msg[] = "Cleaner starting\n";
    write(1, msg, strlen(msg));
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
