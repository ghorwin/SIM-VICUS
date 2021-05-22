#ifndef NANDRAD_HydraulicFluidH
#define NANDRAD_HydraulicFluidH

#include <IBK_Parameter.h>

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

/*! Properties of a fluid within a network. */
class HydraulicFluid {
public:

	/*! Basic parameters. */
	enum para_t {
		P_Density,					// Keyword: Density				[kg/m3]	'Dry density of the material.'
		P_HeatCapacity,				// Keyword: HeatCapacity		[J/kgK]	'Specific heat capacity of the material.'
		P_Conductivity,				// Keyword: Conductivity		[W/mK]	'Thermal conductivity of the dry material.'
		NUM_P
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	/*! Populates the HydraulicFluid object with properties of water. */
	void defaultFluidWater(unsigned int id);

	/*! Checks for valid and required parameters (value ranges). */
	void checkParameters(int networkModelType);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Display name of fluid. */
	std::string							m_displayName;					// XML:A
	/*! List of parameters. */
	IBK::Parameter						m_para[NUM_P];					// XML:E

	/*! Kinematic viscosity [m2/s]. */
	LinearSplineParameter				m_kinematicViscosity;			// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicFluidH
