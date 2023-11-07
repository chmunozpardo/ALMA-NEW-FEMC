/*! \file       interlockState.h
    \ingroup    interlock
    \brief      FETIM interlock state header file

    <b> File information: </b><br>
    Created: 2011/03/29 17:51:00 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the FETIM interlock state. */

/*! \defgroup   interlockState       FETIM interlock state
    \ingroup    interlock
    \brief      FETIM interlock state

    This group includes all the different \ref interlockState submodules. For
    more information on the \ref interlockState module see
    \ref interlockState.h */

#ifndef _INTERLOCK_STATE_H
#define _INTERLOCK_STATE_H

/* Extra includes */
#include "globalDefinitions.h"
#include "interlockGlitch.h"

/* Submodule definitions */
#define INTERLOCK_STATE_MODULES_NUMBER 6  // See list below
#define INTERLOCK_STATE_MODULES_RCA_MASK                                               \
    0x0001C                                   /* Mask to extract the submodule number: \
                                                 0 -> interlockGlitchHandler           \
                                                 1 -> multiFailHandler                 \
                                                 2 -> tempOutOfRangeHandler            \
                                                 3 -> flowOutOfRangeHandler            \
                                                 4 -> delayTrigHandler                 \
                                                 5 -> shutdownTrigHandler */
#define INTERLOCK_STATE_MODULES_MASK_SHIFT 2  // Bits right shift for the submodule mask

/* Typedefs */
//! Current state of the FETIM interlock system
/*! This structure represent the current state of the FETIM interlock system
    \ingroup    interlock
    \param      glitch           This contains the state of the glitch
                                 subsystem. See \ref INTRLK_GLITCH for more
                                 information.
    \param      multiFail    This contains the current state of the multi
                                 fail register which record if more than one
                                 of the sensor failed:
                                     - \ref OK    -> Not more that one sensor failed
                                     - \ref ERROR -> At least two sensors failed
    \param      tempOutRng   This signals if any of the temperature
                                 sensors is out of the preset range:
                                     - \ref OK    -> Sensors in range
                                     - \ref ERROR -> Sensors out of range
    \param      flowOutRng   This signals if any of the flow
                                 sensors is out of the preset range:
                                     - \ref OK    -> Sensors in range
                                     - \ref ERROR -> Sensors out of range
    \param      delayTrig    This signals if the shutdown delay has been
                                 triggered:
                                     - \ref OFF -> Delay not triggered
                                     - \ref ON  -> Delay triggered
    \param      shutdownTrig This signals if the final shutdown delay
                                 has been triggered (no coming back):
                                     - \ref OFF -> Delay not triggered
                                     - \ref ON  -> Delay triggered */
typedef struct {
    //! FETIM interlock glitch counter state
    /*! This contains the state of the interlock glitch counter. See
        \ref INTRLK_GLITCH for more info */
    INTRLK_GLITCH glitch;
    //! Sensors multifail
    /*! This contains the state of the multifail register:
            - \ref OK    -> Not more than one sensor failes
            - \ref ERROR -> At least two sensors failed */
    unsigned char multiFail;
    //! Temperature sensor out of range
    /*! This contains the state of the temperature sensor out of range
        register:
            - \ref OK    -> Sensors in range
            - \ref ERROR -> Sensors out of range */
    unsigned char tempOutRng;
    //! Flow sensor out of range
    /*! This contains the state of the flowe sensor out of range register:
            - \ref OK    -> Sensors in range
            - \ref ERROR -> Sensors out of range */
    unsigned char flowOutRng;
    //! Shutdown delay
    /*! This contains the state of the shutdown delay register:
            - \ref OFF -> Delay not triggered
            - \ref OK  -> Delay triggered */
    unsigned char delayTrig;
    //! Final shutdown delay
    /*! This contains the state of the final shutdown delay register:
            - \ref OFF -> Delay not triggered
            - \ref OK  -> Delay triggered */
    unsigned char shutdownTrig;
} INTRLK_STATE;

/* Prototypes */
void multiFailHandler(void);
void tempOutOfRangeHandler(void);
void flowOutOfRangeHandler(void);
void delayTrigHandler(void);
void shutdownTrigHandler(void);
void interlockStateHandler(void);  //!< This function deals with the incoming CAN messages

#endif /* _INTERLOCK_STATE_H */
