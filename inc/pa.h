/*! \file       pa.h
    \ingroup    lo
    \brief      PA header file

    <b> File information: </b><br>
    Created: 2004/08/24 16:13:43 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the PA. */

/*! \defgroup   pa  Power Amplifier (PA)
    \ingroup    lo
    \brief      PA module

    This group includes all the different \ref pa submodules. For more
    information on the \ref pa module see \ref pa.h */

#ifndef _PA_H
#define _PA_H

/* Extra includes */
#include "globalDefinitions.h"
#include "paChannel.h"

/* Submodules definitions */
#define PA_MODULES_NUMBER 4  // See list below
#define PA_MODULES_RCA_MASK                                               \
    0x0000C                      /* Mask to extract the submodule number: \
                                    0-1 -> paChannelHandler               \
                                    2   -> supplyVoltage3VHandler         \
                                    3   -> supplyVoltage5VHandler */
#define PA_MODULES_MASK_SHIFT 2  // Bits right shift for the submodules mask

/* Typedefs */
//! Current state of the PA
typedef struct {
    //! A Channel current state
    /*! There is one power amplifier channel per polarization. */
    PA_CHANNEL paChannel[PA_CHANNELS_NUMBER];
    //! PA 3V supply voltage
    /*! This is the PA 3V supply voltage (in V). */
    float supplyVoltage3V;
    //! PA 5V supply voltage
    /*! This is the PA 5V supply voltage (in V). */
    float supplyVoltage5V;

    //! If true, we use an alternate method for handling the PA VD commands
    unsigned char hasTeledynePa;

    //! Control byte value to use for collector voltage control when Teledyne PA is operating
    unsigned char teledyneCollectorByte[2];

    //! Last control message: hasTeledynePA, teledyneCollectorByte:
    LAST_CONTROL_MESSAGE lastHasTeledynePa;
    LAST_CONTROL_MESSAGE lastTeledyneCollectorByte[2];
} PA;

/* Prototypes */
void paSupplyVoltage3VHandler(int currentModule, int currentPaModule);
void paSupplyVoltage5VHandler(int currentModule, int currentPaModule);
void paHandler(int currentModule);  //!< This function deals with the incoming can message

#endif /* _PA_H */
