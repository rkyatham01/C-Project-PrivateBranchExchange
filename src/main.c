#include <stdlib.h>
#include <unistd.h>

#include "pbx.h"
#include "server.h"
#include "debug.h"
#include "csapp.h"
#include <getopt.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#if 1
static void terminate(int status);

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */

int listenfd, *connfdp;

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
/* 
 * echoservert.c - A concurrent echo server using threads
 */
/* $begin echoservertmain */
    char* port = NULL;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid; 

    if (argc != 3) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(0);
    }

    //Using GetOpt TO get the -p
    int getchar; //Handles the option

    //Handles the flags
    while((getchar = getopt(argc,argv, "p:")) != -1){
      {
        switch(getchar)
        {
            case 'p':
                port = optarg; //sets the port number here
                break;

            default:
                fprintf(stderr, "Incorrect Format");
                return EXIT_FAILURE;

        }
      }
    }

    listenfd = Open_listenfd(port); //takes a char ptr
    debug("Initializing PBX...");
    pbx = pbx_init();

    //The handler for sigaction
    struct sigaction new_action;
    new_action.sa_handler = &terminate;
    Sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGHUP, &new_action, NULL);
    signal(SIGPIPE, SIG_IGN);

    while (1){
	    connfdp = Malloc(sizeof(int)); //line:conc:echoservert:beginmalloc
	    *connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen); //line:conc:echoservert:endmalloc
	    Pthread_create(&tid, NULL, pbx_client_service, connfdp);
    }

}

// Perform required initialization of the PBX module.

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    free(connfdp);
    close(listenfd); //closing the file descriptor for the server
    debug("PBX server terminating");
    exit(status);
}

#endif