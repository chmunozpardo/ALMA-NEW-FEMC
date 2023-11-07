/*! \file       sisHeater.h
    \ingroup    polarization
    \brief      SIS heater header file

    <b> File information: </b><br>
    Created: 2004/08/24 14:02:29 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the SIS heater. */

/*! \defgroup   sisHeater   SIS Heater
    \ingroup    polarization
    \brief      SIS heater module
    \note       The \ref sisHeater module doesn't include any submodule.

    For more information on this module see \ref sisHeater.h */

#ifndef _SISHEATER_H
#define _SISHEATER_H

/* Extra includes */
#include "globalDefinitions.h"
#include "packet.h"

/* Submodules definitions */
#define SIS_HEATER_MODULES_NUMBER 2  // See list below
#define SIS_HEATER_MODULES_RCA_MASK                                               \
    0x00040                              /* Mask to extract the submodule number: \
                                            0 -> enableHandler                    \
                                            1 -> currentHandler */
#define SIS_HEATER_MODULES_MASK_SHIFT 6  // Bits right shift for the submodules mask

/* Typedefs */
//! Current state of the SIS heater
/*! This structure represent the current state of the SIS heater.
    \ingroup    polarization
    \param      available   This indicates the availability of the rrequired
                            heater:
                                - \ref UNAVAILABLE -> Unavailable
                                - \ref AVAILABLE -> Available
    \param      enable  This contains the curren state of the lna. It
                            has to be remembered that this is \em not a read
                            back from the hardware but just a register
                            holding the last issued control:
                                - \ref SIS_HEATER_DISABLE -> OFF
                                - \ref SIS_HEATER_ENABLE -> ON
    \param      current This contains the most recent read-back value
                            for the heater current.
    \param      lastEnable  This contains a copy of the last issued control
                            message for the current. */
typedef struct {
    //! SIS heater availability
    unsigned char available;
    //! SIS heater state
    /*! This is the state of the SIS heater:
            - 0 -> disable (power up state)
            - 1 -> enable
        \warning    It is not a read back of the actual value. The returned
                    value is the one stored by the software after a control
                    command has been issued.*/
    unsigned char enable;
    //! SIS heater current
    /*! This is the current (in mA) across the SIS heater. */
    float current;
    //! Last control message: SIS heater state
    /*! This is the content of the last control message sent to the SIS
        heater state. */
    LAST_CONTROL_MESSAGE lastEnable;
} SIS_HEATER;

/* Prototypes */
void sisHeaterEnableHandler(int currentModule, int currentBiasModule, int currentPolarizationModule);
void sisHeaterCurrentHandler(int currentModule, int currentBiasModule, int currentPolarizationModule);
void sisHeaterHandler(int currentModule, int currentBiasModule,
                      int currentPolarizationModule);  //!< This function deals with the incoming CAN message

#endif /* _SISHEATER_H */
