/*! \file       dewar.h
    \ingroup    fetim
    \brief      FETIM dewar header file

    <b> File information: </b><br>
    Created: 2011/03/28 18:06:53 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the FETIM dewar. */

/*! \defgroup   dewar  FETIM dewar
    \ingroup    fetim
    \brief      FETIM dewar module
    \note       The \ref dewar module doesn't include any submodule.

    For more information on this module see \ref dewar.h */

#ifndef _DEWAR_H
#define _DEWAR_H

/* Extra includes */
#include "globalDefinitions.h"

/* Submodules definitions */
#define DEWAR_MODULES_NUMBER 1  // It's just the n2FillHandler

/* Typedefs */
//! Current state of the FETIM dewar subsystem
/*! This structure represent the current state of the FETIM dewar subsystem.
    \ingroup    fetim
    \param      n2Fill  This contains the current state of the n2Fill
                            system. It has to be remembered that this is
                            \em not a read-back from the hardware
                            but just a register holding the last issued
                            control:
                                - 0 -> OFF
                                - 1 -> ON
    \param      lastN2Fill  This contains a copy of the last issued control
                            message for the n2Fill. */
typedef struct {
    //! FETIM N2 Fill system
    /*! This is the state of the N2 fill system:
            - 0 -> OFF (power up state)
            - 1 -> ON
        \warning    It is not a read back of the actual value. The returned
                    value is the one stored by the software after a control
                    command has ben issued. */
    unsigned char n2Fill;
    //! Last control message: FETIM N2 fill
    /*! This is the content of the last control message sent to the N2 fill
        state. */
    LAST_CONTROL_MESSAGE lastN2Fill;
} DEWAR;

/* Globals */
/* Externs */
extern unsigned char currentDewarModule;  //!< Currently addressed FETIM dewar submodule

/* Prototypes */
/* Statics */
static void n2FillHandler(void);
/* Externs */
extern void dewarHandler(void);  //!< This function deals with the incoming CAN message

#endif /* _DEWAR_H */
