#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "goodmalloc.h"

#define MAXLEN (1024 * 1024)

typedef struct seg {
    size_t size;
    void *addr;
} segment;

typedef struct symtable {
    List *lst;
    unsigned int depth;
} sentry;

// Data-structures for memory
segment freeseg[MAXLEN];
sentry stable[MAXLEN];

unsigned int depth;
unsigned int nfreeseg;
unsigned int nlist;

void createMem(size_t msize) {

    if (nfreeseg != 0) {
        printf("[ERROR] memory already initialized, exiting\n");
        exit(EXIT_FAILURE);
    }

    memset(freeseg, -1, MAXLEN * sizeof (segment));

    // first element set
    freeseg[0].addr = malloc(msize);
    freeseg[0].size = msize;

    // updated the index of the array
    nfreeseg = 1;
    depth = 1;
    nlist = 0;
}

// void _print_symtab() {

//     printf("[SYMT]\n");
//     for (unsigned int i = 0; i < nlist; i ++)
//         printf("\t[LIST%u] depth:%u|addr:%ld|endaddr:%ld|size:%ld\n", i, stable[i].depth, (stable[i].lst -> root), (stable[i].lst -> root) + ((stable[i].lst -> nelems)), ((stable[i].lst -> nelems) * sizeof(Node)) );
// }

void _populate_list(List *lst, size_t nelems, void *addr) {

    void *bkaddr = addr;
    Node *lnode = NULL, *rnode = (Node *) ( addr );

    for (size_t i = 0; i < nelems; i ++) {

        // printf("[DEBUG][ITER] idx: %ld, lnode: %p, rnode: %p\n", i, lnode, rnode);

        Node *nd = rnode;
        rnode = (Node *) ( nd + 1 );

        Node tmp;
        tmp.lnode = lnode; tmp.rnode = rnode; tmp.value = rand() % RANDLIM;

        if (i == (nelems - 1)) {
            tmp.rnode = NULL;
        }
        memcpy((void *)nd, &tmp, sizeof(Node));
        // if (i == (nelems - 1)) rnode = NULL;

        // printf("[DEBUG][ITER] addr: %p\n", nd);

        // (nd -> lnode) = lnode;

        // printf("[DEBUG][ITER] lnode done\n");
        // (nd -> rnode) = rnode;

        // printf("[DEBUG][ITER] rnode done\n");
        // (nd -> value) = rand() % RANDLIM;

        // printf("[DEBUG][ITER] curr-node done\n");

        // iterating to next node
        lnode = nd;

        // printf("[DEBUG][ITER] done\n");
    }
    
    (lst -> root) = (Node *) bkaddr;
    (lst -> nelems) = nelems;
}

// void _print_freeseg() {

//     printf("[SEGMENT]\n");
//     for (int i = 0; i < nfreeseg; i ++) {
//         printf("\t[SEG%d] addr: %ld|endaddr: %ld| size: %ld\n", i, freeseg[i].addr, (freeseg[i].addr + freeseg[i].size), freeseg[i].size );
//     }
// }

int createList(List *lst, size_t nelems) {

    // checking if there are no free segments
    if (nfreeseg == 0)
        return ENOMEM;

    // printf("[DEBUG] start\n");

    // local variables initializing
    int idx = 0;
    size_t size = nelems * sizeof (Node), segsize = 0;

    // finding the first fit segment
    for (; idx < nfreeseg; idx ++) {

        // checking if the current segment
        // is up to the mark
        if (freeseg[idx].size < size) continue;

        // if the segment is actually useful
        segsize = freeseg[idx].size;
        break;
    }

    // checking if no segment of size
    // was found
    if (segsize == 0) {
        printf("[ERR] insufficient mem\n");
        return EINSUF;
    }

    // printf("[DEBUG] segsize found\n");

    // assigning to lst
    _populate_list(lst, nelems, freeseg[idx].addr);

    // printf("[DEBUG] lst nelems %ld, adding to stable\n", nelems);
    // appending to symboltable
    stable[nlist].lst = lst;
    stable[nlist].depth = depth;

    nlist ++;
    // _print_symtab();

    // modifying or removing the current
    // segment we're taking mem from
    if (segsize > size) {

        // printf("[DEBUG] init %ld|taken %ld\n", segsize, size);
        freeseg[idx].size = (segsize - size);
        freeseg[idx].addr = (freeseg[idx].addr + size);
        // _print_freeseg();
        return SUCCESS;
    }

    // if the free element got empty
    for (int i = idx; i < (nfreeseg - 1); i ++)
        freeseg[i] = freeseg[i + 1];
    
    nfreeseg --;

    // _print_freeseg();
    return SUCCESS;
}

void _housekeep() {

    // printf("[DEBUG] housekeep\n");
    // checking the current depth
    // and deleting the lists with
    // that depth

    unsigned int tmpdepth = stable[nlist - 1].depth;
    // printf("[DEBUG] nlist:%u|tmp:%u|depth:%u\n", nlist, tmpdepth, depth);

    while (tmpdepth == depth) {

        // basically freeing a list
        // using freeElem
        freeElem(stable[nlist - 1].lst);
        tmpdepth = stable[nlist - 1].depth;
        // printf("\t[DEBUG] tmp: %d, depth: %d\n", tmpdepth, depth);
    }

    depth --;
}

int freeElem(List *lst) {

    if (lst == NULL) {
        _housekeep();
        return SUCCESS;
    }

    // adding a free segment to the
    // global array
    if (nfreeseg == MAXLEN) {
        printf("[ERR] system memory out\n");
        return ELIBMEM;
    }

    int idx; int flag = 0;
    for (idx = 0; idx < (int) nlist; idx ++) {
        if (stable[idx].lst == lst) {
            flag = 1;
            break;
        }
    }

    if (flag == 0) {
        printf("[ERR] list not present in symboltable\n");
        return ENOLST;
    }

    for (int i = idx; i < (nlist - 1); i ++)
        stable[idx] = stable[idx + 1];
    
    nlist --;

    freeseg[nfreeseg].addr = (lst -> root);
    freeseg[nfreeseg].size = (lst -> nelems) * sizeof(Node);

    nfreeseg ++;

    return SUCCESS;
}

void initFunc() {
    depth ++;
}

int assignVal(List *lst, unsigned int offset , int val) {

    int flag = 0;

    for (unsigned int idx = 0; idx < nlist; idx ++) {
        if (stable[idx].lst == lst) {
            flag = 1;
            break;
        }
    }

    if (flag == 0) {
        printf("[ERR] list not present in symboltable\n");
        return ENOLST;
    }

    Node *tmpnode = (lst -> root);
    for (unsigned int i = 0; i < offset; i ++)
        tmpnode = (tmpnode -> rnode);
    
    (tmpnode -> value) = val;

    return SUCCESS;
}