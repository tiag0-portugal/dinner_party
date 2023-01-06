/**
 *  \file semSharedMemClient.c (implementation file)
 *
 *  \brief Problem name: Restaurant
 *
 *  Synchronization based on semaphores and shared memory.
 *  Implementation with SVIPC.
 *
 *  Definition of the operations carried out by the clients:
 *     \li waitFriends
 *     \li orderFood
 *     \li waitFood
 *     \li travel
 *     \li eat
 *     \li waitAndPay
 *
 *  \author Nuno Lau - December 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>

#include "probConst.h"
#include "probDataStruct.h"
#include "logging.h"
#include "sharedDataSync.h"
#include "semaphore.h"
#include "sharedMemory.h"

/** \brief logging file name */
static char nFic[51];

/** \brief shared memory block access identifier */
static int shmid;

/** \brief semaphore set access identifier */
static int semgid;

/** \brief pointer to shared memory region */
static SHARED_DATA *sh;

static bool waitFriends (int id);
static void orderFood (int id);
static void waitFood (int id);
static void travel (int id);
static void eat (int id);
static void waitAndPay (int id);

/**
 *  \brief Main program.
 *
 *  Its role is to generate the life cycle of one of intervening entities in the problem: the client.
 */
int main (int argc, char *argv[])
{
    int key;                                         /*access key to shared memory and semaphore set */
    char *tinp;                                                    /* numerical parameters test flag */
    int n;

    /* validation of command line parameters */
    if (argc != 5) { 
        freopen ("error_CT", "a", stderr);
        fprintf (stderr, "Number of parameters is incorrect!\n");
        return EXIT_FAILURE;
    }
    else {
       freopen (argv[4], "w", stderr);
       setbuf(stderr,NULL);
    }

    n = (unsigned int) strtol (argv[1], &tinp, 0);
    if ((*tinp != '\0') || (n >= TABLESIZE)) { 
        fprintf (stderr, "Client process identification is wrong!\n");
        return EXIT_FAILURE;
    }
    strcpy (nFic, argv[2]);
    key = (unsigned int) strtol (argv[3], &tinp, 0);
    if (*tinp != '\0') { 
        fprintf (stderr, "Error on the access key communication!\n");
        return EXIT_FAILURE;
    }

    /* connection to the semaphore set and the shared memory region and mapping the shared region onto the
       process address space */
    if ((semgid = semConnect (key)) == -1) { 
        perror ("error on connecting to the semaphore set");
        return EXIT_FAILURE;
    }
    if ((shmid = shmemConnect (key)) == -1) { 
        perror ("error on connecting to the shared memory region");
        return EXIT_FAILURE;
    }
    if (shmemAttach (shmid, (void **) &sh) == -1) { 
        perror ("error on mapping the shared region on the process address space");
        return EXIT_FAILURE;
    }

    /* initialize random generator */
    srandom ((unsigned int) getpid ());                                                 


    /* simulation of the life cycle of the client */
    travel(n);
    bool first = waitFriends(n);
    if (first) orderFood(n);
    waitFood(n);
    eat(n);
    waitAndPay(n);

    /* unmapping the shared region off the process address space */
    if (shmemDettach (sh) == -1) {
        perror ("error on unmapping the shared region off the process address space");
        return EXIT_FAILURE;;
    }

    return EXIT_SUCCESS;
}

/**
 *  \brief client goes to restaurant
 *
 *  The client takes his time to get to restaurant.
 *
 *  \param id client id
 */
static void travel (int id)
{
    usleep((unsigned int) floor ((1000000 * random ()) / RAND_MAX + 1000));
}

/**
 *  \brief client eats
 *
 *  The client takes his time to eat a pleasant dinner.
 *
 *  \param id client id
 */
static void eat (int id)
{
    usleep((unsigned int) floor ((MAXEAT * random ()) / RAND_MAX + 1000));
}

/**
 *  \brief client waits until table is complete 
 *
 *  Client should udpate state, first and last clients should register their values in shared data,
 *  last client should, in addition, inform the others that the table is complete.
 *  Client must wait in this function until the table is complete.
 *  The internal state should be saved.
 *
 *  \param id client id
 *
 *  \return true if first client, false otherwise
 */
static bool waitFriends(int id)
{
    bool first = false;

    if (semDown (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }


        sh->fSt.tableClients +=  1;
        sh->fSt.st.clientStat[id] = WAIT_FOR_FRIENDS;
        
        // Get the first client id (he will ask for the the food)
        if (sh->fSt.tableClients == 1){
            sh->fSt.tableFirst = id;
            first = true;
        }

        // Last Cliente to arrive releases all the table
        if (sh->fSt.tableClients == TABLESIZE) {

            for (int i = 0; i < TABLESIZE; i++) {
                // This semup is inside the critical region, because that way we are certain that while we increment the semaphor no one decrements it.
                semUp(semgid,sh->friendsArrived);
            }

            // Get the last client who will pay the bill
            sh->fSt.tableLast = id;

        }

        saveState(nFic,&sh->fSt);

    if (semUp (semgid, sh->mutex) == -1)                                                      /* exit critical region */
    { perror ("error on the up operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }

    // Wait for other friends
    if (semDown(semgid,sh->friendsArrived) == -1) {
        perror("error on the down operation for semaphore access (CT)");               
        exit(1);
    }

    return first;
}

/**
 *  \brief first client orders food.
 *
 *  This function is used only by the first client.
 *  The first client should update its state, request food to the waiter and 
 *  wait for the waiter to receive the request.
 *  
 *  The internal state should be saved.
 *
 *  \param id client id
 */


// This function is meant to be executed by the first client to arrive to the table
static void orderFood (int id)
{
    if (semDown (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }

        sh->fSt.foodRequest = 1; // Update the fooded request (needed by the waiter to determine what service to do)
        sh->fSt.st.clientStat[id] = FOOD_REQUEST;
        saveState(nFic,&sh->fSt);
    
    if (semUp (semgid, sh->mutex) == -1)                                                      /* exit critical region */
    { perror ("error on the up operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }

    //Signal Waiter
    if (semUp(semgid,sh->waiterRequest) == -1)                                                      /* exit critical region */
    { perror ("error on the up operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }

    // Wait to receive the request from the waiter
    if (semDown(semgid,sh->requestReceived) == -1) {
        perror("error on the down operation for semaphore access (CT)");               
        exit(1);
    }

}

/**
 *  \brief client waits for food.
 *
 *  The client updates its state, and waits until food arrives. 
 *  It should also update state after food arrives.
 *  The internal state should be saved twice.
 *
 *  \param id client id
 */
static void waitFood (int id)
{

    if (semDown (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }
        // Updates the state of the clients.
        sh->fSt.st.clientStat[id] = WAIT_FOR_FOOD;
        saveState(nFic,&sh->fSt);


    if (semUp (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }


    // Wait for the food to arrive

    if (semDown(semgid,sh->foodArrived) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }

    if (semDown (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }
        // Food as arrived the clients are eating
        sh->fSt.st.clientStat[id] = EAT;
        saveState(nFic,&sh->fSt);


    if (semUp (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }
}

/**
 *  \brief client waits for others to finish meal, last client to arrive pays the bill. 
 *
 *  The client updates state and waits for others to finish meal before leaving and update its state. 
 *  Last client to finish meal should inform others that everybody finished.
 *  Last client to arrive at table should pay the bill by contacting waiter and waiting for waiter to arrive.
 *  The internal state should be saved twice.
 *
 *  \param id client id
 */
static void waitAndPay (int id){

    
    bool last=false;
    

    if (semDown (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }
        // Update the number of people who have already eaten
        sh->fSt.tableFinishEat += 1;

        // Update state
        sh->fSt.st.clientStat[id] = WAIT_FOR_OTHERS;
        saveState(nFic,&sh->fSt);

        if (sh->fSt.tableFinishEat == TABLESIZE) {
            // This semup is inside the critical region, because that way we are certain that while we increment the semaphor no one decrements it.
            for (int i = 0; i < TABLESIZE; i++ ) {
                semUp(semgid,sh->allFinished);
            }
        }

        if (sh->fSt.tableLast == id){
            last = true;
        }

        

    if (semUp (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }

    if (semDown (semgid, sh->allFinished) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }

    // Last pays the bill
    if(last) { 

        if (semDown (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
           perror ("error on the down operation for semaphore access (CT)");
           exit (EXIT_FAILURE);
        }
            // Update last client state
            sh->fSt.st.clientStat[id] = WAIT_FOR_BILL;
            // Payment flag for the waiter to use
            sh->fSt.paymentRequest = 1;
            saveState(nFic,&sh->fSt);

            // Signal Waiter
            if (semUp (semgid, sh->waiterRequest) == -1) {                                                  /* enter critical region */
                perror ("error on the down operation for semaphore access (CT)");
                exit (EXIT_FAILURE);
            }

        if (semUp (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
            perror ("error on the down operation for semaphore access (CT)");
            exit (EXIT_FAILURE);
        }

        // Wait to receive the request from the waiter
        if (semDown(semgid,sh->requestReceived) == -1) {
            perror("error on the down operation for semaphore access (CT)");               
            exit(1);
        }

        
    }

    if (semDown (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }

        // Update Client to it's final state
        sh->fSt.st.clientStat[id] = FINISHED;
        saveState(nFic,&sh->fSt);

    if (semUp (semgid, sh->mutex) == -1) {                                                  /* enter critical region */
        perror ("error on the down operation for semaphore access (CT)");
        exit (EXIT_FAILURE);
    }

}

