#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "main.h"

using namespace std;
int *guestPriorities;

int get_rand_inrange(int a, int b) {
  return a + rand() % (b - a + 1);  // generate a random number in the range [a, b]
}

void *guest(void *arg) {
    // sleeps for random time first
    int randomSleepTime = get_rand_inrange(10, 20);
    sleep(randomSleepTime);
}


void *cleaner(void *arg) {

}

int main()
{
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
    Room *rooms = (Room *)malloc(n * sizeof(Room));
    for (int i = 0; i < n; i++)
    {
        rooms[i] = {
            .available = true,
            .pastOccupants = 0,
            .currentOccupant = -1,
            .totalTimeLived = 0};
    }

    // randomly assign priority to guests and store it in array
    guestPriorities = (int *)malloc(y * sizeof(int));
    for (int i = 0; i < y; i++)
        guestPriorities[i] = 1 + rand() % y;

    // explicitly creating threads in a joinable state 
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // create guest thread
    int *guest_id = (int *)malloc(y * sizeof(int));
    pthread_t *guest_thread = (pthread_t *)malloc(y * sizeof(pthread_t));
    for (int i = 0; i < 10; i++)
    {
        guest_id[i] = i;
        pthread_create(&guest_thread[i], &attr, guest, &guest_id[i]);
    }

    // create clealing staff thread
    int *cleaning_staff_id = (int *)malloc(x * sizeof(int));
    pthread_t *cleaning_staff = (pthread_t *)malloc(x * sizeof(pthread_t));
    for (int i = 0; i < 10; i++)
    {
        cleaning_staff_id[i] = i;
        pthread_create(&cleaning_staff[i], &attr, cleaner, &cleaning_staff_id[i]);
    }

    return 0;
}