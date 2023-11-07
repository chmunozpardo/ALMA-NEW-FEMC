/*! \file   amc.c
    \brief  MC functions

    <b> File information: </b><br>
    Created: 2004/08/24 16:24:39 by avaccari

    This file contains all the functions necessary to handle the MC events. */

/* Includes */
#include <stdio.h>  /* printf */
#include <string.h> /* memcpy */

#include "debug.h"
#include "error_local.h"
#include "frontend.h"
#include "loSerialInterface.h"

/* Statics */
static HANDLER_INT amcModulesHandler[AMC_MODULES_NUMBER] = {
    gateAVoltageHandler,  drainAVoltageHandler, drainACurrentHandler,      gateBVoltageHandler,
    drainBVoltageHandler, drainBCurrentHandler, multiplierDVoltageHandler, gateEVoltageHandler,
    drainEVoltageHandler, drainECurrentHandler, multiplierDCurrentHandler, amcSupplyVoltage5VHandler};

/* AMC handler */
/*! This function will be called by the CAN message handler when the received
    message is pertinent to the MC. */
void amcHandler(int currentModule) {
#ifdef DEBUG
    printf("    AMC\n");
#endif /* DEBUG */

    /* Since the LO is always outfitted with all the modules, no hardware check
       is performed. */

    /* Check if the submodule is in range */
    int currentAmcModule = (CAN_ADDRESS & AMC_MODULES_RCA_MASK);
    if (currentAmcModule >= AMC_MODULES_NUMBER) {
        storeError(ERR_AMC, ERC_MODULE_RANGE);  // AMC submodule out of range

        CAN_STATUS = HARDW_RNG_ERR;  // Notify incoming CAN message of the error
        return;
    }
    /* Call the correct handler */
    (amcModulesHandler[currentAmcModule])(currentModule);
}

/* Gate A Voltage Handler */
/* This function deals with all the monitor requests directed to the AMC gate A
   voltage. There are no control messages allowed for the gate A voltage. */
void gateAVoltageHandler(int currentModule) {
#ifdef DEBUG
    printf("     Gate A Voltage\n");
#endif /* DEBUG */

    /* If control (size !=0) store error and return. No control message are
       allowed on this RCA. */
    if (CAN_SIZE) {
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Control message out of range
        return;
    }

    /* If monitor on control RCA return error since there are no control
       messages allowed on this RCA. */
    if (currentClass == CONTROL_CLASS) {     // If monitor on control RCA
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Monitor message out of range
        /* Store the state in the outgoing CAN message */
        CAN_STATUS = MON_CAN_RNG;

        return;
    }

    /* Monitor the amc A gate voltage */
    if (getAmc(AMC_GATE_A_VOLTAGE, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.gateAVoltage;
    } else {
        /* If no error during monitor process, gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.gateAVoltage;
    }

    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* Drain A Voltage Handler */
/* This function deals with all the monitor requests directed to the AMC drain A
   voltage. There are no control messages allowed for the drain A voltage. */
void drainAVoltageHandler(int currentModule) {
#ifdef DEBUG
    printf("     Drain A Voltage\n");
#endif /* DEBUG */

    /* If control (size !=0) store error and return. No control message are
       allowed on this RCA. */
    if (CAN_SIZE) {
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Control message out of range
        return;
    }

    /* If monitor on control RCA return error since there are no control
       messages allowed on this RCA. */
    if (currentClass == CONTROL_CLASS) {     // If monitor on control RCA
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Monitor message out of range
        /* Store the state in the outgoing CAN message */
        CAN_STATUS = MON_CAN_RNG;
        return;
    }

    /* Monitor the amc A drain voltage */
    if (getAmc(AMC_DRAIN_A_VOLTAGE, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainAVoltage;
    } else {
        /* If no error during monitor process, gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainAVoltage;
    }

    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* Drain A Current Handler */
/* This function deals with all the monitor requests directed to the AMC drain A
   current. There are no control messages allowed for the drain A current. */
void drainACurrentHandler(int currentModule) {
#ifdef DEBUG
    printf("     Drain A Current\n");
#endif /* DEBUG */

    /* If control (size !=0) store error and return. No control message are
       allowed on this RCA. */
    if (CAN_SIZE) {
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Control message out of range
        return;
    }

    /* If monitor on control RCA return error since there are no control
       messages allowed on this RCA. */
    if (currentClass == CONTROL_CLASS) {     // If monitor on control RCA
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Monitor message out of range
        /* Store the state in the outgoing CAN message */
        CAN_STATUS = MON_CAN_RNG;

        return;
    }

    /* Monitor the AMC drain A current */
    if (getAmc(AMC_DRAIN_A_CURRENT, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainACurrent;
    } else {
        /* If no error during monitor process, gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainACurrent;
    }

    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* Gate B Voltage Handler */
/* This function deals with all the monitor requests directed to the AMC gate B
   voltage. There are no control messages allowed for the gate B voltage. */
void gateBVoltageHandler(int currentModule) {
#ifdef DEBUG
    printf("     Gate B Voltage\n");
#endif /* DEBUG */

    /* If control (size !=0) store error and return. No control message are
       allowed on this RCA. */
    if (CAN_SIZE) {
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Control message out of range
        return;
    }

    /* If monitor on control RCA return error since there are no control
       messages allowed on this RCA. */
    if (currentClass == CONTROL_CLASS) {     // If monitor on control RCA
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Monitor message out of range
        /* Store the state in the outgoing CAN message */
        CAN_STATUS = MON_CAN_RNG;

        return;
    }

    /* Monitor the amc B gate voltage */
    if (getAmc(AMC_GATE_B_VOLTAGE, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.gateBVoltage;
    } else {
        /* If no error during monitor process, gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.gateBVoltage;
    }

    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* Drain B Voltage Handler */
/* This function will deal with monitor and control requests to the drain b
   voltage. */
void drainBVoltageHandler(int currentModule) {
#ifdef DEBUG
    printf("     Drain B Voltage\n");
#endif /* DEBUG */

    /* If control (size !=0) */
    if (CAN_SIZE) {
        // save the incoming message:
        SAVE_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].lo.amc.lastDrainBVoltage)

        /* Extract the float from the can message */
        changeEndian(CONV_CHR_ADD, CAN_DATA_ADD);

        /* Set the AMC drain B voltage. If an error occurs then store the state
           and return the error state then return. */
        if (setAmc(AMC_DRAIN_B_VOLTAGE, currentModule) == ERROR) {
            /* Store the ERROR state in the last control message variable */
            frontend.cartridge[currentModule].lo.amc.lastDrainBVoltage.status = ERROR;

            return;
        }

        /* If everything went fine, it's a control message, we're done. */
        return;
    }

    /* If monitor on control RCA */
    if (currentClass == CONTROL_CLASS) {  // If monitor on a control RCA
        // return the last control message and status
        RETURN_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].lo.amc.lastDrainBVoltage)

        return;
    }

    /* If monitor on a monitor RCA */
    /* Monitor the AMC drain B voltage */
    if (getAmc(AMC_DRAIN_B_VOLTAGE, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainBVoltage;
    } else {
        /* If no error during the monitor process gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainBVoltage;
    }
    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* Drain B Current Handler */
/* This function deals with all the monitor requests directed to the AMC drain B
   current. There are no control messages allowed for the drain B current. */
void drainBCurrentHandler(int currentModule) {
#ifdef DEBUG
    printf("     Drain B Current\n");
#endif /* DEBUG */

    /* If control (size !=0) store error and return. No control message are
       allowed on this RCA. */
    if (CAN_SIZE) {
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Control message out of range
        return;
    }

    /* If monitor on control RCA return error since there are no control
       messages allowed on this RCA. */
    if (currentClass == CONTROL_CLASS) {     // If monitor on control RCA
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Monitor message out of range
        /* Store the state in the outgoing CAN message */
        CAN_STATUS = MON_CAN_RNG;

        return;
    }

    /* Monitor the AMC drain B current */
    if (getAmc(AMC_DRAIN_B_CURRENT, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainBCurrent;
    } else {
        /* If no error during monitor process, gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainBCurrent;
    }

    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* Multiplier D Voltage Handler */
/* This function will deal with monitor and control requests to the multiplier
   D voltage. */
void multiplierDVoltageHandler(int currentModule) {
#ifdef DEBUG
    printf("     Multiplier D Voltage\n");
#endif /* DEBUG */

    /* If control (size !=0) */
    if (CAN_SIZE) {
        // save the incoming message:
        SAVE_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].lo.amc.lastMultiplierDVoltage);
        /* Set the AMC multiplier D voltage. If an error occurs then store the state
           and return the error state then return. */
        if (setAmc(AMC_MULTIPLIER_D_VOLTAGE, currentModule) == ERROR) {
            /* Store the ERROR state in the last control message variable */
            frontend.cartridge[currentModule].lo.amc.lastMultiplierDVoltage.status = ERROR;
            return;
        }

        /* If everything went fine, it's a control message, we're done. */
        return;
    }

    /* If monitor on control RCA */
    if (currentClass == CONTROL_CLASS) {  // If monitor on a control RCA
        // return the last control message and status
        RETURN_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].lo.amc.lastMultiplierDVoltage)
        return;
    }

    /* If monitor on a monitor RCA */
    /* This monitor point doesn't return an hardware status but just the
       current status that is stored in memory. The memory status is
       update when the state of the multiplier D voltage tune is changed by a
       control command. */
    CAN_BYTE = frontend.cartridge[currentModule].lo.amc.multiplierDVoltage;

    CAN_SIZE = CAN_BYTE_SIZE;
}

/* Multiplier D Current Handler */
/* This function will deal with monitor requests to the multiplier D current. */
void multiplierDCurrentHandler(int currentModule) {
#ifdef DEBUG
    printf("     Multiplier D Current\n");
#endif /* DEBUG */

    /* If control (size !=0) store error and return. No control message are
       allowed on this RCA. */
    if (CAN_SIZE) {
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Control message out of range
        return;
    }

    /* If monitor on control RCA return error since there are no control
       messages allowed on this RCA. */
    if (currentClass == CONTROL_CLASS) {     // If monitor on control RCA
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Monitor message out of range
        /* Store the state in the outgoing CAN message */
        CAN_STATUS = MON_CAN_RNG;

        return;
    }

    /* If monitor on a monitor RCA */
    /* Monitor the AMC multiplier D voltage */
    if (getAmc(AMC_MULTIPLIER_D_CURRENT, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.multiplierDCurrent;
    } else {
        /* If no error during the monitor process gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.multiplierDCurrent;
    }
    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* Gate E Voltage Handler */
/* This function will deal with monitor and control requests to the gate E
   voltage. */
void gateEVoltageHandler(int currentModule) {
#ifdef DEBUG
    printf("     Gate E Voltage\n");
#endif /* DEBUG */

    /* If control (size !=0) */
    if (CAN_SIZE) {
        // save the incoming message:
        SAVE_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].lo.amc.lastGateEVoltage);

        /* Extract the float from the can message */
        changeEndian(CONV_CHR_ADD, CAN_DATA_ADD);

        /* Set the AMC gate E voltage. If an error occurs then store the state
           and return the error state then return. */
        if (setAmc(AMC_GATE_E_VOLTAGE, currentModule) == ERROR) {
            /* Store the ERROR state in the last control message variable */
            frontend.cartridge[currentModule].lo.amc.lastGateEVoltage.status = ERROR;
            return;
        }

        /* If everything went fine, it's a control message, we're done. */
        return;
    }

    /* If monitor on control RCA */
    if (currentClass == CONTROL_CLASS) {  // If monitor on a control RCA
        // return the last control message and status
        RETURN_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].lo.amc.lastGateEVoltage)
        return;
    }

    /* If monitor on a monitor RCA */
    /* Monitor the AMC gate E voltage */
    if (getAmc(AMC_GATE_E_VOLTAGE, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.gateEVoltage;
    } else {
        /* If no error during the monitor process gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.gateEVoltage;
    }
    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* Drain E Voltage Handler */
/* This function will deal with monitor and control requests to the gate E
   voltage. */
void drainEVoltageHandler(int currentModule) {
#ifdef DEBUG
    printf("     Drain E Voltage\n");
#endif /* DEBUG */

    /* If control (size !=0) */
    if (CAN_SIZE) {
        // save the incoming message:
        SAVE_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].lo.amc.lastDrainEVoltage)

        /* Extract the float from the can message */
        changeEndian(CONV_CHR_ADD, CAN_DATA_ADD);

        /* Set the AMC drain E voltage. If an error occurs then store the state
           and return the error state then return. */
        if (setAmc(AMC_DRAIN_E_VOLTAGE, currentModule) == ERROR) {
            /* Store the ERROR state in the last control message variable */
            frontend.cartridge[currentModule].lo.amc.lastDrainEVoltage.status = ERROR;

            return;
        }

        /* If everything went fine, it's a control message, we're done. */
        return;
    }

    /* If monitor on control RCA */
    if (currentClass == CONTROL_CLASS) {  // If monitor on a control RCA
        // return the last control message and status
        RETURN_LAST_CONTROL_MESSAGE(frontend.cartridge[currentModule].lo.amc.lastDrainEVoltage)
        return;
    }

    /* If monitor on a monitor RCA */
    /* Monitor the AMC drain E voltage */
    if (getAmc(AMC_DRAIN_E_VOLTAGE, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainEVoltage;
    } else {
        /* If no error during the monitor process gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainEVoltage;
    }
    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* Drain E Current Handler */
/* This function deals with all the monitor requests directed to the AMC drain E
   current. There are no control messages allowed for the drain E current. */
void drainECurrentHandler(int currentModule) {
#ifdef DEBUG
    printf("     Drain E Current\n");
#endif /* DEBUG */

    /* If control (size !=0) store error and return. No control message are
       allowed on this RCA. */
    if (CAN_SIZE) {
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Control message out of range
        return;
    }

    /* If monitor on control RCA return error since there are no control
       messages allowed on this RCA. */
    if (currentClass == CONTROL_CLASS) {     // If monitor on control RCA
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Monitor message out of range
        /* Store the state in the outgoing CAN message */
        CAN_STATUS = MON_CAN_RNG;

        return;
    }

    /* Monitor the AMC drain E current */
    if (getAmc(AMC_DRAIN_E_CURRENT, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainECurrent;
    } else {
        /* If no error during monitor process, gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.drainECurrent;
    }

    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}

/* Supply Voltage 5V Handler */
/* This function deals with all the monitor requests directed to the AMC 5V
   supply voltage. There are no control messages allowed for the 5V supply
   voltage. */
void amcSupplyVoltage5VHandler(int currentModule) {
#ifdef DEBUG
    printf("     Supply Voltage 5V\n");
#endif /* DEBUG */

    /* If control (size !=0) store error and return. No control message are
       allowed on this RCA. */
    if (CAN_SIZE) {
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Control message out of range
        return;
    }

    /* If monitor on control RCA return error since there are no control
       messages allowed on this RCA. */
    if (currentClass == CONTROL_CLASS) {     // If monitor on control RCA
        storeError(ERR_AMC, ERC_RCA_RANGE);  // Monitor message out of range
        /* Store the state in the outgoing CAN message */
        CAN_STATUS = MON_CAN_RNG;

        return;
    }

    /* Monitor the AMC 5V supply voltage */
    if (getAmc(AMC_5V_SUPPLY_VOLTAGE, currentModule) == ERROR) {
        /* If error during monitoring, store the ERROR state in the outgoing
           CAN message state. */
        CAN_STATUS = ERROR;
        /* Store the last known value in the outgoing message */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.supplyVoltage5V;
    } else {
        /* If no error during monitor process, gather the stored data */
        CONV_FLOAT = frontend.cartridge[currentModule].lo.amc.supplyVoltage5V;
    }

    /* Load the CAN message payload with the returned value and set the
       size. The value has to be converted from little endian (Intel) to
       big endian (CAN). It is done directly instead of using a function
       to save some time. */
    changeEndian(CAN_DATA_ADD, CONV_CHR_ADD);
    CAN_SIZE = CAN_FLOAT_SIZE;
}
