#include<stdio.h>

//size of the queue
#define size 100

// intialize front and rear as 0 for initial queue
int queue_arr[size];
int front  = 0;
int rear   = 0;

int isQueueEmpty();
int isQueueFull();
int dequeue();
void enqueue(int);