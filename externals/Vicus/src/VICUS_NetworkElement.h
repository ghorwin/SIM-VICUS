#ifndef NETWORKELEMENT_H
#define NETWORKELEMENT_H

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_NetworkHeatExchange.h"

#include <NANDRAD_HydraulicNetworkElement.h>

namespace VICUS {


/*! This is basically a copy of NANDRAD::HydraulicNetworkElement and used only for VICUS::SubNetwork */

class NetworkElement
{
public:
	NetworkElement();

//	// *** PUBLIC MEMBER FUNCTIONS ***

//	VICUS_READWRITE


//	// *** PUBLIC MEMBER VARIABLES ***

//	NANDRAD::HydraulicNetworkElement				m_data;				// XML:E:tag=Data

//	/*! Display name. */
//	IBK::MultiLanguageString						m_displayName;		// XML:A

};


} // namespace VICUS

#endif // NETWORKELEMENT_H
