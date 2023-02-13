#ifndef VECTOR_H
#define VECTOR_H
 
#define VECTOR_INIT_CAPACITY 4
#define TRUE 1
#define FALSE 0


#define VECTOR_INIT(vec) vector vec; vector_init(&vec)
#define VECTOR_ADD(vec, item) vector_add(&vec, (char *) item)
#define VECTOR_SET(vec, id, item) vector_set(&vec, id, (char *) item)
#define VECTOR_GET(vec, type, id) (type) vector_get(&vec, id)
#define VECTOR_DELETE(vec, id) vector_delete(&vec, id)
#define VECTOR_TOTAL(vec) vector_total(&vec)
#define VECTOR_FREE(vec) vector_free(&vec)
 
typedef struct vector {
    char **items;
    int capacity;
    int total;
} vector;
 
void vector_init(vector *);
int vector_total(vector *);
static void vector_resize(vector *, int);
void vector_add(vector *, char *);
void vector_set(vector *, int, char *);
char *vector_get(vector *, int);
void vector_delete(vector *, int);
void vector_free(vector *);
 
#endif