/*! \file       vacuumController.h
    \ingroup    cryostat
    \brief      Vacuum controller header file

    <b> File information: </b><br>
    Created: 2004/10/25 17:30:53 by avaccari

    This file contains all the information necessary to define the
    characteristics and operate the vacuum sensor. */

/*! \defgroup   vacuumController    Vacuum controller
    \ingroup    cryostat
    \brief      Vacuum controller module

    This group includes all the different \ref vacuumController submodules. For
    more information on the \ref vacuumController module see
    \ref vacuumController.h */

#ifndef _VACUUMCONTROLLER_H
#define _VACUUMCONTROLLER_H

/* Extra Includes */
#include "globalDefinitions.h"
#include "packet.h"
#include "vacuumSensor.h"

/* Defines */
#define VACUUM_CONTROLLER_MODULES_NUMBER 4  // See list below
#define VACUUM_CONTROLLER_MODULES_RCA_MASK           \
    0x00003 /* Mask to extract the submodule number: \
               0 -> vacuumSensorHandler              \
               1 -> vaccumSensorHandler              \
               2 -> enableHandler                    \
               3 -> stateHandler */

/* Typedefs */
//! Current state of the vacuum controller
/*! This structure represent the current state of the vacuum controller.
    \ingroup    cryostat
    \param      vacuumSensor[Ps]    This contains the information about the
                                    sensors available to the vacuum
                                    controller. There can be up to
                                    \ref VACUUM_SENSORS_NUMBER sensors
                                    connected to the vacuum controller.
    \param      enable          This contains the current state of the
                                    vacuum controller. It has to be
                                    remembered that this is \em not a read
                                    back from the hardware byt just a
                                    register holding the last issued
                                    control:
                                        - \ref VACUUM_CONTROLLER_DISABLE
                                               -> OFF
                                        - \ref VACUUM_CONTROLLER_ENABLE
                                               -> ON
    \param      state           This contains the current state of the
                                    vacuum controller:
                                        - \ref NO_ERROR -> No Error
                                        - \ref ERROR -> Error
    \param      lastEnable          This contains a copu of the last issued
                                    control message to the state of the
                                    vacuum controller. */
typedef struct {
    //! Current pressure reading
    /*! There are \ref VACUUM_SENSOR_NUMBERS attached to the vacuum
        controller. The sensor \p Se are assigned acconding to the
        following:
            - Se = 0: Cryostat Pressure
            - Se = 1: Vacuum Port Pressure
        Please see \ref VACUUM_SENSOR for more information. */
    VACUUM_SENSOR vacuumSensor[VACUUM_SENSORS_NUMBER];
    //! Vacuum controller state
    /*! This is the state of the vacuum controller:
            - \ref VACUUM_CONTROLLER_DISABLE -> OFF/Disable
            - \ref VACCUM_CONTROLLER_ENABLE -> ON/Enable
        \warning    It is not a read back of the actual value. The returned
                    value is th one stored by the software after a control
                    command has been issued. */
    unsigned char enable;
    //! Vacuum controller error state
    /*! This is the error state of the vacuum controller:
            - \ref NO_ERROR -> no error
            - \ref ERROR -> error */
    unsigned char state;
    //! Last control message: vacuum controller state
    /*! This is the content of the last control message sent to the vacuum
        controller state. */
    LAST_CONTROL_MESSAGE lastEnable;
} VACUUM_CONTROLLER;

/* Prototypes */
void vacuumControllerEnableHandler(int currentVacuumControllerModule);
void vacuumControllerStateHandler(int currentVacuumControllerModule);
void vacuumControllerHandler(int currentCryostatModule);  //!< This function deals with the incoming CAN message

#endif /* _VACUUMCONTROLLER_H */
