/*
 * "PBX" server module.
 * Manages interaction with a client telephone unit (TU).
 */
#include <stdlib.h>

#include "debug.h"
#include "pbx.h"
#include "server.h"
#include "csapp.h"

/*
 * Thread function for the thread that handles interaction with a client TU.
 * This is called after a network connection has been made via the main server
 * thread and a new thread has been created to handle the connection.
 */
#if 1
void *pbx_client_service(void *arg) {
    int filedescrip = *((int *)arg); //gets the file descriptor
    Pthread_detach(pthread_self()); //Detaches
    free(arg); //Frees the storage
    TU *receivedTU = tu_init(filedescrip); //takes the file descriptor and initializes a New TU
    pbx_register(pbx,receivedTU, filedescrip);    
    
    FILE *OpenedFile = Fdopen(filedescrip, "r");
    FILE *Streamtaken;
    char temp;
    size_t bufferlen = 0;
    char *buffr;
    
    while(1){
        Streamtaken = open_memstream(&buffr,&bufferlen); //here because it looks for another command always
        if (Streamtaken == NULL){
            continue; //so it waits for the next command
        }

            while((temp = fgetc(OpenedFile)) != EOF){
                if(temp == '\n'){
            //        flag = 1;
                    break;
                }

                if(temp == '\r'){
                    continue;
                }
                fputc(temp,Streamtaken);
            }

            if(temp == -1){
                break;
            }

            fflush(Streamtaken);
            char *firstindx;

            if(strcmp(buffr, "") == 0){ //if the buffer is empty, you would continue
                continue;
            }

            char copyof[bufferlen];;
            strcpy(copyof, buffr);
            firstindx = strtok(buffr, " "); //gets first one
            if(firstindx == NULL){
                free(buffr);
                continue;
            }
        
            //PICKUP
            if(strcmp(copyof, tu_command_names[TU_PICKUP_CMD]) == 0){ //if same string
                tu_pickup(receivedTU);
            }
            //DIAL #, where # is the number of the extension to be dialed.
            else if(strcmp(firstindx, tu_command_names[TU_DIAL_CMD]) == 0){
                char *getsthenum;
                getsthenum = strtok(copyof, " ");
                getsthenum = strtok(NULL, " ");
                if(getsthenum == NULL){
                    int neverused = 21;
                    neverused = neverused - 1;
                }
                else{
                    int flagone = 0;
                    size_t length = strlen(getsthenum);
                    size_t i = 0; 
                    for (i=0; i < length; i++) {
                        if(isdigit(getsthenum[i]) == 0){
                            flagone = 1;
                        }
                    }
                    if(flagone == 1){
                        int num = -1;
                        pbx_dial(pbx, receivedTU, num);
                    }
                    else{
                        int numpass = atoi(getsthenum);
                        pbx_dial(pbx, receivedTU, numpass);
                    } 
              }
            }
    
            //HANGUP
            else if(strcmp(firstindx, tu_command_names[TU_HANGUP_CMD]) == 0){
                tu_hangup(receivedTU);
            }
            //CHAT arguements would be this
            else if(strcmp(firstindx, tu_command_names[TU_CHAT_CMD]) == 0){
                char *getsthenum;
                getsthenum = strtok(copyof, " ");
                getsthenum = strtok(NULL, "\r");

                tu_chat(receivedTU, getsthenum);
            }

        free(buffr);
    }
        close(filedescrip);
        pbx_unregister(pbx, receivedTU);
        tu_unref(receivedTU, "Ended the server");
        return NULL;
}
#endif