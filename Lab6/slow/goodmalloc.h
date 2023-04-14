#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

enum ERR {
    ENOMEM = -15
    , EINSUF
    , ELIBMEM
    , ENOLST
};

#define LOGFILE "memfootprint.csv"
#define SUCCESS 0
#define RANDLIM 100001
#define LOGCTL 1

typedef struct node {
    struct node * lnode;
    struct node * rnode;
    int value;
} Node;

typedef struct list {
    Node * root;
    size_t nelems;
} List;

void createMem(size_t);

int createList(List*, size_t);

void initFunc();

int freeElem(List *);

int assignVal(List *, unsigned int, int);