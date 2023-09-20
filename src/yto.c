/*! \file   yto.c
    \brief  YTO functions

    <b> File information: </b><br>
    Created: 2004/08/24 16:24:39 by avaccari

    This file contains all the functions necessary to handle YIG tuned
    oscillator events. */

/* Includes */
#include <stdio.h>  /* printf */
#include <string.h> /* memcpy */

#include "debug.h"
#include "error_local.h"
#include "frontend.h"
#include "loSerialInterface.h"

/* Globals */
/* Externs */
unsigned char currentYtoModule = 0;
/* Statics */
static HANDLER ytoModulesHandler[YTO_MODULES_NUMBER] = {ytoCoarseTuneHandler};

/* YIG tuned oscillator handler */
/*! This function will be called by the CAN message handler when the received
    message is pertinent to the YIG tuned oscillator. */
void ytoHandler(void) {
    /* The value of currentYtoModule is not changed since there is only one
       submodule in the YTO module.
       This structure is preserved only for consistency.
       If the timing should be an issue, it can be removed and the functionality
       can be placed directly in the ytoHandler function. */

#ifdef DEBUG
    printf("    YTO\n");
#endif /* DEBUG */

    /* Since the LO is always outfitted with all the modules, no hardware check
       is performed. */

    /* Since the is only one submodule in the yto, the check to see if the
       desired submodule is in range, is not needed and we can directly call the
       correct handler. */
    /* Call the correct handler */
    (ytoModulesHandler[currentYtoModule])();
}

/* Coarse Tune Handler */
static void ytoCoarseTuneHandler(void) {
    int ret = NO_ERROR;

#ifdef DEBUG
    printf("     Coarse Tune\n");
#endif /* DEBUG */

    /* If control (size !=0) */
    if (CAN_SIZE) {
        // save the incoming message:
        SAVE_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].lo.yto.lastYtoCoarseTune)

        /* Extract the unsigned int from the CAN message. */
        changeEndianInt(CONV_CHR_ADD, CAN_DATA_ADD);

        /* Check the value against the YTO limits. */
        if (checkRange(YTO_COARSE_SET_MIN, CONV_UINT(0), YTO_COARSE_SET_MAX)) {
            storeError(ERR_YTO, ERC_COMMAND_VAL);  // YTO coarse tune set value out of range

            /* Store the error in the last control message variable */
            frontend.cartridge[currentModule].lo.yto.lastYtoCoarseTune.status = CON_ERROR_RNG;

            return;
        }

        // if not in TROUBLESHOOTING mode, check that the LO PA setting is safe for the new YTO tuning:
        if (frontend.mode == TROUBLESHOOTING_MODE)
            ret = NO_ERROR;
        else
            ret = limitSafeYtoTuning();

        if (ret == HARDW_BLKD_ERR) {
            // report that the limit was violated:
            storeError(ERR_YTO, ERC_HARDWARE_BLOCKED);  // LO PA drain voltages were limited before YTO tuning
        }

        if (ret == ERROR) {
            // some other error.   Don't retune!
            frontend.cartridge[currentModule].lo.yto.lastYtoCoarseTune.status = ERROR;
            return;
        }

        /* Set the YTO coarse tune. If an error occurs then store the state and return. */
        if (setYtoCoarseTune() == ERROR) {
            /* Store the ERROR state in the last control message variable */
            frontend.cartridge[currentModule].lo.yto.lastYtoCoarseTune.status = ERROR;

            /* if limitSafeYtoTuning() above returned a problem, we want to save that error status */
        } else {
            frontend.cartridge[currentModule].lo.yto.lastYtoCoarseTune.status = ret;
        }

        /* If everything went fine, it's a control message, we're done. */
        return;
    }

    /* If monitor on control RCA */
    if (currentClass == CONTROL_CLASS) {
        // return the last control message and status
        RETURN_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].lo.yto.lastYtoCoarseTune)
        return;
    }

    /* If monitor on a monitor RCA */
    /* This monitor point doesn't return an hardware status but just the
       current status that is stored in memory. The memory status is
       update when the state of the YTO coarse tune is changed by a
       control command. */
    CONV_UINT(0) = frontend.cartridge[currentModule].lo.yto.ytoCoarseTune;

    /* Turn the bytes around! la-la-la-la-la-la */
    changeEndianInt(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_INT_SIZE;
}
