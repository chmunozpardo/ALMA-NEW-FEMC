/*! \file   sideband.c
    \brief  Sideband functions

    <b> File information: </b><br>
    Created: 2004/08/24 16:24:39 by avaccari

    This file contains all the functions necessary to handle sideband events. */

/* Includes */
#include <stdio.h> /* printf */

#include "debug.h"
#include "error_local.h"
#include "frontend.h"

/* Statics */
static HANDLER_INT_INT_INT sidebandModulesHandler[SIDEBAND_MODULES_NUMBER] = {sisHandler, sisMagnetHandler, lnaHandler};

/* Sideband handler */
/*! This function will be called by the CAN message handling subroutine when the
    received message is pertinent to the sideband. */
void sidebandHandler(int currentModule, int currentBiasModule, int currentPolarizationModule) {
#ifdef DEBUG
    printf("    Sideband: %d (currentPolarizationModule)\n", currentPolarizationModule);
#endif /* DEBUG */

    /* Check if the submodule is in range */
    int currentSidebandModule = (CAN_ADDRESS & SIDEBAND_MODULES_RCA_MASK) >> SIDEBAND_MODULES_MASK_SHIFT;
    if (currentSidebandModule >= SIDEBAND_MODULES_NUMBER) {
        storeError(ERR_SIDEBAND, ERC_MODULE_RANGE);  // Sideband submodule out of range
        CAN_STATUS = HARDW_RNG_ERR;
        return;
    }

    /* Call the correct handler */
    (sidebandModulesHandler[currentSidebandModule])(currentModule, currentBiasModule, currentPolarizationModule);
}
