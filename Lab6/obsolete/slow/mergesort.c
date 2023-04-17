#include "goodmalloc.h"
#include <stdio.h>

#define MEMSIZE 250
#define MB (1024 * 1024)
#define TOTAL_SIZE (MEMSIZE * MB)
#define ui unsigned int
#define NLST 50000

void printList(List *lst) {
    Node *root = (lst -> root);

    printf("[DEBUG] ");

    while (root) {

        printf("%d ", (root -> value));
        root = (root -> rnode);
    }
    printf("\n");
}

void merge(List *lst, ui begin, ui mid, ui end) {
    initFunc();
    
    // printf("[LOG][STC1]");
    // printList(lst);
    // printf("[LOG] merge(%u, %u, %u)\n", begin, mid, end);
    // if (lst->root)
    //     printf("[2nd elem] %ld\n", (lst->root)->rnode );

    ui fsize = (mid - begin), sesize = (end - mid);

    List flst, slst;
    createList(&flst, fsize);
    // printList(&flst);

    // printf("[LOG][STC2]");
    // printList(lst);
    createList(&slst, sesize);
    // printList(&slst);

    // printf("[LOG][STC3]");
    // printList(lst);
    // printf("[LOG] lists created(%u, %u, %u)\n", begin, mid, end);

    // assigning values to flst, and slst
    // ui idx = begin;
    Node *troot = (lst -> root);
    for (ui i = 0; i < begin; i ++) {
        // printf("[DEBUG]{LOG} %u|%d\n", i, (troot -> value));
        troot = (troot -> rnode);
    }

    // printf("[LOG] starting assignment from %d\n", (troot -> value));

    Node *frt = (flst.root), *srt = (slst.root);
    for (ui idx = 0; idx < (mid - begin); idx ++) {
        // extracting the value from lst
        // storing in flst
        // printf("[LOG] value to be assigned: %d\n", (troot -> value));
        // assignVal(&flst, idx, (troot -> value));
        (frt -> value) = (troot ->value);
        frt = (frt -> rnode);
        troot = (troot -> rnode);
    }
    for (ui idx = 0; idx < (end - mid); idx ++) {
        // extracting the value from lst
        // storing in slst
        // printf("[LOG] value to be assigned: %d\n", (troot -> value));
        // assignVal(&slst, idx, (troot -> value));
        (srt -> value) = (troot ->value);
        srt = (srt -> rnode);
        troot = (troot -> rnode);
    }

    // printf("[LOG] values assigned (%u, %u, %u)\n", begin, mid, end);
    // printf("[LOG] flst ");
    // printList(&flst);
    // printf("[LOG] slst ");
    // printList(&slst);

    // merging 'em into lst
    ui cnt = begin;
    Node *froot = (flst.root), *sroot = (slst.root);
    troot = (lst -> root);
    for (ui i = 0; i < begin; i ++) {
        // printf("[DEBUG]{LOG} %u|%d\n", i, (troot -> value));
        troot = (troot -> rnode);
    }
    for (; cnt < end; cnt ++) {
        if ( (froot == NULL) && (sroot == NULL) )
            break;

        // if (froot ==  NULL) printf("[YATIII] %u\n", cnt);        
        if (froot == NULL) {
            // int sval = (sroot -> value);
            // assignVal(lst, cnt, sval);
            (troot -> value) = (sroot -> value);
            troot = (troot -> rnode);
            sroot = (sroot -> rnode);

            // printf("[LOG] %d, <sroot assigned to> %u\n", sval, cnt);
            continue;
        } else if (sroot == NULL) {
            // int fval = (froot -> value);
            // assignVal(lst, cnt, fval);
            (troot -> value) = (froot -> value);
            troot = (troot -> rnode);
            froot = (froot -> rnode);

            // printf("[LOG] %d, <froot assigned to> %u\n", fval, cnt);
            continue;
        }

        int fval = (froot -> value);
        int sval = (sroot -> value);
        
        if (fval < sval) {
            // assignVal(lst, cnt, fval);
            (troot -> value) = fval;
            troot = (troot -> rnode);
            froot = (froot -> rnode);

            // printf("[LOG] %d, froot assigned to %u\n", fval, cnt);
        } else {
            // assignVal(lst, cnt, sval);
            (troot -> value) = sval;
            troot = (troot -> rnode);
            sroot = (sroot -> rnode);

            // printf("[LOG] %d, sroot assigned to %u\n", sval, cnt);
        }
    }

    // printList(lst);
    freeElem(NULL);
    return;
}

void merge_sort(List *lst, ui begin, ui end) {

    // printf("[LOG] mergesort(%u, %u)\n", begin, end);
    if (begin >= (end - 1))
        return;

    initFunc();
    
    ui mid = (begin + end) / 2;

    merge_sort(lst, begin, mid);
    merge_sort(lst, mid, end);

    merge(lst, begin, mid, end);

    freeElem(NULL);
    return;
}

int main () {

    createMem(TOTAL_SIZE);
    
    List lst;
    createList(&lst, NLST);
    printList(&lst);

    // printf("[LOG] list craeted\n");

    merge_sort(&lst, 0, NLST);

    printList(&lst);

    freeElem(&lst);
    
    return 0;
}