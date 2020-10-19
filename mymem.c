#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct memoryList
{
    // doubly-linked list
    struct memoryList *prev;
    struct memoryList *next;

    int placement;
    void *previousAllocated; //the one allocated before this one
    int size;            // How many bytes in this block?
    char alloc;          // 1 if this block is allocated,
    // 0 if this block is free.
    void *ptr;           // location of block in memory pool.
};

strategies myStrategy = NotSet;    // Current strategy


size_t mySize;
void *myMemory = NULL;

static struct memoryList *head;
static struct memoryList *next;
static struct memoryList *latest; // i use latest to point to the last one that was allocated that is not freed


/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

void initmem(strategies strategy, size_t sz)
{
    myStrategy = strategy;

    /* all implementations will need an actual block of memory to use */
    mySize = sz;

    if (myMemory != NULL) free(myMemory); /* in case this is not the first time initmem2 is called */

    /* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */

    // here i free all the stuff in memory that was still active
    if (head != NULL) {
        struct memoryList *trav;
        for (trav = head->next; trav->next != NULL; trav = trav->next) {
            free(trav->prev);
        }
        free(trav);
    }
    myMemory = malloc(sz);

    /* TODO: Initialize memory management structure. */

    // reinitialize my structs
    head = (struct memoryList*) malloc(sizeof (struct memoryList));
    head->prev = NULL;
    head->next = NULL;
    head->placement = -1; // not in memory
    head->size = sz; // initialy the first block size is equals to the memory pool size.
    head->alloc = 0;  // not allocated
    head->ptr = myMemory;  // points to the same memory adress as the memory pool

    latest = (struct memoryList*) malloc(sizeof(struct memoryList));
    latest->prev = NULL;
    latest->next = NULL;
    latest->placement = -1; // not in memory
    latest->size = NULL; // initialy the first block size is equals to the memory pool size.
    latest->alloc = 0;  // not allocated
    latest->ptr = NULL;
}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1
 */

void *mymalloc(size_t requested)
{
    // i first check if there is enough space in the whole memory (without considering used space), if not return NULL
    if (requested > mySize){return NULL;}
    assert((int)myStrategy > 0);

    switch (myStrategy)
    {
        case NotSet:
            return NULL;
        case First:
            return NULL;
        case Best:
            return NULL;
        case Worst:
            return NULL;
        case Next: {
            //first i check if there is a big enough block of free space, if not return NULL
            if (mem_largest_free() < requested){return NULL;}

            //i create a struct that i will use as a traversal pointer. It points to head
            struct memoryList *trav;
            trav=head;

            // first check if there is max amount of space available (first thing to be allocated)
            if (mySize == trav->size){
                // create a struct and call it temp and set given parameters
                struct memoryList *temp = (struct memoryList*) malloc(sizeof (struct memoryList));
                temp->size=requested;
                temp->prev=head;
                temp->placement=0;
                temp->alloc=1;
                temp->next=NULL;
                temp->ptr=temp;
                temp->previousAllocated = NULL; // here i set the previous to NULL since there is nothing before it
                mySize=mySize-requested; //reduce the available size in my memory
                head->next=temp; //change connectivity between structs
                latest=temp; //set the latest struct allocated to temp
                return latest->ptr; //here i return the pointer to the latest created struct (temp)
            }
            else{
                // if something has already been initialized in memory i set the current traversal path to my latest struct (since i have to find the next fit)
                trav = latest;
                int startPlacement = trav->placement+trav->size; // i will use this as a break from an infinite loop COME BACK TO LATER
                int thisPlacement = trav->placement+trav->size; //i will use this to make some calculations (this is the point in the memory where latest is located at)
                if (thisPlacement+requested <= head->size && trav->next == NULL) { // if there is still space available in the memory (before i have to loop) and nothing is to the right
                    // lav en struct og giv den de rigtige parametre
                    struct memoryList *temp = (struct memoryList *) malloc(sizeof(struct memoryList));
                    temp->size = requested;
                    temp->prev = trav;
                    temp->placement = thisPlacement;
                    temp->alloc = 1;
                    temp->next = NULL;
                    temp->previousAllocated=latest;
                    temp->ptr = temp;
                    mySize = mySize - requested;
                    trav->next = temp;
                    latest = temp;
                    return latest->ptr;
                }
                else {
                    // check up until the end of the memory
                    while (trav->next != NULL){
                        if (trav->next->placement - (trav->placement+trav->size) > requested){ // if there is space for the requested memory segment before the next memory locatio
                            // create struct and give it its parameters
                            struct memoryList *temp = (struct memoryList *) malloc(sizeof(struct memoryList));
                            temp->size = requested;
                            temp->prev = trav->prev;
                            temp->placement = trav->placement+trav->size;
                            temp->alloc = 1;
                            temp->next = trav->next;
                            temp->ptr = temp;
                            temp->next->prev = temp;
                            temp->prev->next=temp;
                            mySize = mySize - requested;
                            temp->previousAllocated=latest;
                            latest = temp;
                            return latest->ptr;
                        }
                        // loop every point after latest
                        trav = trav->next;
                    }



                    // if there is not enough space at the end of the memory, then it means it is before latest
                    // set current path to first allocated memory.
                    trav = head->next;
                    // if there is space before that allocated memory then we insert the new memory before
                    if (trav->placement >= requested){
                        struct memoryList *temp = (struct memoryList *) malloc(sizeof(struct memoryList));
                        temp->size = requested;
                        temp->prev = trav->prev;
                        temp->placement = 0;
                        temp->alloc = 1;
                        temp->next = trav;
                        temp->ptr = temp;
                        temp->next->prev = temp;
                        temp->prev->next=temp;
                        mySize = mySize - requested;
                        temp->previousAllocated=latest;
                        latest = temp;
                        return latest->ptr;
                    }

                    // make a counter for how much space there is at the current segment
                    int thisSpace;

                    // while you are not at the same position as start position (looped the whole way around)
                    while (trav->placement + trav->size != startPlacement) {
                        // current space is from current position + size until next current position
                        thisSpace = trav->next->placement - (trav->placement + trav->size);
                        // if there is enough space then we put it in here
                        if (thisSpace >= requested) {
                            struct memoryList *temp = (struct memoryList *) malloc(sizeof(struct memoryList));
                            temp->size = requested;
                            temp->prev = trav;
                            temp->placement = trav->placement + trav->size;
                            temp->alloc = 1;
                            temp->next = trav->next;
                            temp->ptr = temp;
                            trav->next->prev = temp;
                            trav->next = temp;
                            mySize = mySize - requested;
                            latest = temp;
                            temp->previousAllocated=trav;
                            return latest->ptr;
                        }
                        trav = trav->next;
                    }
                }
            }
            // return NULL if no place has been found
            return NULL;
        }
    }
    // return NULL if no place has been found
    return NULL;
}


/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block)
{
    // make a struct for traversing the memory and set it to first place in memory
    struct memoryList *trav;
    trav=head->next;
    while(trav != NULL) { // check if you hit the end of the memory
        if (trav->ptr == block) { // check if you found the correct pointer
            trav->prev->next = trav->next; // connect the previous and next memory to eachother
            if (trav->next != NULL){trav->next->prev = trav->prev;}
            mySize = mySize+trav->size; // you got some extra space now so we need to declare that
            if (trav == latest){ // if you used free on the latest you allocated then set the current latest to the previous allocted one
                latest = trav->previousAllocated;
            }
            free(block); // free the block from the heap
            break; //if the pointer was found then break the function
        }
        trav = trav->next; //if not the correct pointer check next one
    }
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when refered to "memory" here, it is meant that the
 * memory pool this module manages via initmem/mymalloc/myfree.
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes()
{
    struct memoryList *trav;
    trav=head->next;
    if(trav != NULL) {
        int holes = 0;
        if (trav->placement != 0){holes++;}
        while (trav->next != NULL) {
            if (trav->placement + trav->size != trav->next->placement) {
                holes++;
            }
            trav = trav->next;
        }
        if (trav->placement+trav->size < head->size){holes++;}
        return holes;
    }
    if (mySize>0) return 1;
    else return 0;
}

/* Get the number of bytes allocated */
int mem_allocated()
{
    return head->size-mySize;
}

/* Number of non-allocated bytes */
int mem_free()
{
    return mySize;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free() {
    struct memoryList *trav;
    trav = head->next;
    int largestHole = 0;
    if (trav != NULL) {
        if (trav->placement != 0) {
            largestHole = trav->placement;
        }
        while (trav->next != NULL) {
            if (trav->placement + trav->size != trav->next->placement) {
                if (trav->next->placement - (trav->placement + trav->size) > largestHole) {
                    largestHole = trav->next->placement - (trav->placement + trav->size);
                }
            }
            trav = trav->next;
        }
        if (trav->placement + trav->size < head->size) {
            if (head->size - (trav->placement + trav->size) > largestHole) {
                largestHole = head->size - (trav->placement + trav->size);
            }
        }

        if (largestHole < head->size - (trav->placement + trav->size)) {
            return head->size - (trav->placement + trav->size);
        }
        return largestHole;
    } else return head->size;
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
    struct memoryList *trav;
    trav=head->next;
    if(trav != NULL) {
        int holes = 0;
        while (trav->next != NULL) {
            if (trav->placement + trav->size != trav->next->placement && trav->next->placement - (trav->placement+trav->size) > size) {
                holes++;
            }
            trav = trav->next;
        }
        if (trav->placement+trav->size < head->size){holes++;}
        return holes;
    }
    if (mySize>0) return 1;
    else return 0;
}

char mem_is_alloc(void *ptr)
{
    struct memoryList *trav;
    trav=head->next;
    while(trav != NULL) {
        if (trav->ptr == ptr) {
            return 'y';
        }
        trav = trav->next;
    }
    return 'n';
}

/*
 * Feel free to use these functions, but do not modify them.
 * The test code uses them, but you may find them useful.
 */


//Returns a pointer to the memory pool.
void *mem_pool()
{
    return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
    return head->size;
}


// Get string name for a strategy.
char *strategy_name(strategies strategy)
{
    switch (strategy)
    {
        case Best:
            return "best";
        case Worst:
            return "worst";
        case First:
            return "first";
        case Next:
            return "next";
        default:
            return "unknown";
    }
}

// Get strategy from name.
strategies strategyFromString(char * strategy)
{
    if (!strcmp(strategy,"best"))
    {
        return Best;
    }
    else if (!strcmp(strategy,"worst"))
    {
        return Worst;
    }
    else if (!strcmp(strategy,"first"))
    {
        return First;
    }
    else if (!strcmp(strategy,"next"))
    {
        return Next;
    }
    else
    {
        return 0;
    }
}


/*
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
    struct memoryList *trav = head->next;
    while (trav != NULL) {
        printf("%d\t%d\t%d\n", trav->placement, trav->size, trav->alloc);
        trav = trav->next;
    }
}

/* Use this function to track memory allocation performance.
 * This function does not depend on your implementation,
 * but on the functions you wrote above.
 */
void print_memory_status()
{
    printf("%d out of %d bytes allocated.\n",mem_allocated(),mem_total());
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("Average hole size is %f.\n\n",((float)mem_free())/mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
    strategies strat;
    void *a, *b, *c, *d, *e;
    if(argc > 1)
        strat = strategyFromString(argv[1]);
    else
        strat = Next;


    /* A simple example.
       Each algorithm should produce a different layout. */

    initmem(strat,500);

    a = mymalloc(100);
    printf("a allocated\n");
    print_memory();
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("\n");

    b = mymalloc(150);
    printf("b allocated\n");
    print_memory();
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("\n");

    c = mymalloc(100);
    printf("c allocated\n");
    print_memory();
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("\n");

    myfree(c);
    printf("c freed\n");
    print_memory();
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("\n");

    d = mymalloc(50);
    printf("d allocated\n");
    print_memory();
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("\n");

    myfree(a);
    printf("a freed\n");
    print_memory();
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("\n");

    e = mymalloc(150);
    printf("e allocated\n");
    print_memory();
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("\n");

    void* f = mymalloc(100);
    printf("f allocated\n");
    print_memory();
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("\n");

    myfree(b);
    myfree(d);
    printf("b and d freed\n");
    print_memory();
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("\n");


    void* g = mymalloc(200);
    printf("g allocated\n");
    print_memory();
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
    printf("\n");


    //print_memory();
    //print_memory_status();

}

int main(){
    try_mymem(NULL,NULL);
}