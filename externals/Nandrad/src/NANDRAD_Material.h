#ifndef NANDRAD_MATERIAL_H
#define NANDRAD_MATERIAL_H

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"

class TiXmlElement;

namespace NANDRAD {


class Material
{
public:

	enum para_t {
		MP_DENSITY,					// Keyword: Density				[kg/m3]	'Density of material.'
		MP_SPECHEAT,				// Keyword: SpecHeat			[J/kgK]	'Specific heat capacity of material.'
		MP_CONDUCTIVITY,			// Keyword: Conductivity		[W/mK]	'Conductivity of material.'
		NUM_MP
	};
	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE


	/*! Compares this instance with another by physical values and returns true if they differ. */
	bool operator!=(const Material & other) const;

	/*! Compares this instance with another by physical values and returns true if they are the same. */
	bool operator==(const Material & other) const { return ! operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***
	/*! Unique id number. */
	unsigned int				m_id;							// XML:A:required
	/*! IBK-language encoded name of material. */
	std::string					m_displayName;					// XML:A

	/*! List of parameters. */
	IBK::Parameter				m_para[NUM_MP];					// XML:E

};
}

#endif // NANDRAD_MATERIAL_H
