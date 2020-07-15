#ifndef NANDRAD_ConstructionTypeH
#define NANDRAD_ConstructionTypeH

#include <vector>
#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_MaterialLayer.h"

namespace NANDRAD {

/*! Defines a multi-layered construction (without the boundary conditions). */
class ConstructionType {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMP(ConstructionType)
	NANDRAD_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***
	/*! Unique id number. */
	unsigned int				m_id;							// XML:A:required
	/*! IBK-language encoded name of construction. */
	std::string					m_displayName;					// XML:A

	/*! List of material layers. */
	std::vector<MaterialLayer>	m_materialLayers;				// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_ConstructionTypeH
