/*! \file       opticalSwitch.h
    \ingroup    lpr
    \brief      Optical switch header file

    <b> File information: </b><br>
    Created: 2007/05/29 14:50:22 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the optical switch included in the lpr. */

/*! \defgroup   opticalSwitch   Optical Switch
    \ingroup    lpr
    \brief      Optical Switch module
    \note       The \ref opticalSwitch module doesn't include any submodule

    For mode information on this module see \ref opticalSwitch.h */

#ifndef _OPTICALSWITCH_H
#define _OPTICALSWITCH_H

/* Extra includes */
#include "globalDefinitions.h"
#include "packet.h"

/* Submodule definitions */
#define OPTICAL_SWITCH_MODULES_NUMBER 5  // See list below
#define OPTICAL_SWITCH_MODULES_RCA_MASK                                                 \
    0x0000F                                  /* Mask to extract the submodule number:   \
                                                0 -> portHandler                        \
                                                1 -> shutterHandler                     \
                                                2 -> forceShutterHandler (only control) \
                                                3 -> stateHandler                       \
                                                4 -> busyHandler */
#define OPTICAL_SWITCH_MODULES_MASK_SHIFT 1  // Bits right shift for the submodule mask

/* Typedefs */
//! Current state of the optical switch
/*! This structure represent the current state of the optical switch
    \ingroup    lpr
    \param      port        This contains the currently selected port of
                                the lpr. It has to be remembered that this
                                is \em not a read-back from the hardware but
                                just a register holding the last issued
                                control.
    \param      lastPort        This contains a copy of the last issued
                                control message to the port selection.
    \param      shutter     This contains the current state of the
                                shutter.
                                It has to be remembered that this is \em not
                                a read-back from the hardware but just a
                                register holding the last issued control:
                                    - \ref SHUTTER_ENABLE   -> Enable/ON
                                    - \ref SHUTTER_DISABLE  -> Disable/OFF
    \param      lastShutter     This contains a copy of the last issed
                                control message to the shutter.
    \param      lastForceShutter    This contains a copy of the last issed
                                    control message to the force shutter.
    \param      state       This contains the current error state for
                                the optical switch:
                                    - \ref NO_ERROR -> No error
                                    - \ref ERROR    -> Error
    \param      busy        This contains the current busy state for the
                                oprical switch:
                                    - \ref SWITCH_BUSY  -> Busy
                                    - \ref SWITCH_IDLE  -> Idle */
typedef struct {
    //! Port
    /*! This is the currently selected port:
        \warning    It is not a read back of the actual value. The returned
                    value is the one stored by the software after a control
                    command has been issued. */
    unsigned char port;
    //! Last control message: port
    /*! This is the content of the last control message sent to the port
        select of the optical switch. */
    LAST_CONTROL_MESSAGE lastPort;
    //! Shutter
    /*! This is the current state of the shutter:
            - \ref SHUTTER_ENABLE   -> Enable/ON
            - \ref SHUTTER_DISABLE  -> Disable/OFF
        \warning    It is not a read back of the actual value. The returned
                    value is the one stored by the software after a control
                    command has been issued. */
    unsigned char shutter;
    //! Last control message: shutter
    /*! This is the content of the last control message sent to the shutter
        of the optical switch. */
    LAST_CONTROL_MESSAGE lastShutter;
    //! Last control message: forceShutter
    /*! This is the content of the last control message sent to the force
        shutter of the optical switch. */
    LAST_CONTROL_MESSAGE lastForceShutter;
    //! Optical switch error state
    /*! This is the error state of the optical switch:
            - \ref NO_ERROR -> no error
            - \ref ERROR -> error */
    unsigned char state;
    //! Optical switch busy state
    /*! This is the busy state of the optical switch:
            - \ref SWITCH_BUSY  -> Busy
            - \ref SWITCH_IDLE  -> Idle */
    unsigned char busy;
} OPTICAL_SWITCH;

/* Globals */
/* Externs */
extern unsigned char currentOpticalSwitchModule;  //!< Currently addressed optical switch module

/* Prototypes */
/* Statics */
static void portHandler(void);
static void shutterHandler(void);
static void forceShutterHandler(void);
static void stateHandler(void);
static void busyHandler(void);
/* Externs */
extern void opticalSwitchHandler(void);  //!< This function deals with the incoming CAN messages

#endif /* _OPTICALSWITCH_H */
