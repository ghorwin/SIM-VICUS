#ifndef NANDRAD_CONSTRUCTIONTYPE_H
#define NANDRAD_CONSTRUCTIONTYPE_H

#include <vector>

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_MaterialLayer.h"

class TiXmlElement;

namespace NANDRAD {

class Material;

class ConstructionType
{
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	/*! Compares this instance with another by physical values and returns true if they differ. */
	bool operator!=(const ConstructionType & other) const;

	/*! Compares this instance with another by physical values and returns true if they are the same. */
	bool operator==(const ConstructionType & other) const { return ! operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***
	/*! Unique id number. */
	unsigned int				m_id;							// XML:A:required
	/*! IBK-language encoded name of construction. */
	std::string					m_displayName;					// XML:A

	/*! List of material layers. */
	std::vector<MaterialLayer>	m_materialLayers;				// XML:E

};
}

#endif // NANDRAD_CONSTRUCTIONTYPE_H
