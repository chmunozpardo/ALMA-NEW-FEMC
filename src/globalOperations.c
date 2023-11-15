/*! \file   globalOperations.c
    \brief  Global operations
    Hardware initialization and shutdown. */

/* Includes */
#include <stdio.h> /* printf */

#include "debug.h"
#include "error_local.h"
#include "frontend.h"
#include "globalDefinitions.h"
#include "main.h"
#include "owb.h"
#include "serialMux.h"
#include "timer.h"

int fd_mem;

volatile unsigned int *pico_mem;
pthread_mutex_t pico_lock[NUMBER_OF_DEVICES];

/* Initialization */
/*! This function takes care of initializing all the subsystem of the system.
    \return
        - \ref NO_ERROR -> if no error occurred
        - \ref ERROR    -> if something wrong happened */
int initialization(void) {
#ifdef DEBUG_STARTUP
    printf("Initializing...\n\n");
#endif

    for (unsigned char i = 0; i < NUMBER_OF_DEVICES; i++) pthread_mutex_init(&pico_lock[i], NULL);

    /* Initialize the error library */
    if (errorInit() == ERROR) {
        return ERROR;
    }

    init_mem_map();

/* One wire bus initialization */
#ifdef OWB
    if (owbInit() == ERROR) {
        return ERROR;
    }

    if (owbGetEsn() == ERROR) {
        return ERROR;
    }
#endif /* OWB */

    /* Switch to maintenance while initializing frontend and before enabling
     * interrupt. */
    frontend.mode = MAINTENANCE_MODE;

    /* At this point we gathered all the information about the ESNs and the
       communication is fully established with the AMBSI. */

    /* Initialize the frontend */
    if (frontendInit() == ERROR) {
        return ERROR;
    }

    /* Switch to operational mode */
    frontend.mode = OPERATIONAL_MODE;

#ifdef DEBUG_STARTUP
    printf("End initialization!\n\n");
#endif

    return NO_ERROR;
}

/*! This function takes care of shutting down all the subsystems.
    \return
        - \ref NO_ERROR -> if no error occurred
        - \ref ERROR    -> if something wrong happened */
int shutDown(void) {
#ifdef DEBUG_STARTUP
    printf("Shutting down...\n\n");
#endif

    /* Switch to maintenance so no commands will be processed during shutdown.
     */
    frontend.mode = MAINTENANCE_MODE;

    /* Shut down the frontend */
    frontendStop();

    /* Shut down the error handling */
    errorStop();

#ifdef DEBUG_STARTUP
    printf("Shut down complete! Exiting...\n\n");
#endif

    return NO_ERROR;
}
