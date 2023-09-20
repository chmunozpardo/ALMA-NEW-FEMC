/*! \file   main.c
    \brief  Main module functions
    This is \ref main.c */

/* Includes */
#include "main.h"

#include <stdio.h>  /* printf */
#include <stdlib.h> /* system */
#include <time.h>

#include "async.h"
#include "debug.h"
#include "error_local.h"
#include "globalDefinitions.h"
#include "globalOperations.h"
#include "packet.h"
#include "timer.h"
#include "version.h"

/* Globals */
/* Externs */
unsigned char stop = 0;    /*!< This global can be set by any module and will
                              gently    stop the program if necessary. */
unsigned char restart = 0; /*!< This global can be set by any module and will
                                cause the system to reboot after a stop is
                                received. */

volatile unsigned char newCANMsg =
    0;                           /*!< This variable is a semaphore which will
                                      notify the entire program of the arrival
                                      of a new CAN message. It is cleared
                                      once the message has been dealt with. */
CAN_MESSAGE CANMessage;          /*!< This variable hold the latest message
                                      received. */
unsigned char currentModule = 0; /*!< This variable stores the current module
                                      information. This is the front end item
                                      the request are currently directed to. */
unsigned char currentClass = 0;  /*!< This variable stores the current class
                                      information. This is a specifier of the
                                      type of message: monitor, control or
                                      special that has been received. */

int main(void) {
    /* Print version information */
    displayVersion();

    /* Initialize the frontend */
    if (initialization() == ERROR) {
        return ERROR;
    }

    /* Main loop */
    CAN_ADDRESS = 0x1D038;
    CAN_SIZE = 4;
    CONV_FLOAT = 2.5;
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CANMessageHandler();

    sleep(1);

    CAN_ADDRESS = 0x0D038;
    CAN_SIZE = 0;
    CANMessageHandler();

    CAN_ADDRESS = 0x0D000;
    CAN_SIZE = 0;
    CANMessageHandler();

    CAN_ADDRESS = 0x0D010;
    CAN_SIZE = 0;
    CANMessageHandler();

    CAN_ADDRESS = 0x0D030;
    CAN_SIZE = 0;
    CANMessageHandler();

    CAN_ADDRESS = 0x0D031;
    CAN_SIZE = 0;
    CANMessageHandler();

    CAN_ADDRESS = 0x0D032;
    CAN_SIZE = 0;
    CANMessageHandler();

    CAN_ADDRESS = 0x0D034;
    CAN_SIZE = 0;
    CANMessageHandler();

    CAN_ADDRESS = 0x0D036;
    CAN_SIZE = 0;
    CANMessageHandler();

    CAN_ADDRESS = 0x0D038;
    CAN_SIZE = 0;
    CANMessageHandler();

    /* Shut down the frontend */
    if (shutDown() == ERROR) {
    }

    return NO_ERROR;
}
