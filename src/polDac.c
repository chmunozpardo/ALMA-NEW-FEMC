/*! \file   polDac.c
    \brief  Polarization DAC functions

    <b> File information: </b><br>
    Created: 2004/08/24 16:24:39 by avaccari

    This file contains all the functions necessary to handle the polarization
    DAC events. */

/* Includes */
#include <stdio.h>  /* printf */
#include <string.h> /* memcpy */

#include "biasSerialInterface.h"
#include "debug.h"
#include "error_local.h"
#include "frontend.h"

/* Statics */
static HANDLER_INT_INT_INT_INT_INT polDacModulesHandler[POL_DAC_MODULES_NUMBER] = {polDacResetStrobeHandler,
                                                                                   polDacClearStrobeHandler};

/* Polarization DAC handler */
/*! This function will be called by the CAN message handler when the received
    message is pertinent to the polarization DAC. */
void polDacHandler(int currentModule, int currentBiasModule, int currentPolarizationModule,
                   int currentPolSpecialMsgsModule) {
#ifdef DEBUG
    printf("      Polarization DAC: %d\n", currentPolSpecialMsgsModule);
#endif /* DEBUG */

    /* No check is necessary on the existance of the hardware since, if the
       check on the polarization passed, the DACs are automatically installed. */

    /* Check if the submodule is in range */
    int currentPolDacModule = (CAN_ADDRESS & POL_DAC_MODULES_RCA_MASK) >> POL_DAC_MODULES_MASK_SHIFT;
    if (currentPolDacModule >= POL_DAC_MODULES_NUMBER) {
        storeError(ERR_POL_DAC, ERC_MODULE_RANGE);  // Polarization DAC submodule out of range
        CAN_STATUS = HARDW_RNG_ERR;
        return;
    }
    /* Call the correct handler */
    (polDacModulesHandler[currentPolDacModule])(currentModule, currentBiasModule, currentPolarizationModule,
                                                currentPolSpecialMsgsModule, currentPolDacModule);
}

/* Reset Strobe Handler */
void polDacResetStrobeHandler(int currentModule, int currentBiasModule, int currentPolarizationModule,
                              int currentPolSpecialMsgsModule, int currentPolDacModule) {
#ifdef DEBUG
    printf("       Reset Strobe\n");
#endif /* DEBUG */

    /* Check direction and perform the required operation */
    if (CAN_SIZE) {  // If control (size !=0)
        // save the incoming message:
        SAVE_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule]
                                      .polarization[currentBiasModule]
                                      .polSpecialMsgs.polDac[currentPolSpecialMsgsModule]
                                      .lastResetStrobe)

        /* Send the strobe */
        if (setBiasDacStrobe(currentModule, currentBiasModule, currentPolarizationModule, currentPolSpecialMsgsModule,
                             currentPolDacModule) == ERROR) {
            /* Store the ERROR state in the last control message variable */
            frontend.cartridge[currentModule]
                .polarization[currentBiasModule]
                .polSpecialMsgs.polDac[currentPolSpecialMsgsModule]
                .lastResetStrobe.status = ERROR;

            return;
        }
    }

    /* If it's a monitor message on a control RCA */
    if (currentClass == CONTROL_CLASS) {  // If monitor on a control RCA
        // return the last control message and status
        RETURN_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule]
                                        .polarization[currentBiasModule]
                                        .polSpecialMsgs.polDac[currentPolSpecialMsgsModule]
                                        .lastResetStrobe)
        return;
    }

    /* If monitor on monitor RCA: this should never happen because there are
       no monitor available for this particular device. In case though, return
       a CAN range error to avoid timeouts. */
    storeError(ERR_POL_DAC, ERC_RCA_RANGE);  // Monitor RCA out of range
    /* Store the state in the outgoing CAN message */
    CAN_STATUS = MON_CAN_RNG;
}

/* Clear Strobe Handler */
void polDacClearStrobeHandler(int currentModule, int currentBiasModule, int currentPolarizationModule,
                              int currentPolSpecialMsgsModule, int currentPolDacModule) {
#ifdef DEBUG
    printf("       Clear Strobe\n");
#endif /* DEBUG */

    /* Check if the submodule is in range: Only DAC1 is able to receive a clear
       strobe. */
    if (currentPolSpecialMsgsModule != POL_DAC_ALLOW_CLEAR_STROBE) {
        storeError(ERR_POL_DAC, ERC_RCA_RANGE);  // Polarization DAC doesn't accept clear strobe
        CAN_STATUS = HARDW_RNG_ERR;
        return;
    }

    /* Check direction and perform the required operation */
    if (CAN_SIZE) {  // If control (size !=0)
        // save the incoming message:
        SAVE_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule]
                                      .polarization[currentBiasModule]
                                      .polSpecialMsgs.polDac[currentPolSpecialMsgsModule]
                                      .lastClearStrobe)

        /* Send the strobe */
        if (setBiasDacStrobe(currentModule, currentBiasModule, currentPolarizationModule, currentPolSpecialMsgsModule,
                             currentPolDacModule) == ERROR) {
            /* Store the ERROR state in the last control message variable. */
            frontend.cartridge[currentModule]
                .polarization[currentBiasModule]
                .polSpecialMsgs.polDac[currentPolSpecialMsgsModule]
                .lastClearStrobe.status = ERROR;

            return;
        }
    }

    /* If it's a monitor message on a control RCA */
    if (currentClass == CONTROL_CLASS) {  // If monitor on a control RCA
        // return the last control message and status
        RETURN_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule]
                                        .polarization[currentBiasModule]
                                        .polSpecialMsgs.polDac[currentPolSpecialMsgsModule]
                                        .lastClearStrobe)
    }

    /* If monitor on monitor RCA: this should never happen because there are
       no monitor available for this particular device. In case though, return
       a CAN range error to avoid timeouts. */
    storeError(ERR_POL_DAC, ERC_RCA_RANGE);  // Monitor RCA out of range
    /* Store the state in the outgoing CAN message */
    CAN_STATUS = MON_CAN_RNG;
}
