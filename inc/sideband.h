/*! \file       sideband.h
    \ingroup    polarization
    \brief      Sideband header file

    <b> File information: </b><br>
    Created: 2004/08/24 16:24:39 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate one of the two sidebands available for each
    polarization. See \ref sideband for more information. */

/*! \defgroup   sideband    Sideband
    \ingroup    polarization
    \brief      Sideband module

    This group includes all the different \ref sideband submodules. For more
    information on the \ref sideband module see \ref sideband.h */

#ifndef _SIDEBAND_H
#define _SIDEBAND_H

/* Extra includes */
#include "lna.h"
#include "sis.h"
#include "sisMagnet.h"

/* Defines */
#define SIDEBANDS_NUMBER 2  //!< Number of sidebands per polarization
#define SIDEBAND0 0         //!< 0: Upper
#define SIDEBAND1 1         //!< 1: Lower

/* Submodules definitions */
#define SIDEBAND_MODULES_NUMBER 3  // See list below
#define SIDEBAND_MODULES_RCA_MASK                                               \
    0x00060                            /* Mask to extract the submodule number: \
                                          0 -> sis                              \
                                          1 -> sisMagnet                        \
                                          2 -> lna */
#define SIDEBAND_MODULES_MASK_SHIFT 5  // Bits right shift for the submodules mask

/* Typedefs */
//! Current state of the sideband
typedef struct {
    //! LNAcurrent state
    /*! Please see the definition of the \ref LNA structure for more
        information. */
    LNA lna;
    //! SIS mixer current state
    /*! Please see the definition of the \ref SIS structure for more
        information. */
    SIS sis;
    //! SIS magnetic coil current state
    /*! Please see the definition of the \ref SIS_MAGNET structure for more
        information. */
    SIS_MAGNET sisMagnet;
} SIDEBAND;

/* Prototypes */
/* Externs */
void sidebandHandler(int currentModule, int currentBiasModule,
                     int currentPolarizationModule);  //!< This function deals with the incoming CAN message

#endif /* _SIDEBAND_H */
