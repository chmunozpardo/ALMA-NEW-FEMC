/*! \file   lnaLed.c
    \brief  LNA led functions

    <b> File information: </b><br>
    Created: 2004/08/24 16:24:39 by avaccari

    This file contains all the functions necessary to handle LNA led
    events. */

/* Includes */
#include <stdio.h>  /* printf */
#include <string.h> /* memcpy */

#include "biasSerialInterface.h"
#include "debug.h"
#include "error_local.h"
#include "frontend.h"

/* Globals */
/* Externs */
unsigned char currentLnaLedModule = 0;
/* Statics */
static HANDLER lnaLedModulesHandler[LNA_LED_MODULES_NUMBER] = {enableHandler};

/* LNA led handler */
/*! This function will be called by the CAN message handler when the received
    message is pertinent to the LNA led. */
void lnaLedHandler(void) {
    /* The value of currentLnaLedModule is not changed since there is only one
       submodule in the LNA led module.
       This structure is preserved only for consistency.
       If the timing should be an issue, it can be removed and the functionality
       can be placed directly in the lnaLedHandler function. */

#ifdef DEBUG
    printf("    LNA Led\n");
#endif /* DEBUG */

    /* Since the is only one submodule in the lna led, the check to see if the
       desired submodule is in range, is not needed and we can directly call
       the correct handler. */
    /* Call the correct handler */
    (lnaLedModulesHandler[currentLnaLedModule])();
}

/*  LNA led enable Handler */
static void enableHandler(void) {
#ifdef DEBUG
    printf("     LNA led enable\n");
#endif /* DEBUG */

    /* If contro (size !=0) */
    if (CAN_SIZE) {
        // save the incoming message:
        SAVE_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].polarization[currentBiasModule].lnaLed.lastEnable)

        // If we are in STANDBY2 mode, return HARDW_BLKD_ERR
        if (frontend.cartridge[currentModule].standby2) {
            /* Store the ERROR state in the last control message variable */
            frontend.cartridge[currentModule].polarization[currentBiasModule].lnaLed.lastEnable.status = HARDW_BLKD_ERR;

            return;
        }

        /* Change the status of the LNA led according to the content of the CAN
           message. */
        if (setLnaLedEnable(CAN_BYTE ? LNA_LED_ENABLE : LNA_LED_DISABLE) == ERROR) {
            /* Store the ERROR state in the last control message variable */
            frontend.cartridge[currentModule].polarization[currentBiasModule].lnaLed.lastEnable.status = ERROR;

            return;
        }

        /* If everything went fine, it's a control message, we're done. */
        return;
    }

    /* If monitor on a control RCA */
    if (currentClass == CONTROL_CLASS) {
        // return the last control message and status
        RETURN_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].polarization[currentBiasModule].lnaLed.lastEnable)
        return;
    }

    /* If monitor on a monitor RCA */
    /* This monitor point doesn't return an hardware status but just the
       current status that is stored in memory. The memory status is
       update when the state of the mixer bias mode is changed by a
       control command. */
    CAN_BYTE = frontend.cartridge[currentModule].polarization[currentBiasModule].lnaLed.enable;
    CAN_SIZE = CAN_BOOLEAN_SIZE;
}

// set the specified LNA LED to STANDBY2 mode
void lnaLedGoStandby2() {
    int ret;

#ifdef DEBUG_GO_STANDBY2
    printf(" - lnaLedGoStandby2 pol=%d\n", currentBiasModule);
#endif  // DEBUG_GO_STANDBY2

    // disable the LNA LED:
    ret = setLnaLedEnable(LNA_LED_DISABLE);

#ifdef DEBUG_GO_STANDBY2
    if (ret) printf(" -- ret=%d\n", ret);
#endif  // DEBUG_GO_STANDBY2
}
