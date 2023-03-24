#include <iostream>
#include "main.h"

using namespace std;

int totalOccupiedSinceLastClean;
int *guestPriorities;

pthread_mutex_t changeTotalOccupied, changeOccupiedRoom;

sem_t roomSemaphore;

Room* allRooms;
stack<int> availableRooms, unavailableRooms;
set<int> occupiedRooms;

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
