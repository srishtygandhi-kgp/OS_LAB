#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdio.h>
#include "macros.h"

//size of the queue
#define size ROWS

int isQueueEmpty();
int isQueueFull();
int dequeue();
void enqueue(int);

#endif