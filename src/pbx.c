/*
 * PBX: simulates a Private Branch Exchange.
 */

#include <stdlib.h>

#include "pbx.h"
#include "debug.h"
#include "semaphore.h"
#include "sys/socket.h"

#if 1
//have the PBX_MAX_EXTENSIONS for 
//Implementing a Doubly Linked List
struct pbx{
    TU *currentTU; //the current TU
    struct pbx *next;
    struct pbx *prev;
    sem_t semaphore;
};

static PBX *head = NULL; //global dummy head node
static int TUCounter = 0; // has to be <= PBX_MAX_EXTENSIONS
static int flag1 = 0; //global dummy head node

/*
 * Initialize a new PBX.
 *
 * @return the newly initialized PBX, or NULL if initialization fails.
 */

PBX *pbx_init() {
    // TO BE IMPLEMENTED
    pbx = calloc(1, sizeof(PBX));
    sem_init(&pbx->semaphore, 0, 1); //initializing the semaphore
    return pbx;
}

/*
 * Shut down a pbx, shutting down all network connections, waiting for all server
 * threads to terminate, and freeing all associated resources.
 * If there are any registered extensions, the associated network connections are
 * shut down, which will cause the server threads to terminate.
 * Once all the server threads have terminated, any remaining resources associated
 * with the PBX are freed.  The PBX object itself is freed, and should not be used again.
 *
 * @param pbx  The PBX tocalloc be shut down.
 */

void pbx_shutdown(PBX *pbx) {
    // TO BE IMPLEMENTED

    PBX *temp = head;
    sem_wait(&pbx->semaphore); //for sem_wait

    while(temp->next != NULL){
        shutdown(tu_fileno(temp->currentTU), SHUT_RDWR); //Disables further send and receive operations.
        sem_post(&pbx->semaphore); //for sem_wait
        pbx_unregister(pbx,temp->currentTU);
        sem_wait(&pbx->semaphore); //for sem_wait

        if(temp->next == NULL){
            free(temp->currentTU);
            free(temp); //frees the current
            break; //and then breaks
        }

        temp = temp->next;
        free(temp->prev->currentTU);
        free(temp->prev); //frees the previous
    }

    sem_destroy(&pbx->semaphore); //for sem_destroy
    free(pbx); //free at the end
}

/*
 * Register a telephone unit with a PBX at a specified extension number.
 * This amounts to "plugging a telephone unit into the PBX".
 * The TU is initialized to the TU_ON_HOOK state.
 * The reference count of the TU is increased and the PBX retains this reference
 *for as long as the TU remains registered.
 * A notification of the assigned extension number is sent to the underlying network
 * client.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU to be registered.
 * @param ext  The extension number on which the TU is to be registered.
 * @return 0 if registration succeeds, otherwise -1.
 */

int pbx_register(PBX *pbx, TU *tu, int ext) {
    if(TUCounter > PBX_MAX_EXTENSIONS){
        return -1; //too many max extensions 
    }
    // TO BE IMPLEMENTED
    sem_wait(&pbx->semaphore); //for sem_wait
    //The Node you are inputting in 
    PBX *newTU = malloc(sizeof(PBX)); //allocates space of the size of the PBX struct
    newTU->currentTU = tu; //setting the Tu
    tu_ref(newTU->currentTU, "registering"); // to ref inside the tu
    tu_set_extension(newTU->currentTU, ext);//setting extension

    PBX *temp = head;

    if(flag1 == 1){
        temp = temp->next;            
    }

    if(temp == NULL){
        newTU->next = NULL; //set the next to NULL
        head = malloc(sizeof(PBX)); //dummy node
        head->next = newTU; //setting the next of head to the TU
        newTU->prev = head; //sets the previous to head
        TUCounter = TUCounter + 1;
        flag1 = 1;
        sem_post(&pbx->semaphore); //for sem_post
        return 0;
    }
    else{
        while(temp->next != NULL){
            temp = temp->next; //keep iterating till u at the end of it
        }
        //Now temp should be at NULL

        temp->next = newTU;
        newTU->prev = temp; //connects the to the end
        newTU->next = NULL;
        sem_post(&pbx->semaphore); //for sem_post
        TUCounter = TUCounter + 1;
        return 0;
    }
    sem_post(&pbx->semaphore); //for sem_post
    return -1; //if someway it gets out of both of these
}

/*
 * Unregister a TU from a PBX.
 * This amounts to "unplugging a telephone unit from the PBX".
 * The TU is disassociated from its extension number.
 * Then a hangup operation is performed on the TU to cancel any
 * call that might be in progress.
 * Finally, the reference held by the PBX to the TU is released.
 *
 * @param pbx  The PBX.
 * @param tu  The TU to be unregistered.
 * @return 0 if unregistration succeeds, otherwise -1.
 */

int pbx_unregister(PBX *pbx, TU *tu) {
    // TO BE IMPLEMENTED
    sem_wait(&pbx->semaphore); //for sem_wait
    PBX *temp = head;

    if(temp->next == NULL){
        return -1; //if its empty nothing to register
    }
    temp = temp->next;
    while(temp != NULL){
        if(temp->currentTU == tu){
            if(temp->next == NULL){ //case 1
                temp->prev->next = NULL;
                tu_hangup(temp->currentTU);//hangs up the TU
                free(temp->currentTU); //frees the currentTU
                free(temp); //frees whatevers inside the node at the end
                TUCounter = TUCounter - 1;
                sem_post(&pbx->semaphore); //for sem_post
                return 0;
            }
            else{
                temp->prev->next = temp->next;
                temp->next->prev = temp->prev; //sets the 2 connections
                tu_hangup(temp->currentTU); //hangs up the TU
                free(temp->currentTU); //frees the currentTU
                free(temp); //frees whatevers inside the node at the end
                TUCounter = TUCounter - 1;
                sem_post(&pbx->semaphore); //for sem_post
                return 0;
            }
        
        }
        temp = temp->next; //iterates through
    }
    sem_post(&pbx->semaphore); //for sem_post
    return -1; //if nothing found

}

/*
 * Use the PBX to initiate a call from a specified TU to a specified extension.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU that is initiating the call.
 * @param ext  The extension number to be called.
 * @return 0 if dialing succeeds, otherwise -1.
 */

int pbx_dial(PBX *pbx, TU *tu, int ext) {
    // TO BE IMPLEMENTED
    sem_wait(&pbx->semaphore); //for sem_wait
    //find the extension 
    PBX *temp = head;
    if(temp->next == NULL){
        sem_post(&pbx->semaphore); //for sem_post
        return -1; //no nodes
    }

    while(temp->next != NULL){
        temp = temp->next;
        if(tu_extension(temp->currentTU) == ext){ //if the extension is found then do this
            tu_dial(tu, temp->currentTU);
            sem_post(&pbx->semaphore); //for sem_post
            return 0;
        }
    }
    tu_dial(tu, NULL);
    sem_post(&pbx->semaphore); //for sem_post
    return -1; //did not find

}

#endif