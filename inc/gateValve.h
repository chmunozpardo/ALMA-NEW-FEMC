/*! \file       gateValve.h
    \ingroup    cryostat
    \brief      Gate valve header file

    <b> File information: </b><br>
    Created: 2004/10/25 18:06:53 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the gate valve. */

/*! \defgroup   gateValve  Gate valve
    \ingroup    cryostat
    \brief      Gate valve module
    \note       The \ref gateValve module doesn't include any submodule.

    For more information on this module see \ref gateValve.h */

#ifndef _GATEVALVE_H
#define _GATEVALVE_H

/* Extra Includes */
#include "globalDefinitions.h"
#include "packet.h"

/* Submodules definitions */
#define GATE_VALVE_MODULES_NUMBER 1  // It's just the stateHandler

/* Typedefs */
//! Current state of the gate valve
/*! This structure represent the current state of the gate valve system.
    \ingroup    cryostat
    \param      state   This contains the current state of the gate
                            valve:
                                - \ref GATE_VALVE_OPEN -> Valve is open
                                - \ref GATE_VALVE_CLOSE -> Valve is close
                                - \ref GATE_VALVE_UNKNOWN -> Valve is in an
                                  unknown state.
                                - \ref GATE_VALVE_OVER_CURR -> Valve is
                                  stuck due to an overcurrent
                                - \ref GATE_VALVE_ERROR -> Valve is in error
                                  state
    \param      lastState   This contains a copy of the last issued control
                            message to the state of the gate valve. */
typedef struct {
    //! Gate valve state
    /*! This is the gate valve state as monitored through two limit switch.
        The possible states are:
            - \ref GATE_VALVE_OPEN -> Valve is open
            - \ref GATE_VALVE_CLOSE -> Valve is close
            - \ref GATE_VALVE_UNKNOWN -> Valve is in an unknown state
            - \ref GATE_VALVE_OVER_CURR -> Valve is stuck due to an overcurrent
            - \ref GATE_VALVE_ERROR -> Valve is in error state. */
    unsigned char state;
    //! Last control message: gate valve state
    /*! This is the content of the last control message sent to the gate
        valve state. */
    LAST_CONTROL_MESSAGE lastState;
} GATE_VALVE;

/* Prototypes */
void gatevalveStateHandler(void);
void gateValveHandler(int currentCryostatModule);  //!< This function deals with the incoming CAN message

#endif /* _GATEVALVE_H */
