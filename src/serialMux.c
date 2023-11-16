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

int LATCH_DEBUG_SERIAL_WRITE;

static inline int check_done(unsigned int port) {
    while (ssc_mem[port][SSC_STATUS] & 0x4)
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
inline int writeMux(unsigned int port, FRAME *frame) {
    /* Check if the lenght is within the hardware limit (40 bits) */
    if (frame->dataLength > FRAME_DATA_BIT_SIZE) {
        storeError(ERR_SERIAL_MUX, ERC_COMMAND_VAL);  // Data length out of
                                                      // range
        return ERROR;
    }

    pthread_mutex_lock(&(ssc_lock[port]));

    /* 1 - Load the data registers. */
    ssc_mem[port][SSC_DATAWR] = frame->data[FRAME_DATA_LSW];

    /* 2 - Write the outgoing data lenght register with the number of bits to be
           sent. */
    ssc_mem[port][SSC_LENGTH] = frame->dataLength;

    /* 3 - Write the command register. This will initiate the transmission of
           data. */
    ssc_mem[port][SSC_COMMAND] = frame->command;
    ssc_mem[port][SSC_STATUS] = WR_SSC;

#ifdef DEBUG_SERIAL_WRITE
    if (LATCH_DEBUG_SERIAL_WRITE) {
        LATCH_DEBUG_SERIAL_WRITE = 0;
        printf("            (0x%04X) <- Frame.port: 0x%04X\n", MUX_PORT_ADD, frame.port);
        printf("            (0x%04X) <- Frame.data[LSW]: 0x%04X\n", MUX_DATA_ADD(FRAME_DATA_LSW),
               frame.data[FRAME_DATA_LSW]);
        printf("            (0x%04X) <- Frame.dataLength: 0x%04X\n", MUX_WLENGTH_ADD, frame.dataLength);
        printf("            (0x%04X) <- Frame.command: 0x%04X\n", MUX_COMMAND_ADD, frame.command);
    }
#endif /* DEBUG_SERIAL_WRITE */

    check_done(port);

    pthread_mutex_unlock(&ssc_lock[port]);

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
inline int readMux(unsigned int port, FRAME *frame) {
    /* Check if the lenght is within the hardware limit (40 bits) */
    if (frame->dataLength > FRAME_DATA_BIT_SIZE) {
        storeError(ERR_SERIAL_MUX, ERC_COMMAND_VAL);  // Data length out of
                                                      // range
        return ERROR;
    }

    pthread_mutex_lock(&ssc_lock[port]);
    /* 1 - Write the incoming data lenght register with the number of bits to be
           received. */
    ssc_mem[port][SSC_LENGTH] = frame->dataLength;

    /* 2 - Write the command register. This will initiate the transmission of
           data. */
    ssc_mem[port][SSC_COMMAND] = frame->command;
    ssc_mem[port][SSC_STATUS] = RD_SSC;

    check_done(port);

#ifdef DEBUG_SERIAL_READ
    printf("            (0x%04X) <- Frame.port: 0x%04X\n", MUX_PORT_ADD, frame.port);
    printf("            (0x%04X) <- Frame.dataLength: 0x%04X\n", MUX_RLENGTH_ADD, frame.dataLength);
    printf("            (0x%04X) <- Frame.command: 0x%04X\n", MUX_COMMAND_ADD, frame.command);
#endif /* DEBUG_SERIAL_READ */

    /* 3 - Load the data registers */
    frame->data[FRAME_DATA_MSW] = ssc_mem[port][SSC_DATARD1] & 0xFF;
    frame->data[FRAME_DATA_LSW] = ssc_mem[port][SSC_DATARD0];

    pthread_mutex_unlock(&ssc_lock[port]);

    return NO_ERROR;
}

unsigned char init_mem_map(void) {
    int fd_mem;
    if ((fd_mem = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno));
        exit(1);
    };
    printf("/dev/mem opened.\n");
    fflush(stdout);

    main_map = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, BASE_LPR);
    if (main_map == (void *)-1) {
        fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno));
        exit(1);
    }
    printf("Memory mapped at address %p.\n", main_map);

    owb_mem = main_map;
    for (unsigned char i = 0; i < NUMBER_OF_DEVICES; i++) {
        ssc_mem[i] = main_map + 8 * (i + 1);
    }
    fflush(stdout);

    return 0;
}