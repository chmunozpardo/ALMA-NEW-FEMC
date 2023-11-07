/*! \file   ifSwitch.c
    \brief  IF Switch functions

    <b> File information: </b><br>
    Created: 2004/08/24 16:24:39 by avaccari

    This file contains all the functions necessary to handle IF Switch
    events. */

/* Includes */
#include <stdio.h>  /* printf */
#include <string.h> /* memcpy */

#include "debug.h"
#include "error_local.h"
#include "frontend.h"
#include "ifSerialInterface.h"

/* Globals */
/* Externs */
/* An extern to perform the mapping between bands and IF switch way. */
unsigned char currentIfSwitchWay[IF_SWITCH_WAYS] = {WAY0,   // Band 1
                                                    WAY1,   // Band 2
                                                    WAY2,   // Band 3
                                                    WAY3,   // Band 4
                                                    WAY4,   // Band 5
                                                    WAY5,   // Band 6
                                                    WAY6,   // Band 7
                                                    WAY7,   // Band 8
                                                    WAY8,   // Band 9
                                                    WAY9};  // Band 10
/* Statics */
static HANDLER_INT ifSwitchModulesHandler[IF_SWITCH_MODULES_NUMBER] = {
    ifChannelHandler, ifChannelHandler, ifChannelHandler, ifChannelHandler, bandSelectHandler, allChannelsHandler};

/* IF Switch handler */
/*! This function will be called by the CAN message handler when the received
    message is pertinent to the IF Switch. */
void ifSwitchHandler(int currentModule) {
#ifdef DEBUG_IFSWITCH
    printf(" IF Switch\n");
#endif /* DEBUG_IFSWITCH */

    /* Since the IF Switch is always installed in the frontend., no hardware
       check is performed. */

    /* Check if the submodule is in range */
    int currentIfSwitchModule = (CAN_ADDRESS & IF_SWITCH_MODULES_RCA_MASK) >> IF_SWITCH_MODULES_MASK_SHIFT;
    if (currentIfSwitchModule >= IF_SWITCH_MODULES_NUMBER) {
        storeError(ERR_IF_SWITCH, ERC_MODULE_RANGE);  // IF Switch submodule out of range
        CAN_STATUS = HARDW_RNG_ERR;                   // Notify incoming CAN message of error
        return;
    }

    /* Call the correct handler */
    (ifSwitchModulesHandler[currentIfSwitchModule])(currentIfSwitchModule);
}

/* Band select handler */
void bandSelectHandler(int currentIfSwitchModule) {
#ifdef DEBUG_IFSWITCH
    printf("  Band Select\n");
#endif /* DEBUG_IFSWITCH */

    /* If control (size!=0) */
    if (CAN_SIZE) {
        // save the incoming message:
        SAVE_LAST_CONTROL_MESSAGE(frontend.ifSwitch.lastBandSelect)

        /* Since the payload is just a byte, there is no need to conver the
           received data from the can message to any particular format, the
           data is already available in CAN_BYTE. */
        if (checkRange(BAND1, CAN_BYTE, BAND10)) {
            storeError(ERR_IF_SWITCH, ERC_COMMAND_VAL);  // Selected band set value out of range

            /* Store error in the last control message variable */
            frontend.ifSwitch.lastBandSelect.status = CON_ERROR_RNG;
            return;
        }

        /* Set the IF switch band. If an error occurs then store the state and
           return. */
        if (setIfSwitchBandSelect() == ERROR) {
            /* Store the error state in the last control message variable */
            frontend.ifSwitch.lastBandSelect.status = ERROR;
            return;
        }

        /* If everything went fine, it's a control message, we're done. */
        return;
    }

    /* If monitor on control RCA */
    if (currentClass == CONTROL_CLASS) {  // If monitor on control RCA
        // return the last control message and status
        RETURN_LAST_CONTROL_MESSAGE(frontend.ifSwitch.lastBandSelect)
        return;
    }

    /* If monitor on monitor RCA */
    /* This monitor point doesn't return an hardware status but just the
       current status that is stored in memory. The memory status is
       update when the state of the IF switch select band is changed by a
       control command. */
    CAN_BYTE = frontend.ifSwitch.bandSelect;
    CAN_SIZE = CAN_BYTE_SIZE;
}

void allChannelsHandler(int currentIfSwitchModule) {
#ifdef DEBUG_IFSWITCH
    printf("  All Channels Atten\n");
#endif /* DEBUG_IFSWITCH */

    /* If control (size!=0) */
    if (CAN_SIZE) {
        // save the incoming message:
        SAVE_LAST_CONTROL_MESSAGE(frontend.ifSwitch.lastAllChannelsAtten)

        // Set all four IF switch attenuators:
        for (int currentIfSwitchModule = 0; currentIfSwitchModule < IF_CHANNELS_NUMBER; currentIfSwitchModule++) {
            // Range check:
            if (checkRange(IF_CHANNEL_SET_ATTENUATION_MIN, CAN_DATA(currentIfSwitchModule),
                           IF_CHANNEL_SET_ATTENUATION_MAX)) {
                storeError(ERR_IF_SWITCH, ERC_COMMAND_VAL);  // Attenuation set value out of range

                /* Store error in the last control message variable */
                frontend.ifSwitch.lastAllChannelsAtten.status = CON_ERROR_RNG;
                return;
            }
            // Copy the attenuation for the current module being set to CAN_BYTE:
            CAN_BYTE = CAN_DATA(currentIfSwitchModule);

            // Set the current attenuator using CAN_BYTE:
            if (setIfChannelAttenuation(currentIfSwitchModule) == ERROR) {
                /* Store the Error state in the last control message variable */
                frontend.ifSwitch.lastAllChannelsAtten.status = ERROR;
                // bail out early if error:
                return;
            }
        }
        /* If everything went fine, it's a control message, we're done. */
        return;
    }

    /* If monitor on control RCA */
    if (currentClass == CONTROL_CLASS) {  // If monitor on control RCA
        // return the last control message and status
        RETURN_LAST_CONTROL_MESSAGE(frontend.ifSwitch.lastAllChannelsAtten)
        return;
    }

    /* If monitor on monitor RCA */
    for (int currentIfSwitchModule = 0; currentIfSwitchModule < IF_CHANNELS_NUMBER; currentIfSwitchModule++) {
        CAN_DATA(currentIfSwitchModule) = frontend.ifSwitch
                                              .ifChannel[currentIfChannelPolarization[currentIfSwitchModule]]
                                                        [currentIfChannelSideband[currentIfSwitchModule]]
                                              .attenuation;
    }
    CAN_SIZE = IF_CHANNELS_NUMBER;
}

/* IF Switch initialization */
/*! This function performs all the necessary initialization for the IF switch
    system. These are executed only once at startup.
    \return
        - \ref NO_ERROR -> if no error occurred
        - \ref ERROR    -> if something wrong happened */
int ifSwitchStartup(void) {
    /* Set the currentModule variable to reflect the fact that the IF switch is
       selected. This is necessary because currentModule is the global variable
       used to select the communication channel. This is only necessary if
       serial communication have to be implemented. */

#ifdef DEBUG_STARTUP
    printf(" Initializing IF Switch Module...\n");
    printf("  - Reading IF switch M&C module hardware revision level...\n");
#endif

    /* Call the getIfSwitchHadrwRevision() function to read the hardware
       revision level. If error, return error and abort initialization. */
    if (getIfSwitchHardwRevision() == ERROR) {
        return ERROR;
    }

#ifdef DEBUG_STARTUP
    printf("     Revision level: %d\n", frontend.ifSwitch.hardwRevision);
    printf("    done!\n");  // Hardware Revision Level
    printf(" done!\n\n");   // Initialization
#endif
    return NO_ERROR;
}
