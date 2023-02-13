#include <stdio.h>
#include <stdlib.h>
 
#include "utils.h"
 
void vector_init(vector *v) {
    v->capacity = VECTOR_INIT_CAPACITY;
    v->total = 0;
    v->items = (char **)malloc(sizeof(char *) * v->capacity);
}

 
int vector_total(vector *v) {
    return v->total;
}
 
static void vector_resize(vector *v, int capacity) {
#ifdef DEBUG_ON
    printf("vector_resize: %d to %d\n", v->capacity, capacity);
#endif
 
    char **items = (char **)realloc(v->items, sizeof(char *) * capacity);
    if (items) {
        v->items = items;
        v->capacity = capacity;
    }
}
 
void vector_add(vector *v, char *item) {
    if (v->capacity == v->total)
        vector_resize(v, v->capacity * 2);
    v->items[v->total++] = item;
}
 
void vector_set(vector *v, int index, char *item) {
    if (index >= 0 && index < v->total)
        v->items[index] = item;
}
 
char *vector_get(vector *v, int index) {
    if (index >= 0 && index < v->total)
        return v->items[index];
    return NULL;
}
 
void vector_delete(vector *v, int index) {
    if (index < 0 || index >= v->total)
        return;
 
    v->items[index] = NULL;
    int i;
    for (i = 0; i < v->total - 1; i++) {
        v->items[i] = v->items[i + 1];
        v->items[i + 1] = NULL;
    }
 
    v->total--;
 
    if (v->total > 0 && v->total == v->capacity / 4)
        vector_resize(v, v->capacity / 2);
}
 
void vector_free(vector *v) {
    free(v->items);
}
 
// int main(void) {
//     int i;
 
//     vector v;
//     vector_init(&v);
 
//     vector_add(&v, "Bonjour");
//     vector_add(&v, "tout");
//     vector_add(&v, "le");
//     vector_add(&v, "monde");

 
//     for (i = 0; i < vector_total(&v); i++)
//         printf("%s ", (char *) vector_get(&v, i));
//     printf("\n");
 
//     vector_delete(&v, 3);
//     // vector_delete(&v, 2);
//     vector_delete(&v, 1);

//     vector a;
//     vector_init(&a);
//     vector_add(&a,"no");
//     vector_set(&a,0,"hii");
//     vector_set(&a,1,"boo");
//     for (i = 0; i < vector_total(&a); i++)
//         printf("%s ", (char *) vector_get(&a, i));
//     printf("\n");

    
//     vector_delete(&v, 3);
//     vector_delete(&v, 2);
//     vector_delete(&v, 1);

 
//     vector_set(&v, 0, "Hello");
//     vector_add(&v, "World");
 
//     for (i = 0; i < vector_total(&v); i++)
//         printf("%s ", (char *) vector_get(&v, i));
//     printf("\n");
 
//     vector_free(&v);
// }
