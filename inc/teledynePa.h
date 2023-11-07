/*! \file  teledynePa.h
    \brief Handles the requests to configure the band 7 Teledyne PA chip
 */

#ifndef _TELEDYNEPA_H
#define _TELEDYNEPA_H
#include "globalDefinitions.h"

/* Submodules definitions */
#define TELEDYNE_PA_MODULES_NUMBER 3  // See list below
#define TELEDYNE_PA_MODULES_RCA_MASK                 \
    0x00003 /* Mask to extract the submodule number: \
               0 -> hasTeledynePa                    \
               1 -> collectorBytePol0                \
               2 -> collectorBytePol1 */

void teledynePaHandler(int currentModule);
void hasTeledynePaHandler(int currentModule, int currentTeledynePaModule);
void collectorByteHandler(int currentModule, int currentTeledynePaModule);

#endif
