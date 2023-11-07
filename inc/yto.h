/*! \file       yto.h
    \ingroup    lo
    \brief      YTO header file

    <b> File information: </b><br>
    Created: 2004/08/24 16:01:13 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the YTO . */

/*! \defgroup   yto YIG Tuned Oscillator (YTO)
    \ingroup    lo
    \brief      YTO module
    \note       The \ref yto module doesn't include any submodule.

    For more information on this module see \ref yto.h */

#ifndef _YTO_H
#define _YTO_H

/* Extra includes */
#include "globalDefinitions.h"
#include "packet.h"

/* Submodules definitions */
#define YTO_MODULES_NUMBER 1  // It's just the coarseTuneHandler

/* YTO Definitions */
#define YTO_COARSE_SET_MIN 0     // Minimum value for the coarse tuning of the YTO
#define YTO_COARSE_SET_MAX 4095  // Maximum value for the coarse tuning of the YTO

/* Typedefs */
//! Current state of the YTO
/*! This structure represent the current state of the YTO.
    \ingroup    lo
    \param      coarseTune      This contains the current coarse tune of the
                                YTO. It has to be stored because this is \em
                                not a read-back value from the hardware but
                                just a register holding the last issued
                                control.
    \param      lastCoarseTune  This contains a copy of the last issued
                                control to the YTO coarse tune. */
typedef struct {
    //! Current YTO counts
    /*! These are the counts as set by the operator with the last issued
        control command.
        \warning    It is not a read back of the actual value. The returned
                    value is the one stored by the software after a control
                    command has been issued.*/
    unsigned int ytoCoarseTune;
    //! Last control message: Current YTO counts
    /*! This is the content of the last control message sent to the current
        YTO counts. */
    LAST_CONTROL_MESSAGE lastYtoCoarseTune;
} YTO;

/* Prototypes */
void ytoCoarseTuneHandler(int currentModule);
void ytoHandler(int currentModule);  //!< This function deals with the incoming can message

#endif /* _YTO_H */
