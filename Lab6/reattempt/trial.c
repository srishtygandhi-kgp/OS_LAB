#include "goodmalloc.h"
#include <stdio.h>

void printList(List *lst) {
    Node *root = (lst -> root);

    while (root -> rnode) {

        printf("%d ", (root -> value));
        root = (root -> rnode);
    }
    printf("\n");
}

int main () {
    createMem(1024 * 1024 * 1024);
    
    List lst;
    createList(&lst, 10);
    printList(&lst);
    assignVal(&lst, 3, 69);
    printList(&lst);

    List lst2;

    for (int i = 0; i < 5; i ++) {
        createList(&lst2, 10);
        printList(&lst2);
        assignVal(&lst2, 3, 69);
        printList(&lst2);
        //freeElem(&lst2);
        assignVal(&lst2, 4, 669);
        printList(&lst2);
        freeElem(&lst2);
    }

    List lst3;
    createList(&lst3, 10);
    printList(&lst3);
    assignVal(&lst3, 3, 69);
    printList(&lst3);
    //freeElem(&lst2);
    assignVal(&lst3, 4, 669);
    printList(&lst3);
    freeElem(&lst3);

    List lst4;
    createList(&lst4, 10);
    printList(&lst4);
    assignVal(&lst4, 3, 69);
    printList(&lst4);
    //freeElem(&lst2);
    assignVal(&lst4, 4, 669);
    printList(&lst4);
    freeElem(&lst4);
}