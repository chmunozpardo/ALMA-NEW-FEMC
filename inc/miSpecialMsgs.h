/*! \file       miSpecialMsgs.h
    \ingroup    specialMsgs
    \ingroup    modulationInput
    \brief      Modulation Input special messages header file

    <b> File information: </b><br>
    Created: 2007/06/22 16:40:01 by avaccari

    This file contains all the information necessary to define the
    characteristics and handle the modulation input special messages. */

/*! \defgroup   miSpecialMsgs  Polarization special messages
    \ingroup    specialMsgs
    \ingroup    modulationInput
    \brief      Modulation Input special messages module

    This group includes all the different \ref miSpecialMsgs submodules. For
    more information on the \ref miSpecialMsgs module see \ref miSpecialMsgs.h */

#ifndef _MISPECIALMSGS_H
#define _MISPECIALMSGS_H

/* Extra includes */
#include "miDac.h"

/* Submodules definitions */
#define MI_SPECIAL_MSGS_MODULES_NUMBER 1  // Only one DAC

/* Typedefs */
//! Current state of the modulation input special messages
/*! This structure represent the current state of the modulation input
    special messages.
    \ingroup    modulationInput
    \param      This contains the information about the DAC available for
                this module. There can be up to \ref MI_DAC_NUMBER for
                each special messages module. */
typedef struct {
    //! Polarization dac current state
    /*! Please see the definition of the \ref MI_DAC structure for more
        information.*/
    MI_DAC miDac;
} MI_SPECIAL_MSGS;

/* Prototypes */
void miSpecialMsgsHandler(void);  //!< This function deals with the incoming can message

#endif /* _MISPECIALMSGS_H */
