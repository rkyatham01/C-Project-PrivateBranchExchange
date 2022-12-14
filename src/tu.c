/*
 * TU: simulates a "telephone unit", which interfaces a client with the PBX.
 */
#include <stdlib.h>

#include "pbx.h"
#include "debug.h"
#include <pthread.h>
#include <string.h>

#if 1
struct tu{
    TU_STATE thestate;
    int fd; //for file descriptor
    int refcounter; //for reference
    TU *forConnected; //for connected its pier
    int ext;
    pthread_mutex_t mutex; //each struct has its own mutex
};

/*
 * Initialize a TU
 *
 * @param fd  The file descriptor of the underlying network connection.
 * @return  The TU, newly initialized and in the TU_ON_HOOK state, if initialization
 * was successful, otherwise NULL.
 */
TU *tu_init(int fd) {
    TU *initializedTU = malloc(sizeof(TU));    
    initializedTU->fd = fd; //setting the file descriptor
    initializedTU->refcounter = 0; //startin the count at 0 each TU has own reference count
    initializedTU->thestate = TU_ON_HOOK; //initializing to on Hook
    pthread_mutex_init(&initializedTU->mutex, NULL); //initially set the mutex to NULL
    return initializedTU;
}

/*
 * Increment the reference count on a TU.
 *
 * @param tu  The TU whose reference count is to be incremented
 * @param reason  A string describing the reason why the count is being incremented
 * (for debugging purposes).
 */

void tu_ref(TU *tu, char *reason) {
    // TO BE IMPLEMENTED
    tu->refcounter = tu->refcounter + 1;
    debug("%s", reason); //reason
}

/*
 * Decrement the reference count on a TU, freeing it if the count becomes 0.
 *
 * @param tu  The TU whose reference count is to be decremented
 * @param reason  A string describing the reason why the count is being decremented
 * (for debugging purposes).
 */

void tu_unref(TU *tu, char *reason) {
    // TO BE IMPLEMENTED

    tu->refcounter = tu->refcounter - 1;

    if(tu->refcounter == 0){
        pthread_mutex_destroy(&tu->mutex);
        free(tu);
        return;
    } 

    debug("%s", reason); //reason
}

/*
 * Get the file descriptor for the network connection underlying a TU.
 * This file descriptor should only be used by a server to read input from
 * the connection.  Output to the connection must only be performed within
 * the PBX functions.
 *
 * @param tu
 * @return the underlying file descriptor, if any, otherwise -1.
 */
int tu_fileno(TU *tu) {
    // TO BE IMPLEMENTED
    if(tu == NULL){
        return -1;
    }
    //else you do this
    return tu->fd; //returns the file descriptor
}

/*
 * Get the extension number for a TU.
 * This extension number is assigned by the PBX when a TU is registered
 * and it is used to identify a particular TU in calls to tu_dial().
 * The value returned might be the same as the value returned by tu_fileno(),
 * but is not necessarily so.
 *
 * @param tu
 * @return the extension number, if any, otherwise -1.
 */

int tu_extension(TU *tu) {
    // TO BE IMPLEMENTED
    if(tu == NULL){
        return -1;
    }
    //else you do this
    return tu->ext;
}

/*
 * Set the extension number for a TU.
 * A notification is set to the client of the TU.
 * This function should be called at most once one any particular TU.
 *
 * @param tu  The TU whose extension is being set.
 */

int tu_set_extension(TU *tu, int ext) {
    // TO BE IMPLEMENTED
    if(tu == NULL){
        return -1;
    }
    dprintf(tu->fd, "%s",tu_state_names[tu->thestate]);        
    char buffr[16];
    snprintf(buffr, 16, " %d%s", tu->fd, EOL);
    write(tu->fd, buffr,strlen(buffr));
    tu->ext = ext; 
    return 0;
}

/*
 * Initiate a call from a specified originating TU to a specified target TU.
 *   If the originating TU is not in the TU_DIAL_TONE state, then there is no effect.
 *   If the target TU is the same as the originating TU, then the TU transitions
 *     to the TU_BUSY_SIGNAL state.
 *   If the target TU already has a peer, or the target TU is not in the TU_ON_HOOK
 *     state, then the originating TU transitions to the TU_BUSY_SIGNAL state.
 *   Otherwise, the originating TU and the target TU are recorded as peers of each other
 *     (this causes the reference count of each of them to be incremented),
 *     the target TU transitions to the TU_RINGING state, and the originating TU
 *     transitions to the TU_RING_BACK state.
 *
 * In all cases, a notification of the resulting state of the originating TU is sent to
 * to the associated network client.  If the target TU has changed state, then its client
 * is also notified of its new state.
 *
 * If the caller of this function was unable to determine a target TU to be called,
 * it will pass NULL as the target TU.  In this case, the originating TU will transition
 * to the TU_ERROR state if it was in the TU_DIAL_TONE state, and there will be no
 * effect otherwise.  This situation is handled here, rather than in the caller,
 * because here we have knowledge of the current TU state and we do not want to introduce
 * the possibility of transitions to a TU_ERROR state from arbitrary other states,
 * especially in states where there could be a peer TU that would have to be dealt with.
 *
 * @param tu  The originating TU.
 * @param target  The target TU, or NULL if the caller of this function was unable to
 * identify a TU to be dialed.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */

int tu_dial(TU *tu, TU *target) {
    // TO BE IMPLEMENTED
    pthread_mutex_lock(&tu->mutex); //locks tu
    
    if(tu->thestate != TU_DIAL_TONE){
        dprintf(tu->fd, "%s\n",tu_state_names[tu->thestate]);        
        pthread_mutex_unlock(&tu->mutex); //locks tu
        return 0;
    }

    if(tu == target){ //check 4 if its the same pier
        tu->thestate = TU_BUSY_SIGNAL; //busy state
        dprintf(tu->fd, "%s\n",tu_state_names[TU_BUSY_SIGNAL]);        
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        return 0;
    }

     if(target == NULL){ //check 2
        if(tu->thestate == TU_DIAL_TONE){
            tu->thestate = TU_ERROR; //turns this to this state
            dprintf(tu->fd, "%s\n",tu_state_names[TU_ERROR]);        
            pthread_mutex_unlock(&tu->mutex); //unlocks the lock
            return 0;
        }
        else{
            dprintf(tu->fd, "%s\n",tu_state_names[tu->thestate]);        
            pthread_mutex_unlock(&tu->mutex); //unlocks the lock
            return 0;
        }
    }
    
    pthread_mutex_lock(&target->mutex); //locks tu

    if(target->forConnected != NULL || target->thestate != TU_ON_HOOK){
        tu->thestate = TU_BUSY_SIGNAL; //turns to busy state in this case
        dprintf(tu->fd, "%s\n",tu_state_names[TU_BUSY_SIGNAL]);        
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        pthread_mutex_unlock(&target->mutex); //unlocks the lock
        return 0;
    }

    //now we know the target stage is not NULL
    //We do the actual thing

    //otherwise this happens
    tu->forConnected = target;
    target->forConnected = tu; //connecting both like telephones
    tu_ref(tu, "Increased ref from tu");
    tu_ref(target, "Increased ref from target");

    tu->thestate = TU_RING_BACK; //sets this
    dprintf(tu->fd, "%s\n",tu_state_names[TU_RING_BACK]);        
    target->thestate = TU_RINGING; //sets this
    dprintf(target->fd, "%s\n",tu_state_names[TU_RINGING]);        

    pthread_mutex_unlock(&tu->mutex); //unlocks the lock
    pthread_mutex_unlock(&target->mutex); //unlocks the lock
    return 0;

}

/*
 * Take a TU receiver off-hook (i.e. pick up the handset).
 *   If the TU is in neither the TU_ON_HOOK state nor the TU_RINGING state,
 *     then there is no effect.
 *   If the TU is in the TU_ON_HOOK state, it goes to the TU_DIAL_TONE state.
 *   If the TU was in the TU_RINGING state, it goes to the TU_CONNECTED state,
 *     reflecting an answered call.  In this case, the calling TU simultaneously
 *     also transitions to the TU_CONNECTED state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The TU that is to be picked up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
int tu_pickup(TU *tu) {
    // TO BE IMPLEMENTED
    pthread_mutex_lock(&tu->mutex); //locks tu
    
    if(tu->thestate != TU_ON_HOOK && tu->thestate != TU_RINGING){
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        return 0; //successfull
    }

    else if(tu->thestate == TU_ON_HOOK){
        tu->thestate = TU_DIAL_TONE;
        dprintf(tu->fd, "%s\n",tu_state_names[TU_DIAL_TONE]);        
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        return 0;
    }
    else if(tu->thestate == TU_RINGING){
        pthread_mutex_lock(&tu->forConnected->mutex); //unlocks the lock
        tu->thestate = TU_CONNECTED;
        dprintf(tu->fd, "%s %d\n",tu_state_names[TU_CONNECTED],tu->forConnected->fd);        
        tu->forConnected->thestate = TU_CONNECTED;
        dprintf(tu->forConnected->fd, "%s %d\n",tu_state_names[TU_CONNECTED],tu->fd);        
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        pthread_mutex_unlock(&tu->forConnected->mutex); //unlocks the lock
        return 0;
    }
    pthread_mutex_unlock(&tu->mutex); //unlocks the lock
    return 0; //an error
    //before return
}

/*
 * Hang up a TU (i.e. replace the handset on the switchhook).
 *
 *   If the TU is in the TU_CONNECTED or TU_RINGING state, then it goes to the
 *     TU_ON_HOOK state.  In addition, in this case the peer TU (the one to which
 *     the call is currently connected) simultaneously transitions to the TU_DIAL_TONE
 *     state.
 *   If the TU was in the TU_RING_BACK state, then it goes to the TU_ON_HOOK state.
 *     In addition, in this case the calling TU (which is in the TU_RINGING state)
 *     simultaneously transitions to the TU_ON_HOOK state.
 *   If the TU was in the TU_DIAL_TONE, TU_BUSY_SIGNAL, or TU_ERROR state,
 *     then it goes to the TU_ON_HOOK state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The tu that is to be hung up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */

int tu_hangup(TU *tu) {
    // TO BE IMPLEMENTED
    pthread_mutex_lock(&tu->mutex); //locks tu
    
    if(tu->thestate == TU_DIAL_TONE || tu->thestate == TU_BUSY_SIGNAL || tu->thestate == TU_ERROR){//if its in any of these state
        tu->thestate = TU_ON_HOOK; //then it goes to this state
        dprintf(tu->fd, "%s\n",tu_state_names[TU_ON_HOOK]);        
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        return 0;
    }
    if(tu->thestate == TU_CONNECTED || tu->thestate == TU_RINGING){ //if this happens
        tu->thestate = TU_ON_HOOK; //then it goes to this state
        dprintf(tu->fd, "%s\n",tu_state_names[TU_ON_HOOK]);
        pthread_mutex_lock(&tu->forConnected->mutex); //locks the tu connection
        tu->forConnected->thestate = TU_DIAL_TONE; //the pier goes to this state
        dprintf(tu->forConnected->fd, "%s\n",tu_state_names[TU_DIAL_TONE]);
        TU *targ = tu->forConnected;
        tu->forConnected = NULL;
        targ->forConnected = NULL;
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        pthread_mutex_unlock(&targ->mutex); //unlocks the lock
        tu_unref(targ, "HANGING UP");
        tu_unref(tu, "HANGING UP");
        return 0;
    }
    else if(tu->thestate == TU_RING_BACK){
        pthread_mutex_lock(&tu->forConnected->mutex); //locks the tu connection
        tu->thestate = TU_ON_HOOK; //then it goes to this state
        dprintf(tu->fd, "%s\n",tu_state_names[TU_ON_HOOK]);        
        tu->forConnected->thestate = TU_ON_HOOK; //connected state would also go to TU_ON_HOOK state
        dprintf(tu->forConnected->fd, "%s\n",tu_state_names[TU_ON_HOOK]);        
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        pthread_mutex_unlock(&tu->forConnected->mutex); //unlocks the lock
        tu_unref(tu->forConnected, "HANGING UP");
        tu_unref(tu, "HANGING UP");
        return 0;
    }

    dprintf(tu->fd, "%s\n",tu_state_names[tu->thestate]);        
    pthread_mutex_unlock(&tu->mutex); //unlocks the lock
    return 0;
}

/*
 * "Chat" over a connection.
 *
 * If the state of the TU is not TU_CONNECTED, then nothing is sent and -1 is returned.
 * Otherwise, the specified message is sent via the network connection to the peer TU.
 * In all cases, the states of the TUs are left unchanged and a notification containing
 * the current state is sent to the TU sending the chat.
 *
 * @param tu  The tu sending the chat.
 * @param msg  The message to be sent.
 * @return 0  If the chat was successfully sent, -1 if there is no call in progress
 * or some other error occurs.
 */

int tu_chat(TU *tu, char *msg) {
    // TO BE IMPLEMENTED

    pthread_mutex_lock(&tu->mutex); //locks tu
    if(tu->forConnected == NULL){ //if its NULl ,then do this
        dprintf(tu->fd, "%s\n",tu_state_names[tu->thestate]);        
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        return -1;
    }


    if(tu->thestate != TU_CONNECTED){ //not connected returns -1
        dprintf(tu->fd, "%s\n",tu_state_names[tu->thestate]);        
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        pthread_mutex_unlock(&tu->forConnected->mutex); //unlocks the lock
        return -1;
    }
    else if(tu->thestate == TU_CONNECTED){ //if it is connected then
        pthread_mutex_lock(&tu->forConnected->mutex); //locks the tu connection
        if(msg == NULL){
            msg = "";
            dprintf(tu->forConnected->fd, "CHAT %s\n",msg);        
            dprintf(tu->fd, "%s %d\n",tu_state_names[tu->thestate], tu->forConnected->fd);           
            pthread_mutex_unlock(&tu->mutex); //unlocks the lock
            pthread_mutex_unlock(&tu->forConnected->mutex); //unlocks the lock
            return 0;
        }

        dprintf(tu->forConnected->fd, "CHAT %s\n",msg);        
        dprintf(tu->fd, "%s %d\n",tu_state_names[tu->thestate], tu->forConnected->fd);        
        pthread_mutex_unlock(&tu->mutex); //unlocks the lock
        pthread_mutex_unlock(&tu->forConnected->mutex); //unlocks the lock
        return 0;
    }
    
    dprintf( tu->fd, "%s\n",tu_state_names[tu->thestate]);        
    pthread_mutex_unlock(&tu->mutex); //unlocks the lock
    return 0;
}

#endif
