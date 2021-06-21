#ifndef NETWORKELEMENT_H
#define NETWORKELEMENT_H

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <NANDRAD_HydraulicNetworkElement.h>

namespace VICUS {


/*! This is basically a copy of NANDRAD::HydraulicNetworkElement and used only for VICUS::SubNetwork */

class NetworkElement
{
public:
	NetworkElement();

};


} // namespace VICUS

#endif // NETWORKELEMENT_H
