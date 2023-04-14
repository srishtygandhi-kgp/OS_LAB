#include "goodmalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define NLTABLE 1
#define NAME_INIT 10
#define NAAME_THRESH 0.8
#define NAME_RATIO 2

#define CHCKMEM(MEM)                            \
    do {                                        \
        if (MEM == NULL) {                      \
            printf("[ERR] memory not init\n");  \
            return EMEMINIT;                    \
        }                                       \
    } while (0)

#define CHCKMALLOC(MEM)                         \
    do {                                        \
        if (MEM == NULL) {                      \
            printf("[ERR] malloc failed\n");    \
            return ESYSMEM;                     \
        }                                       \
    } while (0)

typedef struct segment {
    unsigned int start;
    size_t segsize;
    struct segment *lchld;
    struct segment *rchld;
    struct segment *parent;
} Segment;

typedef struct ltable {
    char **lnames;
    unsigned int nlist;         /*!< the number of lists init*/
    unsigned int depth;
} LTable;

typedef struct mem {
    Segment * root;              /*!< tracks the free segments*/
    void * addr;
    size_t memsize;
    LTable * ltable;
    size_t depth;                /*!< currenr */
} Mem;

/*****Global Vars*****/

static Mem *progmem = (Mem *) NULL;                 /*!< System mem before allocating*/
static int depth = 0;

int createMem(size_t memsize) {

    if (progmem != NULL) {
        printf("[ERR] Mem already initialized");
        return EMEMEXIST;
    }

    // init depth
    depth = 1;

    // allocating the memory using malloc
    Mem * mem = (Mem *) malloc (sizeof(Mem));
    CHCKMALLOC(mem);
    
    (mem -> memsize) = memsize;
    
    LTable *lt = (mem -> ltable) = (LTable *) malloc (sizeof(LTable));
    CHCKMALLOC(mem -> ltable);

    (lt[0].lnames) = NULL;
    (lt[0].nlist) = 0;
    (lt[0].depth) = 0;

    (mem -> depth) = depth;

    (mem -> addr) = malloc (memsize);
    CHCKMALLOC(mem -> addr);
    
    (mem -> root) = (Segment *) malloc (sizeof(Segment));
    CHCKMALLOC(mem -> root);
    
    (mem -> root) -> start = (mem -> addr); (mem -> root) -> segsize = memsize;
    (mem -> root) -> lchld = NULL; (mem -> root) -> rchld = NULL; (mem -> root) -> parent = NULL;

    // assigning to progmem
    progmem = mem;

    return 0;
}

void initFunc() {
    int idx = (progmem -> depth) ++;
    
    LTable *lt = (progmem -> ltable) = (LTable *) reallocarray( (progmem -> ltable), (progmem -> depth), sizeof(LTable) );
    CHCKMALLOC(lt);

    (lt[idx].lnames) = NULL;
    (lt[idx].nlist) = 0;
    (lt[idx].depth) = idx + 1;
}

static int findFit (size_t size, Segment *root) {
    
    if (size > (root -> segsize) )
        return (root -> start);
    else {
        if ( (root -> rchld) == NULL )
            return ENOMEM;
        else
            return findFit (size, (root -> rchld) );
    }
}

// static Node *findNode (size_t size, unsigned int start, Node *root) {

//     if ( root == NULL )
//         return NULL;
//     else if ( size == (root -> size) && start == (root -> start) )
//         return root;
//     else if ( size > (root -> size) )
//         return findFit (size, (root -> lchld) );
//     else 
//         return findFit (size, (root -> rchld) );
// }

// static void transplant(Segment *u, Segment *v) {

//     if (u -> parent) == NULL
//         (progmem -> root) = v;
//     else ( ( (u -> parent) -> lchld ) == u )
//         ( (u -> parent) -> lchld ) = v;
//     else
//         ( (u -> parent) -> rchld ) = v;
    
//     if (v != NULL)
//         (v -> parent) = (u -> parent);
// }

// static void removeNode(Segment *node) {

//     if ( (node -> lchld) == NULL )
//         transplant(node, (node -> rchld));
//     else if ( (node -> rchld) == NULL )
//         transplant(node, (node -> lchld));
//     else {
//         // TODO
//     }
// }

static Segment *minNode(Segment *node) {
    Segment *curr = node;

    while (curr -> lchld)
        curr = (curr -> lchld);
    
    return curr;
}

static Segment * deleteNode (Segment * root, size_t size, unsigned int start) {

    if ( root == NULL )
        return root;

    if ( size < (root -> segsize) )
        (root -> lchld) = deleteNode ((root -> lchld), start, size);
    else if ( size > (root -> segsize) )
        (root -> rchld) = deleteNode ((root -> rchld), size, start);
    else {

        Segment *tmp;

        if ( (root -> lchld) == NULL ) {
            tmp = (root -> rchld);
            (progmem -> root) = NULL;
            return tmp;
        } else if ( (root -> rchld) == NULL ) {
            tmp = (root -> lchld);
            (progmem -> root) = NULL;
            return tmp;
        }

        tmp = minNode(root -> rchld);
        (root -> segsize) = (tmp -> segsize);
        (root -> start) = (tmp -> start);

        (root -> rchld) = deleteNode((root -> rchld), (tmp -> segsize), (tmp -> start));
    }
}

static void insertNode (unsigned int start, size_t size, Segment *root) {

    Segment *troot = root;
    Segment *it = NULL;

    while (troot != NULL) {

        it = troot;
        if (size < (troot -> segsize) )
            troot = (troot -> lchld);
        else
            troot = (troot -> rchld);
    }

    Segment *inseg = (Segment *) malloc (sizeof(Segment));
    
    (inseg -> start) = start;
    (inseg -> segsize) = size;
    (inseg -> rchld) = NULL;
    (inseg -> lchld) = NULL;
    (inseg -> parent) = it;

    if ( it == NULL )
        root = inseg;
    else if ( (inseg -> segsize) < (it -> segsize) )
        (it -> lchld) = inseg;
    else
        (it -> rchld) = inseg;
}

static void * allocChunk (size_t size) {

    CHCKMEM(progmem);

    Segment *root = (progmem -> root);
    int offset = findFit(size, root);

    if ( offset == ENOMEM )
        return NULL;
}

List *createList (size_t nelems, char *lname) {

    CHCKMEM(progmem);
}