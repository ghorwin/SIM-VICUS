#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include "VICUS_Constants.h"
#include "VICUS_CodeGenMacros.h"

#include <IBK_Parameter.h>

#include <NANDRAD_HydraulicNetworkControlElement.h>


namespace VICUS {


class NetworkController
{
public:
	NetworkController();

//	VICUS_READWRITE
//	VICUS_COMP(NetworkController)

//	/*! Checks if all parameters are valid. */
//	bool isValid() const;

//	NANDRAD::HydraulicNetworkControlElement			m_data;							// XML:E:tag=Data

//	/*! Display name. */
//	IBK::MultiLanguageString						m_displayName;					// XML:A

};


} // namespace VICUS


#endif // NETWORKCONTROLLER_H
