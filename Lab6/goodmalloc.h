#define NAME_MAXLEN 200

enum ERR {
    ESYSMEM = -15
    , EMEMEXIST
    , EMEMINIT
    , ENOMEM
    , ENOLIST
    , EOUTOFRANGE
    , ENAMEUSE
};

typedef struct node {
    struct node * lnode;
    struct node * rnode;
    int value;
} Node;

typedef struct list {
    char name[NAME_MAXLEN];
    Node * root;
    size_t nelems;
} List;

/**
* creates a memory of size msize using malloc call, and inits
* the memory management structures
* 
* @param msize [in] size of the memory to init
* @return errorcode if error, or 0 on success
* 
*/

int createMem(size_t msize);

/**
* creates a list of nlist elements, element has 2 pointres and
* one integer value
* 
* @param nelems [in] takes the number of elements to be allocate
* @param lname [in] adds the linked list in the name given
* @param mem [in] the memory segment to add to
* @return returns the list
*/

List* createList(size_t nelems, char *lname);

/**
* assigns value to a given element in the list
* 
* @param lname [in] list name
* @param offset [in] offset to assign to
* @param value [in] the value to be assigned
* @return error code on error, 0 on success
*/

int assignVal(char *lname, unsigned int offset, int value);

/**
* frees a list if given the name, if not, then frees all the lists
* not in use at the moment (if input is null)
* 
* @param lname [in] list name or Null
*/

int freeElem(char *lname);