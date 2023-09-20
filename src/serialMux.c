/*! \file   serialMux.c
    \brief  Serial multiplexing board functions

    <b> File information: </b><br>
    Created: 2004/08/24 16:24:39 by avaccari

    This file contains all the functions necessary to control the serial
    multiplexing board.

    The functions in this module provide the lowest level communication with
    the hardware.
    This module should contain the hardware depended part of the code.
    No check is actually performed to insure that the data is transmitted to the
    serial mux board through ISA commands.
    The only way to do this would be to actually perform a read right after the
    write to insure that the written data is correct. This check is available
    only for few of the addresses due to the FPGA configuration. */

/* Includes */
#include "serialMux.h"

#include <stdio.h> /* printf */

#include "debug.h"
#include "error_local.h"
#include "globalDefinitions.h"
#include "timer.h"

/* Globals */
/* Externs */
FRAME frame; /*! This variable is used to create the serial frame to be
                 handled by the multiplexing board. */

int LATCH_DEBUG_SERIAL_WRITE;

static inline int check_done() {
    while (pico_mem[STATUS] & 0x4)
        ;

    return NO_ERROR;
}

/* Write the data through the Mux board */
/*! This function will trasmit the current courrent \ref frame content to the
    selected device.

    This function performs the following operations:
        -# Check the busy status to verify the synchronous serial bus is ready
           to begin a new cycle
        -# Select the desired port/device
            - by writing to the port select register
        -# Load the data register with the data output word(s). Bits must be
           left-justified and they start from bit 15 of the most significant
           word in the data register. The data is most significant bit first.
        -# Write the desired data word length to be transmitted into the lenght
           register
        -# Write the command register with the desired command. This will
           initiate the serial transfer

    \return
        - \ref NO_ERROR -> if no error occurred
        - \ref ERROR    -> if something wrong happened */
int writeMux(void) {
    /* Check if the lenght is within the hardware limit (40 bits) */
    if (frame.dataLength > FRAME_DATA_BIT_SIZE) {
        storeError(ERR_SERIAL_MUX, ERC_COMMAND_VAL);  // Data length out of
                                                      // range
        return ERROR;
    }

    /* 1 - Wait on busy status */
    if (check_done() == ERROR) {
        return ERROR;
    }

    /* 2 - Select the desired port/device. The check on the availability of the
           device on the selected port should have been done by the CAN message
           handlers. At the point of this call the software should already have
           returned if the addressed device is not available. */

    /* 3 - Load the data registers. */
    pico_mem[DATAWR] = frame.data[FRAME_DATA_LSW];

    /* 4 - Write the outgoing data lenght register with the number of bits to be
           sent. */
    pico_mem[LENGTH] = frame.dataLength;

    /* 5 - Write the command register. This will initiate the transmission of
           data. */
    pico_mem[COMMAND] = frame.command;

    pico_mem[STATUS] = WR_SSC;

    check_done();

    return NO_ERROR;
}

/* Reads the data through the Mux board */
/*! This function will read the required data from the selected device into the
    current \ref frame.

    This function performs the following operations:
        -# Check the busy status to verify the synchronous serial bus is ready
           to begin a new cycle
        -# Select the desiref port/device
            - by writing to the port select register
        -# Write the desired data word length to be read into the lenght
           register
        -# Write the command register with the desired command. This will
           initiate the serial transfer
        -# Check the busy status to verify the read cycle is complete
        -# Load the frame input word(s) with the data register. Bits will be
           right-justified and they end with bit 0 of the least significant
           word in the data register. The data is most significant bit first.

    \return
        - \ref NO_ERROR -> if no error occurred
        - \ref ERROR    -> if something wrong happened */
int readMux(void) {
    /* Check if the lenght is within the hardware limit (40 bits) */
    if (frame.dataLength > FRAME_DATA_BIT_SIZE) {
        storeError(ERR_SERIAL_MUX, ERC_COMMAND_VAL);  // Data length out of
                                                      // range
        return ERROR;
    }

    /* 1 - Wait on busy status */
    if (check_done() == ERROR) {
        return ERROR;
    }

    /* 2 - Select the desired port/device. The check on the availability of the
           device on the selected port should have been done by the CAN message
           handlers. At the point of this call the software should already have
           returned if the addressed device is not available. */

    /* 3 - Write the incoming data lenght register with the number of bits to be
           received. */
    pico_mem[LENGTH] = frame.dataLength;

    /* 4 - Write the command register. This will initiate the transmission of
           data. */
    pico_mem[COMMAND] = frame.command;

    pico_mem[STATUS] = RD_SSC;

    check_done();

    /* 6 - Load the data registers */
    frame.data[FRAME_DATA_MSW] = pico_mem[DATARD1] & 0xFF;
    frame.data[FRAME_DATA_LSW] = pico_mem[DATARD0];

    return NO_ERROR;
}

unsigned char init_mem_map(void) {
    if ((fd_mem = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno));
        exit(1);
    };
    printf("/dev/mem opened.\n");
    fflush(stdout);

    pico_mem = mmap(NULL, BASE_LPR + MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, BASE_LPR);
    if (pico_mem == (void *)-1) {
        fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno));
        exit(1);
    }
    printf("Memory mapped at address %p.\n", pico_mem);
    fflush(stdout);

    return 0;
}