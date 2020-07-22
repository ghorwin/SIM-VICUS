#ifndef NANDRAD_MaterialH
#define NANDRAD_MaterialH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! Class containing material data needed in the construction model. */
class Material {
public:

	enum para_t {
		MP_DENSITY,					// Keyword: Density				[kg/m3]	'Dry density of the material.'
		MP_HEAT_CAPACITY,			// Keyword: HeatCapacity		[J/kgK]	'Specific heat capacity of the material.'
		MP_CONDUCTIVITY,			// Keyword: Conductivity		[W/mK]	'Thermal conductivity of the dry material.'
		NUM_MP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMP(Material)
	NANDRAD_COMPARE_WITH_ID

	/*! Returns true, if all parameters are the same (id and displayname are ignored). */
	bool behavesLike(const Material & other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int				m_id;							// XML:A:required
	/*! Display name of material. */
	std::string					m_displayName;					// XML:A
	/*! List of parameters. */
	IBK::Parameter				m_para[NUM_MP];					// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_MaterialH
