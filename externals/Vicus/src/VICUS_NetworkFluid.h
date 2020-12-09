#ifndef VICUS_NetworkFluidH
#define VICUS_NetworkFluidH

#include <IBK_Parameter.h>
#include <NANDRAD_LinearSplineParameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS{

class NetworkFluid: public AbstractDBElement {
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

	void defaultFluidWater(unsigned int id);

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


#endif // VICUS_NetworkFluidH
