#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct memoryList {
    // doubly-linked list
    struct memoryList *prev;
    struct memoryList *next;

    int placement;       // placement of block on own memory
    void *previousAllocated; //the one allocated before this one
    int size;            // How many bytes in this block?
    char alloc;          // 1 if this block is allocated,
    // 0 if this block is free.
    void *ptr;           // location of block in my own memory pool.
    void *realPointer;   // location of block on heap
};

strategies myStrategy = NotSet;    // Current strategy


size_t mySize;
void *myMemory = NULL;

static struct memoryList *head; //this will be my general memory block
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

void initmem(strategies strategy, size_t sz) {
    myStrategy = strategy;

    /* all implementations will need an actual block of memory to use */
    mySize = sz;

    if (myMemory != NULL) free(myMemory); /* in case this is not the first time initmem2 is called */

    // here i free all the stuff in memory that was still active
    if (head != NULL) {
        struct memoryList *trav;
        for (trav = head->next; trav->next != NULL; trav = trav->next) {
            free(trav->prev);
        }
        free(trav);
    }
    myMemory = malloc(sz);

    // reinitialize my structs
    head = (struct memoryList*) malloc(sizeof (struct memoryList));
    head->prev = NULL;
    head->next = NULL;
    head->placement = -1; // not in memory
    head->size = sz; // initialy the first block size is equals to the memory pool size.
    head->alloc = 0;  // not allocated
    head->ptr = myMemory;  // points to the same memory adress as the memory pool
    head->previousAllocated = NULL;
    head->realPointer = head;

    latest = (struct memoryList*) malloc(sizeof(struct memoryList));
    latest->prev = NULL;
    latest->next = NULL;
    latest->placement = -1; // not in memory
    latest->size = NULL;
    latest->alloc = 0;  // not allocated
    latest->ptr = NULL;
    latest->previousAllocated = NULL;
    latest->realPointer = NULL;
}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1
 */

void *mymalloc(size_t requested) {
    // i first check if there is enough space in the whole memory (without considering used space), if not return NULL
    if (requested > mySize) { return NULL; }
    //first i check if there is a big enough block of free space, if not return NULL
    if (mem_largest_free() < requested) { return NULL; }
    assert((int) myStrategy > 0);

    switch (myStrategy) {
        case NotSet:
            return NULL;
        case First:
            return NULL;
        case Best:
            return NULL;
        case Worst:
            return NULL;
        case Next: {

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
                temp->ptr=head->ptr+temp->placement;
                temp->realPointer=temp;
                temp->previousAllocated = NULL; // here i set the previous to NULL since there is nothing before it
                mySize=mySize-requested; //reduce the available size in my memory
                head->next=temp; //change connectivity between structs
                latest=temp; //set the latest struct allocated to temp
                return latest->ptr; //here i return the pointer to the latest created struct (temp)
            }
            else{
                // if something has already been initialized in memory i set the current traversal path to my latest struct (since i have to find the next fit)
                trav = latest;
                int startPlacement = trav->placement+trav->size; // i will use this as a break from an infinite loop
                int thisPlacement = trav->placement+trav->size; //i will use this to make some calculations (this is the point in the memory where latest is located at)
                if (thisPlacement+requested <= head->size && trav->next == NULL) { // if there is still space available in the memory (before i have to loop) and nothing is to the right
                    // create a struct and give its parameters
                    struct memoryList *temp = (struct memoryList *) malloc(sizeof(struct memoryList));
                    temp->size = requested;
                    temp->prev = trav;
                    temp->placement = thisPlacement;
                    temp->alloc = 1;
                    temp->next = NULL;
                    temp->previousAllocated=latest;
                    temp->ptr = head->ptr+temp->placement;
                    temp->realPointer=temp;
                    mySize = mySize - requested;
                    trav->next = temp;
                    latest = temp;
                    return latest->ptr;
                }
                else {
                    // check up until the end of the memory
                    while (trav->next != NULL){
                        if (trav->next->placement - (trav->placement+trav->size) >= requested){ // if there is space for the requested memory segment before the next memory locatio
                            // create struct and give it its parameters
                            struct memoryList *temp = (struct memoryList *) malloc(sizeof(struct memoryList));
                            temp->size = requested;
                            temp->prev = trav;
                            temp->placement = trav->placement+trav->size;
                            temp->alloc = 1;
                            temp->next = trav->next;
                            temp->ptr = latest->ptr+(temp->placement-latest->placement);
                            temp->realPointer=temp;
                            trav->next = temp;
                            trav->prev=temp;
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
                        temp->prev = head;
                        temp->placement = 0;
                        temp->alloc = 1;
                        temp->next = trav;
                        temp->ptr = latest->ptr+(latest->placement+temp->placement+requested+1)%head->size; // this will be explained in the next segment
                        temp->realPointer=temp;
                        trav->prev->next = temp;
                        trav->prev=temp;
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
                            if (latest->placement > temp->placement){ // if the last block was closer to the end of the memory than the new one i have to calculate an offset based on their positions
                                // this is an example of how it calculates the offset
                                // say we have a block of 100 memory that has been filled to the end but some space have been freed at the front.
                                // head size would be 100 (size of memory block) and latest placement would be 99 (size of 1 block from 99-100)
                                // If we want to insert a block onto space 1 with size requested = 1
                                // temp->ptr = latest->ptr + (99+1+1+1)%100  <== ==> temp->ptr = latest->ptr + 2
                                // by this calculations we can have a continuous memory location
                                temp->ptr = latest->ptr+(latest->placement+temp->placement+requested+1)%head->size;
                            }
                            else {
                                // if the latest block is before this one we just need to calculate the offset onto the new place
                                temp->ptr = latest->ptr + (latest->placement - temp->placement);
                            }
                            temp->realPointer=temp;
                            trav->next->prev = temp;
                            trav->next = temp;
                            temp->previousAllocated=latest;
                            mySize = mySize - requested;
                            latest = temp;
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
void myfree(void* block) {
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
            free(trav->realPointer); // free the block from the heap
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
int mem_holes() {
    // first i make a struct to traverse the memory
    struct memoryList *trav;
    // set the traversal to first allocated memory location
    trav=head->next;
    if(trav != NULL) { //check if it is NULL
        int holes = 0;
        if (trav->placement != 0){holes++;} //if the first located memory is not at point 0 then there is a hole
        while (trav->next != NULL) { //go until the end of the memory
            if (trav->placement + trav->size != trav->next->placement) { //if the next memory is not right after the size of this memory then there is a hole
                holes++;
            }
            trav = trav->next; //go to next
        }
        if (trav->placement+trav->size < head->size){holes++;} //check if there is a hole in the end (the placement+size is not the same as the size of the whole memory)
        return holes;
    }
    if (mySize>0) return 1; // if the first allocated is a NULL (nothing has been assigned) then return 1 if there is a memory assigned else return 0
    else return 0;
}

/* Get the number of bytes allocated */
int mem_allocated(){
    // the full size minus the size that is not allocated on the memory
    return head->size-mySize;
}

/* Number of non-allocated bytes */
int mem_free() {
    // return the size that is not allocated on the memory
    return mySize;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free() {
    // first make a struct used to traverses memory and set it to the first allocated-
    struct memoryList *trav;
    trav = head->next;
    int largestHole = 0;
    if (trav != NULL) { //check if something is allocated
        if (trav->placement != 0) { // if the first allocated is not on space 0 then there is a hole which is from 0 to trav->placement
            largestHole = trav->placement;
        }
        while (trav->next != NULL) { //check until end of memory
            if (trav->placement + trav->size != trav->next->placement) { // check if they are just beside eachother (placement + size equals start of next allocated memory)
                if (trav->next->placement - (trav->placement + trav->size) > largestHole) { // here i check if the hole is bigger than the previous
                    largestHole = trav->next->placement - (trav->placement + trav->size);
                }
            }
            trav = trav->next; // go to next memory
        }
        if (trav->placement + trav->size < head->size) { // check if there is a hole in the end of the memory
            if (head->size - (trav->placement + trav->size) > largestHole) { // check if hole is bigger than previous biggest
                largestHole = head->size - (trav->placement + trav->size);
            }
        }
        return largestHole;
    } else return head->size; // if trav equals NULL from first placement then there is not assigned anything so biggest is whole memory
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size) {
    // traversal struct set to first allocated memory
    struct memoryList *trav;
    trav=head->next;
    if(trav != NULL) { //check if something is allocated
        int holes = 0;
        while (trav->next != NULL) { //check if you reached the end of memory
            if (trav->placement + trav->size != trav->next->placement && trav->next->placement - (trav->placement+trav->size) > size) { // check if there is a hole at first and then check if it is smaller than size
                holes++;
            }
            trav = trav->next; //go to next allocated memory
        }
        if (trav->placement+trav->size < head->size && head->size-(trav->placement+trav->size) < size){holes++;} // on the last allocated memory check if there is enough space until you reach the end of the whole memory
        return holes;
    }
    if (mySize>0) return 1; //if no malloc has been initialized check if memory has been initialized, if yes return 1 hole else 0
    else return 0;
}

char mem_is_alloc(void *ptr) {
    // make a struct to travers
    struct memoryList *trav;
    trav=head->next;
    while(trav != NULL) { //check if it is NULL
        if (trav->ptr == ptr) { //if the pointer is found then return y for yes
            return 'y';
        }
        trav = trav->next;
    }
    return 'n'; //nothing is found therefore return n for no
}

/*
 * Feel free to use these functions, but do not modify them.
 * The test code uses them, but you may find them useful.
 */


//Returns a pointer to the memory pool.
void *mem_pool() {
    return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total() {
    return head->size;
}


// Get string name for a strategy.
char *strategy_name(strategies strategy) {
    switch (strategy) {
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
strategies strategyFromString(char * strategy) {
    if (!strcmp(strategy,"best")) {
        return Best;
    }
    else if (!strcmp(strategy,"worst")) {
        return Worst;
    }
    else if (!strcmp(strategy,"first")) {
        return First;
    }
    else if (!strcmp(strategy,"next")) {
        return Next;
    }
    else {
        return 0;
    }
}


/*
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory() {
    // traversal struct
    struct memoryList *trav = head->next;
    while (trav != NULL) { //check if it is NULL
        printf("Placed at location: %d\tThe size is: %d\tThe memory ends at: %d\tThe memory is placed at location: %p\n", trav->placement, trav->size,trav->placement+trav->size,trav->ptr); //print out some data taken from trav
        trav = trav->next; //loop until end of memory
    }
}

/* Use this function to track memory allocation performance.
 * This function does not depend on your implementation,
 * but on the functions you wrote above.
 */
void print_memory_status() {
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
    if (argc > 1)
        strat = strategyFromString(argv[1]);
    else
        strat = Next;


    /* A simple example.
       Each algorithm should produce a different layout. */

    initmem(strat,500);

    a = mymalloc(100);
    b = mymalloc(100);
    c = mymalloc(100);
    myfree(b);
    d = mymalloc(50);
    myfree(a);
    e = mymalloc(25);

    print_memory();
    print_memory_status();

}