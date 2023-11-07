/*! \file   he2Press.c
    \brief  FETIM Compressor He2 Pressure Sensor

    <b> File information: </b><br>
    Created: 2011/03/28 17:34:50 by avaccari

    This file contains all the functions necessary to handle
    FETIM Compressor He2 Pressure Sensor events. */

/* Includes */
#include <stdio.h> /* printf */

#include "debug.h"
#include "error_local.h"
#include "fetimSerialInterface.h"
#include "frontend.h"

/* Statics */
static HANDLER he2PressModulesHandler[HE2_PRESS_MODULES_NUMBER] = {he2PressPressHandler, he2OutOfRangeHandler};

/* Compressor He2 Pressure Sensor Handler */
/*! This function will be called by the CAN message handler when the received
    message is in the address range of the compressor He2 Pressure sensor */
void he2PressHandler(int currentCompressorModule) {
#ifdef DEBUG_FETIM
    printf("   Compressor He2 Pressure Sensor\n");
#endif /* DEBUG_FETIM */

    /* Check if the specified submodule is in range */
    int currentHe2PressModule = (CAN_ADDRESS & HE2_PRESS_MODULES_RCA_MASK) >> HE2_PRESS_MODULES_MASK_SHIFT;
    if (currentHe2PressModule >= HE2_PRESS_MODULES_NUMBER) {
        storeError(ERR_COMP_HE2_PRESS, ERC_MODULE_RANGE);  // Submodule out of range
        CAN_STATUS = HARDW_RNG_ERR;                        // Notify incoming CAN message of error
        return;
    }

    /* Call the correct function */
    (he2PressModulesHandler[currentHe2PressModule])();

    return;
}

/* He2 Pressure handler */
/* This function return the current temperature of the addressed compressor He2
   pressure sensor */
void he2PressPressHandler(void) {
#ifdef DEBUG_FETIM
    printf("    He2 pressure\n");
#endif /* DEBUG_FETIM */

    /* If control (size !=0) store error and return. No control messages are
       allowed on this RCA. */
    if (CAN_SIZE) {
        storeError(ERR_COMP_HE2_PRESS, ERC_RCA_RANGE);  // Control message out of range
        return;
    }

    /* If monitor on control RCA return error since there are no control messages
       allowed on the RCA. */
    if (currentClass == CONTROL_CLASS) {                // If monitor on a control RCA
        storeError(ERR_COMP_HE2_PRESS, ERC_RCA_RANGE);  // Monitor message out of range
        /* Store the state in the outgoing CAN message */
        CAN_STATUS = MON_CAN_RNG;
        return;
    }

    /* Fetch the last async monitored pressure */
    if (asyncFetimHePressError == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
    }

    /* Store the last monitor value in the outgoing message */
    CONV_FLOAT = frontend.fetim.compressor.he2Press.pressure;

    /* Load the CAN message payload with the returned value and set the size.
       The value has to be converted from little endian (Intel) to big enadian
       (CAN). */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* He2 Pressure out of range handler */
/* This function return the current out of range status of the compressor He2
   pressure sensor */
void he2OutOfRangeHandler(void) {
#ifdef DEBUG_FETIM
    printf("    He2 pressure out of range status\n");
#endif /* DEBUG_FETIM */

    /* If control (size !=0) store error and return. No control messages are
       allowed on this RCA. */
    if (CAN_SIZE) {
        storeError(ERR_COMP_HE2_PRESS, ERC_RCA_RANGE);  // Control message out of range
        return;
    }

    /* If monitor on control RCA return error since there are no control messages
       allowed on the RCA. */
    if (currentClass == CONTROL_CLASS) {                // If monitor on a control RCA
        storeError(ERR_COMP_HE2_PRESS, ERC_RCA_RANGE);  // Monitor message out of range
        /* Store the state in the outgoing CAN message */
        CAN_STATUS = MON_CAN_RNG;
        return;
    }

    /* If Monitor on a Monitor RCA */
    /* Monitor Single Fail digital line */
    if (getFetimDigital(FETIM_DIG_HE2_PRESS_OOR, 0) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
    }

    /* Store the last monitored value in the outgoing message */
    CAN_BYTE = frontend.fetim.compressor.he2Press.pressOutRng;

    /* The CAN message payload is already loaded. Set the size */
    CAN_SIZE = CAN_BOOLEAN_SIZE;
}
