/*! \file   async.h
    \brief  Async operations header file

    <b> File information: </b><br>
    Created: 2009/03/25 18:20:44 by avaccari

    This file contains the information necessary to define the characteristics
    of the functions that take care of executing operation while the cpu is idle
    between incoming CAN messages. */

#ifndef _ASYNC_H
#define _ASYNC_H

/* Extra includes */

/* Defines */
#define ASYNC_DONE 1  //!< Global definition for a completed async job

/* Typedefs */
//! Current state of the asynchronous process
/*! This variables contains the current state of the asynchronous process:
    \param ASYNC_CRYOSTAT   the process is handling the cryostat
    \param ASYNC_CARTRIDGE  the process is handling the cartridges
    \param ASYNC_FETIM      the process is handling the FETIM
    \param ASYNC_OFF        the process is turned off
    \param ASYNC_ON         the process is starting */
typedef enum {
    ASYNC_CRYOSTAT,
    ASYNC_CARTRIDGE,
    ASYNC_FETIM,
    ASYNC_OFF,
    ASYNC_ON
} ASYNC_STATE;  //!< Current state of the async process

#endif /* _ASYNC_H */
