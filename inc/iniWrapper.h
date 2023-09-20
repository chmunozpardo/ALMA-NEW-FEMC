/*! \file       iniWrapper.h
    \brief      Header for wrapper of 3rd party INI file reader

    <b> File information: </b><br>
    Created: 2007/06/29 16:47:04 by avaccari

    This file is a wrapper to interface with a 3rd party library to access
    configuration files written in INI format. */

#ifndef _INIWRAPPER_H
#define INI_WRAPPER_H

/* Extra includes */
#include "ini.h"

/* Defines */
#define DATA_NOT_FOUND (-2)
#define FILE_OPEN_ERROR (-3)
#define FILE_ERROR (-4)
#define FILE_CLOSE_ERROR (-5)
#define ITEMS_NO_ERROR (-6)

/* Prototypes */
/* Statics */
/* Externs */
extern int myReadCfg(const char *fileName, char *sectionName, CFG_STRUCT *searchVar, unsigned char expectedItems);
//!< This function will read the specified data from the specified file
//!< If expectedItems == 0 then no error if zero or too many items returned.
extern int myWriteCfg(const char *fileName, char *sectionName, char *varWanted,
                      char *newData);  //! This function will update the INI file
#endif                                 /* _INIWRAPPER_H */
