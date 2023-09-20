/*! \file   lpr.c
    \brief  LPR functions

    <b> File information: </b><br>
    Created: 2007/06/02 17:01:27 by avaccari

    This file contains all the functions necessary to handle LPR events. */

/* Includes */
#include <stdio.h>  /* printf */
#include <string.h> /* strcpy */

#include "debug.h"
#include "error_local.h"
#include "frontend.h"
#include "iniWrapper.h"
#include "lprSerialInterface.h"
#include "packet.h"
#include "serialInterface.h"
#include "timer.h"

/* Globals */
/* Externs */
unsigned char currentLprModule = 0;
/* Statics */
static HANDLER lprModulesHandler[LPR_MODULES_NUMBER] = {lprTempHandler, lprTempHandler, opticalSwitchHandler,
                                                        edfaHandler};

/* LPR handler */
/*! This function will be called by the CAN message handler when the received
    message is pertinent to the LPR. */
void lprHandler(void) {
#ifdef DEBUG_LPR
    printf(" LPR\n");
#endif /* DEBUG_LPR */

    /* Since the receiver is always outfitted with a LPR, ho hadrware check is
       performed. */

    /* Check if the submodule is in range */
    currentLprModule = (CAN_ADDRESS & LPR_MODULES_RCA_MASK) >> LPR_MODULES_MASK_SHIFT;
    if (currentLprModule >= LPR_MODULES_NUMBER) {
        storeError(ERR_LPR, ERC_MODULE_RANGE);  // LPR submodule out of range

        CAN_STATUS = HARDW_RNG_ERR;  // Notify incoming CAN message of error
        return;
    }

    /* Call the correct handler */
    (lprModulesHandler[currentLprModule])();
}

/* LPR Startup */
/*! This function performs the operations necessary to initialize the LPR. This
    are performed only once at startup.
    \return
        - \ref NO_ERROR -> if no error occurred
        - \ref ERROR    -> if something wrong happened */
int lprStartup(void) {
    /* A variable to keep track of the timer */
    int timedOut;

#ifdef CHECK_HW_AVAIL
    CFG_STRUCT dataIn;
#endif

#ifndef CHECK_HW_AVAIL
    strcpy(frontend.lpr.configFile, "LPR.INI");
#else
    /* Configure the read array */
    dataIn.Name = LPR_CONF_FILE_KEY;
    dataIn.VarType = Cfg_String;
    dataIn.DataPtr = frontend.lpr.configFile;

    /* Access configuration file, if error, return skip the configuration. */
    if (myReadCfg(FRONTEND_CONF_FILE, LPR_CONF_FILE_SECTION, &dataIn, LPR_CONF_FILE_EXPECTED) != NO_ERROR) {
        return NO_ERROR;
    }

    /* Print config file */
    printf("%s\n", frontend.lpr.configFile);
#endif

    // No longer loading from INI file:
    frontend.lpr.edfa.photoDetector.coeff = LPR_ADC_EDFA_PD_POWER_COEFF_DFLT;

    /* Set the currentModule variable to reflect the fact that the LPR is
       selected. This is necessary because currentModule is the global variable
       used to select the communication channel. This is only necessary if
       serial communication have to be implemented. */
    currentModule = LPR_MODULE;
    if (serialAccess(LPR_10MHZ_MODE, NULL, LPR_10MHZ_MODE_SIZE, LPR_10MHZ_MODE_SHIFT_SIZE, LPR_10MHZ_MODE_SHIFT_DIR,
                     SERIAL_WRITE) == ERROR) {
        return ERROR;
    }
    frontend.lpr.ssi10MHzEnable = ENABLE;

    /* Load the CAN float to 0.0V */
    CONV_FLOAT = 0.0;
    /* Call the setModulationInputValue() function to set the value in
       hardware. If error, return error and abort initialization. */
    if (setModulationInputValue() == ERROR) {
        return ERROR;
    }

    /* Setup for 5 seconds and start the asynchornous timer */
    if (startAsyncTimer(TIMER_LPR_SWITCH_RDY, TIMER_LPR_TO_SWITCH_RDY, FALSE) == ERROR) {
        return ERROR;
    }

    /* Try standard shutter for 5 sec. The standard shutter waits for the
       optical switch to be ready. */
    do {
        timedOut = queryAsyncTimer(TIMER_LPR_SWITCH_RDY);
        if (timedOut == ERROR) {
            return ERROR;
        }
    } while ((setOpticalSwitchShutter(STANDARD) == ERROR) && (timedOut == TIMER_RUNNING));

    /* If the timer has expired, signal the error and force the shutter */
    if (timedOut == TIMER_EXPIRED) {
        storeError(ERR_OPTICAL_SWITCH,
                   ERC_HARDWARE_TIMEOUT);  // Time out while waiting for ready
                                           // state during initialization

        /* Force the shutter mode. If error, return error and abort
           initialization. */
        printf(" LPR  - Set optical switch: Forcing shutter mode...\n");
        if (setOpticalSwitchShutter(FORCED) == ERROR) {
            return ERROR;
        }

    } else {
        /* Stop the timer */
        if (stopAsyncTimer(TIMER_LPR_SWITCH_RDY) == ERROR) {
            return ERROR;
        }
    }

    /* If everyting went fine, update the optical switch port to reflect
       the fact that the shutter is enabled. */
    frontend.lpr.opticalSwitch.port = PORT_SHUTTERED;

    return NO_ERROR;
}

/* LPR Stop */
/*! This function performs the operations necessary to shutdown the LPR. This
    are performed only once at shutdown.
    \return
        - \ref NO_ERROR -> if no error occurred
        - \ref ERROR    -> if something wrong happened */
int lprStop(void) {
    /* A variable to keep track of the timer */
    int timedOut;

    /* Set the currentModule variable to reflect the fact that the LPR is
       selected. This is necessary because currentModule is the global variable
       used to select the communication channel. This is only necessary if
       serial communication have to be implemented. */
    currentModule = LPR_MODULE;

    /* Load the CAN float to 0.0V */
    CONV_FLOAT = 0.0;
    /* Call the setModulationInputValue() function to set the value in
       hardware. If error, return error and abort initialization. */
    if (setModulationInputValue() == ERROR) {
        return ERROR;
    }

    /* Setup for 5 seconds and start the asynchornous timer */
    if (startAsyncTimer(TIMER_LPR_SWITCH_RDY, TIMER_LPR_TO_SWITCH_RDY, FALSE) == ERROR) {
        return ERROR;
    }

    /* Try standard shutter for 5 sec. The standard shutter waits for the
       optical switch to be ready. */
    do {
        timedOut = queryAsyncTimer(TIMER_LPR_SWITCH_RDY);
        if (timedOut == ERROR) {
            return ERROR;
        }
    } while ((setOpticalSwitchShutter(STANDARD) == ERROR) && (timedOut == TIMER_RUNNING));

    /* If the timer has expired, signal the error and force the shutter */
    if (timedOut == TIMER_EXPIRED) {
        storeError(ERR_OPTICAL_SWITCH,
                   ERC_HARDWARE_TIMEOUT);  // Time out while waiting for ready
                                           // state during initialization

        /* Force the shutter mode. If error, return error and abort
           initialization. */
        printf(" LPR  - Set optical switch: Forcing shutter mode...\n");
        if (setOpticalSwitchShutter(FORCED) == ERROR) {
            return ERROR;
        }
    } else {
        /* Stop the timer */
        if (stopAsyncTimer(TIMER_LPR_SWITCH_RDY) == ERROR) {
            return ERROR;
        }
    }

    /* If everyting went fine, update the optical switch port to reflect
       the fact that the shutter is enabled. */
    frontend.lpr.opticalSwitch.port = PORT_SHUTTERED;

    return NO_ERROR;
}
