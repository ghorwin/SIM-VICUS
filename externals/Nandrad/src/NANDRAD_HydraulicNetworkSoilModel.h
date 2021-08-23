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

	IDType						m_id = INVALID_ID;					// XML:A

	/*! NANDRAD id of supply pipe */
	std::vector<unsigned int>	m_supplyPipeIds;					// XML:E

	/*! NANDRAD id of return pipe */
	std::vector<unsigned int>	m_returnPipeIds;					// XML:E

	/*! Distance between pipes */
	IBK::Parameter				m_pipeSpacing;						// XML:E

	/*! Distance below surface */
	IBK::Parameter				m_pipeDepth;						// XML:E

	/*! Outer diameter of pipe */
	IBK::Parameter				m_pipeOuterDiameter;				// XML:E

	/*! Length of pipe, needed to calculate the heat loss per m */
	IBK::Parameter				m_pipeLength;						// XML:E
};


} // namespace NANDRAD


#endif // HYDRAULICNETWORKSOILMODEL_H
