#ifndef HYDRAULICNETWORKSOILMODEL_H
#define HYDRAULICNETWORKSOILMODEL_H

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

#include <IBK_Parameter.h>

namespace NANDRAD {

class HydraulicNetworkSoilModel {

public:

	NANDRAD_READWRITE

	HydraulicNetworkSoilModel();

	IDType				m_id = INVALID_ID;							// XML:A

	/*! NANDRAD id of supply pipe */
	IDType				m_supplyPipeId = INVALID_ID;				// XML:A

	/*! NANDRAD id of return pipe */
	IDType				m_returnPipeId = INVALID_ID;				// XML:A

	/*! Distance between pipes */
	IBK::Parameter		m_pipeSpacing;								// XML:E

	/*! Distance below surface */
	IBK::Parameter		m_pipeDepth;								// XML:E

	/*! Outer diameter of pipe */
	IBK::Parameter		m_pipeOuterDiameter;						// XML:E
};


} // namespace NANDRAD


#endif // HYDRAULICNETWORKSOILMODEL_H
