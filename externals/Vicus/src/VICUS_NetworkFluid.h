#ifndef NETWORKFLUID_H
#define NETWORKFLUID_H

#include <IBK_Parameter.h>
#include <NANDRAD_LinearSplineParameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

namespace VICUS {

class NetworkFluid {
public:
	/*! Basic parameters. */
	enum para_t {
		/*! Density. */
		P_Density,					// Keyword: Density				[kg/m3]	'Dry density of the material.'
		/*! Specific heat capacity. */
		P_HeatCapacity,				// Keyword: HeatCapacity		[J/kgK]	'Specific heat capacity of the material.'
		/*! Thermal conductivity. */
		P_Conductivity,				// Keyword: Conductivity		[W/mK]	'Thermal conductivity of the dry material.'
		NUM_P
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int						m_id = INVALID_ID;				// XML:A:required
	/*! Display name of fluid. */
	std::string							m_displayName;					// XML:A
	/*! List of parameters. */
	IBK::Parameter						m_para[NUM_P];					// XML:E

	/*! Kinematic viscosity [m2/s]. */
	NANDRAD::LinearSplineParameter		m_kinematicViscosity;			// XML:E

};

} // namespace VICUS


#endif // NETWORKFLUID_H
