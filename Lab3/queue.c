#include "queue.h"

// return 1 if queue is empty, else return 0
int isQueueEmpty()
{
   return (front == rear) ? 1 : 0;
}

// return 1 if queue is full, else return 0
int isQueueFull()
{
   return (rear == size) ? 1 : 0; 
}

//return the front element and removes it, returns -1 if queue is empty
int dequeue()
{   
    if(isQueueEmpty() == 1) {
        return -1;
    }
    else
    {
        int element = queue_arr[front];
        front++;
        return element;
    }
}

//add element at rear end
void enqueue(int ele)
{
   if(isQueueFull() == 1)
       printf("Queue is Full\n");
   else
   {
       queue_arr[rear] = ele;
       rear++;
   }
}
