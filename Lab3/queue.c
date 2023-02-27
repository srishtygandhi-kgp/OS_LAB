#include "queue.h"

// intialize front and rear as 0 for initial queue
int queue_arr[size];
int front = -1;
int rear = -1;

// return 1 if queue is empty, else return 0
int isQueueEmpty()
{
   return (front == -1) ? 1 : 0;
}

// return 1 if queue is full, else return 0
int isQueueFull()
{
   return ((front == rear + 1) || (front == 0 && rear == size - 1)) ? 1 : 0; 
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
        if(front == rear) {
            front = -1;
            rear = -1;
        }
        else {
            front = (front + 1) % size;
        }
        return element;
    }
}

//add element at rear end
void enqueue(int ele)
{
   if(isQueueFull() == 1) return;
   else
   {   
       for(int i = front; i < rear; i++)
       {
            if(queue_arr[i] == ele)
            {
                return;
            }
       }
       if(isQueueEmpty()) front = 0;
       rear = (rear + 1) % size;
       queue_arr[rear] = ele;
   }
}
