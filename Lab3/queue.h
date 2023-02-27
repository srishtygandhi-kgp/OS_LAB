#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdio.h>

//size of the queue
#define size 100

int isQueueEmpty();
int isQueueFull();
int dequeue();
void enqueue(int);

#endif