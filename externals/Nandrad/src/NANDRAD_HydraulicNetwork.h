#ifndef NANDRAD_HYDRAULICNETWORK_H
#define NANDRAD_HYDRAULICNETWORK_H

#include <vector>

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicFluid.h"

namespace NANDRAD {

/*! Contains all data for a hydraulic network. */
class HydraulicNetwork {
	NANDRAD_READWRITE_PRIVATE
public:


	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOT_INVALID_ID
	NANDRAD_COMPARE_WITH_ID

	/*! Unique ID for this network. */
	unsigned int							m_id			= NANDRAD::INVALID_ID;				// XML:A:required
	/*! Descriptive name. */
	std::string								m_displayName;										// XML:A

	HydraulicFluid							m_fluid;											// XML:E

	/*! List of flow elements that make up this network. */
	std::vector<HydraulicNetworkElement>	m_elements;											// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_HYDRAULICNETWORK_H
