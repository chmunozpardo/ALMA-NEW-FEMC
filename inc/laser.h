/*! \file       laser.h
    \ingroup    edfa
    \brief      EDFA laser header file

    <b> File information: </b><br>
    Created: 2007/05/29 14:51:12 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the EDFA laser. */

/*! \defgroup   laser       EDFA Laser
    \ingroup    edfa
    \brief      EDFA Laser
    \note       The \ref laser module doesn't include andy submodule.

    For more information on this module see \ref laser.h */

#ifndef _LASER_H
#define _LASER_H

/* Extra includes */
#include "globalDefinitions.h"
#include "packet.h"

/* Defines */
/* Laser pump temperature evaluation defines */
#define PTMP_0 (366.86)
#define PTMP_1 (-175.09)
#define PTMP_2 (216.53)
#define PTMP_3 (-175.61)
#define PTMP_4 (85.865)
#define PTMP_5 (-22.96)
#define PTMP_6 (2.5741)

/* Submodule definition */
#define LASER_MODULES_NUMBER 3  // See list below
#define LASER_MODULES_RCA_MASK                       \
    0x00003 /* Mask to extract the submodule number: \
                   0 -> pumpTempHandler              \
                   1 -> driveCurrentHandler          \
                   2 -> photoDetectCurrentHandler */

/* Typedefs */
//! Current state of the EDFA laser
/*! This structure represent the current state of the EDFA laser
    \ingroup    edfa
    \param      pumpTemp            This contains the last read-back
                                        value for the pump temperature.
    \param      driveCurrent        This contains the last read-back
                                        value for the drive current.
    \param      lastDriveCurrent        This contains a copy of the last
                                        issued control message to the
                                        drive current.
    \param      photoDetectCurrent  This contains the last read-back
                                        value for the photo detector
                                        current. */
typedef struct {
    //! Pump Temperature
    /*! This is the current pump temperature for the EDFA pump laser. */
    float pumpTemp;
    //! Drive Current
    /*! This is the current value for the drive current of the EDFA pump
        laser. */
    float driveCurrent;
    //! Last control message: drive current
    /*! This is the content of the last control message sent ot the drive
        current. */
    LAST_CONTROL_MESSAGE lastDriveCurrent;
    //! Photo detector current
    /*! This is the current value for the photo detector current. */
    float photoDetectCurrent;
} LASER;

/* Globals */
/* Externs */
extern unsigned char currentLaserModule;  //!< Currently addressed EDFA laser module

/* Prototypes */
/* Statics */
static void pumpTempHandler(void);
static void driveCurrentHandler(void);
static void photoDetectCurrentHandler(void);
/* Externs */
extern void laserHandler(void);  //!< This function deals with the incoming CAN messages

#endif /* _LASER_H */
