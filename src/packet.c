#include "packet.h"

#include <stdio.h>  /* printf */
#include <stdlib.h> /* system */
#include <string.h> /* memcpy */

#include "debug.h"
#include "error_local.h"
#include "frontend.h"
#include "globalDefinitions.h"
#include "globalOperations.h"
#include "main.h"
#include "serialMux.h"
#include "version.h"

/* Statics */
/* During initialization only special messages are allowed. To minimize the
   time spend to check if the software is initializing, we use a trick where the
   function pointer is shifted by 3 during initialization. This saves time
   during normal execution. */
static HANDLER classesHandler[CLASSES_NUMBER] = {
    standardRCAsHandler, standardRCAsHandler,
    specialRCAsHandler};  // The classes handler array is initialized.
static HANDLER modulesHandler[MODULES_NUMBER] = {
    cartridgeHandler,  // Cartridge 0 -> Band 1
    cartridgeHandler,  // Cartridge 1 -> Band 2
    cartridgeHandler,  // Cartridge 2 -> Band 3
    cartridgeHandler,  // Cartridge 3 -> Band 4
    cartridgeHandler,  // Cartridge 4 -> Band 5
    cartridgeHandler,  // Cartridge 5 -> Band 6
    cartridgeHandler,  // Cartridge 6 -> Band 7
    cartridgeHandler,  // Cartridge 7 -> Band 8
    cartridgeHandler,  // Cartridge 8 -> Band 9
    cartridgeHandler,  // Cartridge 9 -> Band 10
    powerDistributionHandler,
    ifSwitchHandler,
    cryostatHandler,
    lprHandler,
    fetimHandler};  // The modules handler array is initialized

void CANMessageHandler(void) {
    /* Redirect to the correct class handler depending on the RCA */
    currentClass = (CAN_ADDRESS & CLASSES_RCA_MASK) >> CLASSES_MASK_SHIFT;
    /* Check if the addressed class exist */
    if (currentClass >= CLASSES_NUMBER) {
        storeError(ERR_CAN,
                   ERC_RCA_CLASS);  // Error: RCA class outside allowed range
        newCANMsg = 0;              // Clear the new message flag
        return;
    }
    /* If in range call the function and let the handler figure out if the
       receiver is outfitted with the particular device addressed.
       Adding the initializing status variable allows to use different pointers
       while in intialization mode respect to the standard operation. */
    (classesHandler[currentClass])();  // Call the appropriate handler

    /* Clear the new message flag */
    newCANMsg = 0;

    return;
}

/* Standard message handler. */
static void standardRCAsHandler(void) {
    if (CAN_SIZE == CAN_MONITOR) {  // If it is a monitor message

        /* Check if maintenance mode. If we are, then block all standard
           standard messages. */
        if (frontend.mode == MAINTENANCE_MODE) {
            storeError(ERR_CAN,
                       ERC_MAINT_MODE);  // Front End in maintenance mode.
            CAN_STATUS = HARDW_BLKD_ERR;

            /* Return the error since it was a monitor message */

            return;
        }

        /* Store the NO_ERROR default status */
        CAN_STATUS = NO_ERROR;

        /* Check if the addressed module exist */
        currentModule = (CAN_ADDRESS & MODULES_RCA_MASK) >> MODULES_MASK_SHIFT;

        /* If it doesn't exist, return the error, otherwise call the correct
           handler */
        if (currentModule >= MODULES_NUMBER) {
            storeError(ERR_CAN, ERC_MODULE_RANGE);  // Sub-module out of range
            CAN_STATUS =
                HARDW_RNG_ERR;  // Notify incoming CAN message of the error
        } else {
            /* Redirect to the correct module handler depending on the RCA */
            (modulesHandler[currentModule])();  // Call the appropriate module
                                                // handler
        }

        sendCANMessage(TRUE);

        return;
    }

    /* If it is a control message... */
    /* Check if maintenance mode. If we are, then block all standard
       standard messages. */
    if (frontend.mode == MAINTENANCE_MODE) {
        storeError(ERR_CAN, ERC_MAINT_MODE);  // Front End in maintenance mode.
        return;
    }

    if (currentClass == 0) {                 // If it is on a monitor RCA
        storeError(ERR_CAN, ERC_RCA_RANGE);  // Control RCA out of range
        return;
    }

    /* Check if the addressed module exist */
    currentModule = (CAN_ADDRESS & MODULES_RCA_MASK) >> MODULES_MASK_SHIFT;
    if (currentModule >= MODULES_NUMBER) {
        storeError(ERR_CAN, ERC_MODULE_RANGE);  // Module outside allowed range
        /* Since the main module is in error, all the following submodule
           addressing is considered in error as well. Because of this it is not
           possible to write an error in the status byte of the last control
           message received. */
        return;
    }

    /* Redirect to the correct module handler depending on the RCA. Any possible
       error happening after this point will be stored in the last control
       message status byte. This is not true for message directed towards non
       existing harware because we don't know with what hardware to associate
       the error. Since nothing is returned from a control message, we cannot
       return the error state as well. */
    (modulesHandler[currentModule])();  // Call the appropriate module handler

    /* It's a control message, so we're done. */
    return;
}

/* Special messages handler */
static void specialRCAsHandler(void) {
    /* A static to take care of the ESNs monitoring */
    static unsigned char device = 0;
    /* Return code from stdlib calls: */
    static int ret;

    /* Set the status to the default */
    CAN_STATUS = NO_ERROR;

    if (CAN_SIZE == CAN_MONITOR) {  // If size = 0 -> message = monitor,
                                    // everything else -> message = control
        switch (CAN_ADDRESS) {
            case GET_ARCOM_VERSION_INFO:  // 0x20002 -> Return info about the
                                          // version of the firmware
                CAN_DATA(0) = VERSION_MAJOR;
                CAN_DATA(1) = VERSION_MINOR;
                CAN_DATA(2) = VERSION_PATCH;
                CAN_SIZE = 3;
                break;
            case GET_SPECIAL_MONITOR_RCAS:  // 0x20003 -> Return the special
                                            // monitor RCAs information
                /* Since this is a special case, the base address returned
                   to the AMBSI1 is not the actual base address but is the
                   first addressable CAN monitor message that will return
                   information about the ARCOM board.

                   Addresses 0x20000 and 0x20001 are already registered as
                   functions callbacks in the AMBSI1 firmware.
                   If we allow them to be registered again this would cause
                   problems at run time. */
                CAN_DATA(7) = (unsigned char)(LAST_SPECIAL_MONITOR_RCA >> 24);
                CAN_DATA(6) = (unsigned char)(LAST_SPECIAL_MONITOR_RCA >> 16);
                CAN_DATA(5) = (unsigned char)(LAST_SPECIAL_MONITOR_RCA >> 8);
                CAN_DATA(4) = (unsigned char)(LAST_SPECIAL_MONITOR_RCA);
                CAN_DATA(3) = (unsigned char)(GET_ARCOM_VERSION_INFO >> 24);
                CAN_DATA(2) = (unsigned char)(GET_ARCOM_VERSION_INFO >> 16);
                CAN_DATA(1) = (unsigned char)(GET_ARCOM_VERSION_INFO >> 8);
                CAN_DATA(0) = (unsigned char)(GET_ARCOM_VERSION_INFO);
                CAN_SIZE = CAN_FULL_SIZE;
                break;
            case GET_SPECIAL_CONTROL_RCAS:  // 0x20004 -> Return the special
                                            // control RCAs information
                CAN_DATA(7) = (unsigned char)(LAST_SPECIAL_CONTROL_RCA >> 24);
                CAN_DATA(6) = (unsigned char)(LAST_SPECIAL_CONTROL_RCA >> 16);
                CAN_DATA(5) = (unsigned char)(LAST_SPECIAL_CONTROL_RCA >> 8);
                CAN_DATA(4) = (unsigned char)(LAST_SPECIAL_CONTROL_RCA);
                CAN_DATA(3) = (unsigned char)((BASE_SPECIAL_CONTROL_RCA) >> 24);
                CAN_DATA(2) = (unsigned char)((BASE_SPECIAL_CONTROL_RCA) >> 16);
                CAN_DATA(1) = (unsigned char)((BASE_SPECIAL_CONTROL_RCA) >> 8);
                CAN_DATA(0) = (unsigned char)(BASE_SPECIAL_CONTROL_RCA);
                CAN_SIZE = CAN_FULL_SIZE;
                break;
            case GET_MONITOR_RCAS:  // 0x20005 -> Return the monitor RCAs
                                    // information
                CAN_DATA(7) = (unsigned char)(LAST_MONITOR_RCA >> 24);
                CAN_DATA(6) = (unsigned char)(LAST_MONITOR_RCA >> 16);
                CAN_DATA(5) = (unsigned char)(LAST_MONITOR_RCA >> 8);
                CAN_DATA(4) = (unsigned char)(LAST_MONITOR_RCA);
                CAN_DATA(3) = (unsigned char)((BASE_MONITOR_RCA) >> 24);
                CAN_DATA(2) = (unsigned char)((BASE_MONITOR_RCA) >> 16);
                CAN_DATA(1) = (unsigned char)((BASE_MONITOR_RCA) >> 8);
                CAN_DATA(0) = (unsigned char)(BASE_MONITOR_RCA);
                CAN_SIZE = CAN_FULL_SIZE;
                break;
            case GET_CONTROL_RCAS:  // 0x20006 -> Return the control RCAs
                                    // information
                CAN_DATA(7) = (unsigned char)(LAST_CONTROL_RCA >> 24);
                CAN_DATA(6) = (unsigned char)(LAST_CONTROL_RCA >> 16);
                CAN_DATA(5) = (unsigned char)(LAST_CONTROL_RCA >> 8);
                CAN_DATA(4) = (unsigned char)(LAST_CONTROL_RCA);
                CAN_DATA(3) = (unsigned char)((BASE_CONTROL_RCA) >> 24);
                CAN_DATA(2) = (unsigned char)((BASE_CONTROL_RCA) >> 16);
                CAN_DATA(1) = (unsigned char)((BASE_CONTROL_RCA) >> 8);
                CAN_DATA(0) = (unsigned char)(BASE_CONTROL_RCA);
                CAN_SIZE = CAN_FULL_SIZE;
                break;
            case GET_ERRORS_NUMBER:  // 0x2000C -> Returns the number of unread
                                     // errors
                CONV_UINT(0) = (errorNewest >= errorOldest)
                                   ? errorNewest - errorOldest
                                   : ERROR_HISTORY_LENGTH -
                                         (errorOldest - errorNewest) + 1;
                CAN_DATA(0) = CONV_CHR(1);
                CAN_DATA(1) = CONV_CHR(0);
                CAN_SIZE = CAN_INT_SIZE;
                break;
            case GET_NEXT_ERROR:  // 0x2000D -> Returns the next unread error
                CAN_SIZE = CAN_INT_SIZE;
                if (errorNewest == errorOldest) {
                    CONV_UINT(0) = 0xFFFF;
                } else {
                    CONV_UINT(0) = errorHistory[errorOldest];
                    errorOldest++;
                }
                CAN_DATA(0) = CONV_CHR(1);
                CAN_DATA(1) = CONV_CHR(0);
                break;
            case GET_FE_MODE:  // 0x2000E -> Returns the FE operating mode
                CAN_BYTE = frontend.mode;
                CAN_SIZE = CAN_BYTE_SIZE;
                break;

            case GET_TCPIP_ADDRESS:  // 0x2000F -> Get the Ethernet IP address
                CAN_DATA(0) = frontend.ipaddress[0];
                CAN_DATA(1) = frontend.ipaddress[1];
                CAN_DATA(2) = frontend.ipaddress[2];
                CAN_DATA(3) = frontend.ipaddress[3];
                CAN_SIZE = 4;
                break;

            case GET_LO_PA_LIMITS_TABLE_ESN + 0:
            case GET_LO_PA_LIMITS_TABLE_ESN + 1:
            case GET_LO_PA_LIMITS_TABLE_ESN + 2:
            case GET_LO_PA_LIMITS_TABLE_ESN + 3:
            case GET_LO_PA_LIMITS_TABLE_ESN + 4:
            case GET_LO_PA_LIMITS_TABLE_ESN + 5:
            case GET_LO_PA_LIMITS_TABLE_ESN + 6:
            case GET_LO_PA_LIMITS_TABLE_ESN + 7:
            case GET_LO_PA_LIMITS_TABLE_ESN + 8:
            case GET_LO_PA_LIMITS_TABLE_ESN + 9:
                // handler to retrieve maxSafeLoPaESN is here so it can be
                // called
                //  even if the cartridge is powered off.
                {
                    char *str = frontend
                                    .cartridge[(CAN_ADDRESS -
                                                GET_LO_PA_LIMITS_TABLE_ESN)]
                                    .lo.maxSafeLoPaESN;

                    CAN_DATA(7) = str[7];
                    CAN_DATA(6) = str[6];
                    CAN_DATA(5) = str[5];
                    CAN_DATA(4) = str[4];
                    CAN_DATA(3) = str[3];
                    CAN_DATA(2) = str[2];
                    CAN_DATA(1) = str[1];
                    CAN_DATA(0) = str[0];
                    CAN_SIZE = CAN_FULL_SIZE;
                }
                break;

            /* This will take care also of all the monitor request on
               special CAN control RCAs. It should be replaced by a proper
               structure as the one used for standard RCAs */
            default:
                storeError(ERR_CAN,
                           ERC_RCA_RANGE);  // Special Monitor RCA out of range
                CAN_STATUS = MON_CAN_RNG;   // Message out of range
                break;
        }

    } else {
        switch (CAN_ADDRESS) {
            case SET_EXIT_PROGRAM:  // 0x21000 -> Cause the entire program to
                                    // come to a "graceful" end
                stop = 1;
                break;
            case SET_REBOOT:  // 0x21001 -> Reboots the ARCOM board

                stop = 1;
                restart = 1;
                break;

            case SET_WRITE_NV_MEMORY:  // 0x2100D -> Write the flash disk

                frontendWriteNVMemory();
                break;

            case SET_FE_MODE:  // 0x2100E -> Set the FE operating mode

                if (CAN_BYTE == MAINTENANCE_MODE) {
                    if (frontend.mode != MAINTENANCE_MODE) {
                        // entering maintenance mode, start the ftp service:
                        ret = system("ftpd.exe /r\n");
                    }
                    frontend.mode = MAINTENANCE_MODE;

                } else if (CAN_BYTE == OPERATIONAL_MODE ||
                           CAN_BYTE == TROUBLESHOOTING_MODE ||
                           CAN_BYTE == SIMULATION_MODE) {
                    if (frontend.mode == MAINTENANCE_MODE) {
                        // leaving maintenance mode, stop the ftp service:
                        system("ftpd.exe /u\n");
                    }
                    frontend.mode = CAN_BYTE;

                } else {
                    storeError(ERR_CAN,
                               ERC_COMMAND_VAL);  // Illegal Front End Mode
                }
                break;

            case SET_LO_CLEAR_PA_LIMITS + 0:
            case SET_LO_CLEAR_PA_LIMITS + 1:
            case SET_LO_CLEAR_PA_LIMITS + 2:
            case SET_LO_CLEAR_PA_LIMITS + 3:
            case SET_LO_CLEAR_PA_LIMITS + 4:
            case SET_LO_CLEAR_PA_LIMITS + 5:
            case SET_LO_CLEAR_PA_LIMITS + 6:
            case SET_LO_CLEAR_PA_LIMITS + 7:
            case SET_LO_CLEAR_PA_LIMITS + 8:
            case SET_LO_CLEAR_PA_LIMITS + 9:
                // handlers for PA limits table is here so they can be called
                //  even if the cartridge is powered off.
                {
                    unsigned char band =
                        (unsigned char)(CAN_ADDRESS - SET_LO_CLEAR_PA_LIMITS);
                    loResetPaLimitsTable(band);
                }
                break;

            case SET_LO_SET_PA_LIMITS_ENTRY + 0:
            case SET_LO_SET_PA_LIMITS_ENTRY + 1:
            case SET_LO_SET_PA_LIMITS_ENTRY + 2:
            case SET_LO_SET_PA_LIMITS_ENTRY + 3:
            case SET_LO_SET_PA_LIMITS_ENTRY + 4:
            case SET_LO_SET_PA_LIMITS_ENTRY + 5:
            case SET_LO_SET_PA_LIMITS_ENTRY + 6:
            case SET_LO_SET_PA_LIMITS_ENTRY + 7:
            case SET_LO_SET_PA_LIMITS_ENTRY + 8:
            case SET_LO_SET_PA_LIMITS_ENTRY + 9:
                // handlers for PA limits table is here so they can be called
                //  even if the cartridge is powered off.
                {
                    unsigned char band, pol;
                    unsigned int ytoTuning;
                    float maxVD;

                    band = (unsigned char)(CAN_ADDRESS -
                                           SET_LO_SET_PA_LIMITS_ENTRY);

                    // extract the polarization.  It can be 0, 1, or 2 meaning
                    // 'both'
                    pol = CAN_DATA(0);
                    if (pol > 2) {
                        storeError(ERR_CAN, ERC_COMMAND_VAL);

                    } else {
                        // shift and extract the YTO tuning word:
                        CAN_DATA(0) = CAN_DATA(1);
                        CAN_DATA(1) = CAN_DATA(2);
                        changeEndianInt(CONV_CHR_ADD, CAN_DATA_ADD);
                        ytoTuning = CONV_UINT(0);
                        if (ytoTuning > 4095) {
                            storeError(ERR_CAN, ERC_COMMAND_VAL);

                        } else {
                            // shift and extract the maxVD:
                            CAN_DATA(0) = CAN_DATA(3);
                            CAN_DATA(1) = CAN_DATA(4);
                            CAN_DATA(2) = CAN_DATA(5);
                            CAN_DATA(3) = CAN_DATA(6);
                            changeEndian(CONV_CHR_ADD, CAN_DATA_ADD);
                            maxVD = CONV_FLOAT;
                            if (maxVD < 0 || maxVD > 2.5) {
                                storeError(ERR_CAN, ERC_COMMAND_VAL);
                            } else {
                                loAddPaLimitsEntry(band, pol, ytoTuning, maxVD);
                            }
                        }
                    }
                }
                break;

            default:
                storeError(ERR_CAN,
                           ERC_RCA_RANGE);  // Special Control RCA out of range
                break;
        }
    }
}
/* A function to build the outgoing message from CANMessage */
static void sendCANMessage(int appendStatusByte) {
    unsigned char cnt;

    /* If there is space in the message then store the status byte as first
       after the payload and increase the size of the message by 1 */
    if (appendStatusByte && CAN_SIZE < CAN_TX_MAX_PAYLOAD_SIZE) {
        CAN_DATA(CAN_SIZE++) = CAN_STATUS;
    }

    /* If it was coming from console, deal with the console */
    switch (CAN_SIZE) {
        case CAN_FLOAT_SIZE + 1:
            if (currentClass ==
                CONTROL_CLASS) {  // If it was a monitor on a control RCA
                changeEndian(CONV_CHR_ADD, CAN_DATA_ADD);
            }
            printf("%f", CONV_FLOAT);
            break;
        case CAN_FLOAT_SIZE + 2:
            if (currentClass ==
                CONTROL_CLASS) {  // If it was a monitor on a control RCA
                changeEndian(CONV_CHR_ADD, CAN_DATA_ADD);
            }
            printf("%f %u", CONV_FLOAT, CAN_DATA(4));
            break;
        case CAN_INT_SIZE + 1:
            if (currentClass ==
                CONTROL_CLASS) {  // If it was a monitor on a control RCA
                changeEndianInt(CONV_CHR_ADD, CAN_DATA_ADD);
            }
            printf("%u", CONV_UINT(0));
            break;
        case CAN_BYTE_SIZE + 1:
            printf("%u", CAN_BYTE);
            break;
        default:
            for (cnt = 0; cnt < CAN_SIZE; cnt++) {
                printf("%02X ", CAN_DATA(cnt));
            }
            break;
    }

    printf(" (status: 0x%02X)\n\n", CAN_STATUS);
}
