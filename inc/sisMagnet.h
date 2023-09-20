/*! \file       sisMagnet.h
    \ingroup    sideband
    \brief      SIS magnetic coils header file

    <b> File information: </b><br>
    Created: 2004/08/24 13:58:00 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the SIS magnetic coils used to kill the
    zero voltage Josephson currents in the SIS junctions. */

/*! \defgroup   sisMagnet SIS Magnetic Coil
    \ingroup    sideband
    \brief      SIS magnetic coil module
    \note       The \ref sisMagnet module doesn't include any submodule.

    For more information on this module see \ref sisMagnet.h */

#ifndef _SISMAGNET_H
#define _SISMAGNET_H

/* Extra includes */
#include "globalDefinitions.h"
#include "packet.h"

/* Submodules definitions */
#define SIS_MAGNET_MODULES_NUMBER 2  // See list below
#define SIS_MAGNET_MODULES_RCA_MASK                                               \
    0x00010                              /* Mask to extract the submodule number: \
                                            0 -> voltageHandler                   \
                                            1 -> currentHandler */
#define SIS_MAGNET_MODULES_MASK_SHIFT 4  // Bits right shift for the submodules mask

/* Typedefs */
//! Current state of the SIS magnetic coil
/*! This structure represent the current state of the SIS magnetic coil.
    \ingroup    sideband
    \param      available   This indicates the availability of the required
                            magnet:
                                - 0 -> Unavailable
                                - 1 -> Available
    \param      voltage     This contains the most recent read-back value
                            for the magnet voltage.
    \param      current     This contains the most recent read-back value
                            for the magnet current.
    \param      lastCurrent This contains a copy of the last issued control
                            message for the current. */
typedef struct {
    //! SIS magnet availability
    unsigned char available;
    //! SIS magnetic coil voltage
    /*! This is the voltage (in mV) across the magnetic coils. */
    float voltage;
    //! SIS magnetic coil current
    /*! This is the current (in mA) across the magnetic coils. */
    float current;
    //! Last control message: SIS magnetic coil current
    /*! This is the content of the last control message sent to the SIS
        magnetic coil current. */
    LAST_CONTROL_MESSAGE lastCurrent;
} SIS_MAGNET;

/* Globals */
/* Externs */
extern unsigned char currentSisMagnetModule;  //!< Current addressed SIS magnet submodule

/* Prototypes */
/* Statics */
static void voltageHandler(void);
static void currentHandler(void);
/* Externs */
extern void sisMagnetHandler(void);  //!< This function deals with the incoming can message

extern void sisMagnetGoStandby2();
//!< set the specified SIS magnet to STANDBY2 mode.

#endif /* _SISMAGNET_H */
