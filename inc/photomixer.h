/*! \file       photomixer.h
    \ingroup    lo
    \brief      LO photomixer header file

    <b> File information: </b><br>
    Created: 2004/08/24 16:05:40 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the photomixer included in the LO. */

/*! \defgroup   photomixer  LO Photomixer
    \ingroup    lo
    \brief      LO Photomixer  module
    \note       The \ref photomixer module doesn't include any submodule.

    For more information on this module see \ref photomixer.h */

#ifndef _PHOTOMIXER_H
#define _PHOTOMIXER_H

/* Extra includes */
#include "globalDefinitions.h"
#include "packet.h"

/* Submodules definitions */
#define PHOTOMIXER_MODULES_NUMBER 3  // See list below
#define PHOTOMIXER_MODULES_RCA_MASK                                               \
    0x0000C                              /* Mask to extract the submodule number: \
                                            0 -> enable                           \
                                            1 -> voltage                          \
                                            2 -> current */
#define PHOTOMIXER_MODULES_MASK_SHIFT 2  // Bits right shift for the submodules mask

/* Typedefs */
//! Current state of the LO photomixer
/*! This structure represent the current state of the LO photomixer.
    \ingroup    lo
    \param      enable  This contains the current state of the
                            photomixer. It has to be stored since this is
                            \em not a read-back from the hardware but just
                            a register holding the last succesfully issued
                            control:
                                - 0 -> Disable/OFF
                                - 1 -> Enable/ON
    \param      voltage     This contains the most recent read-back value of
                            the mixer bias voltage.
    \param      current     This contains the most recent read-back value of
                            the mixer bias current.
    \param      lastEnable  This contains a copy of the last issued control
                            message for the enable. */
typedef struct {
    //! LO photomixer state
    /*! This is the state of the LO photomixer:\n
            - 0 -> OFF (power up state)\n
            - 1 -> ON
        \warning    It is not a read back of the actual value. The returned
                    value is the one stored by the software after a control
                    command has been issued.*/
    unsigned char enable;
    //! LO photomixer voltage
    /*! This is the voltage (in V) across the LO photomixer. */
    float voltage;
    //! LO photomixer current
    /*! This is the current (in mA) across the LO photomixer. */
    float current;
    //! Last control message: LO photomixer state
    /*! This is the content of the last control message sent to the LO
        photomixer state. */
    LAST_CONTROL_MESSAGE lastEnable;
} PHOTOMIXER;

/* Prototypes */
void pmxEnableHandler(int currentModule);
void pmxVoltageHandler(int currentModule);
void pmxCurrentHandler(int currentModule);
void photomixerHandler(int currentModule);  //!< This function deals with the incoming can message

#endif /* _PHOTOMIXER_H */
