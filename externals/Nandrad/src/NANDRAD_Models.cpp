#include "NANDRAD_Models.h"

#include <tinyxml.h>
#include <IBK_messages.h>

namespace NANDRAD {

void Models::readXML(const TiXmlElement * element) {
	FUNCID(Models::readXML);

	// loop over all elements in this XML element
	for (const TiXmlElement * e=element->FirstChildElement(); e; e = e->NextSiblingElement()) {

		// get element name
		std::string name = e->Value();
		// handle known elements

		if (name == "NaturalVentilationModel") {
			NaturalVentilationModel model;
			try {
				model.readXML(e);
			} catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, "Error reading NaturalVentilationModel in Models tag.", FUNC_ID);
			}
			m_naturalVentilationModels.push_back(model);
		}
		else {
			IBK::IBK_Message(IBK::FormatString(
					"Unknown element '%1' in Models section.").arg(name), IBK::MSG_WARNING);
		}

	}
}
// ----------------------------------------------------------------------------


TiXmlElement * Models::writeXML(TiXmlElement * parent) const {

	// write nothing if all containers are empty
	if (m_naturalVentilationModels.empty()) return nullptr;

	TiXmlComment::addComment(parent, "Model parameterization blocks");

	TiXmlElement * e1 = new TiXmlElement( "Models" );
	parent->LinkEndChild( e1 );

	// now write all models as they are defined
	for (auto m : m_naturalVentilationModels)
		m.writeXML(e1);

	TiXmlComment::addSeparatorComment(parent);
	return e1;
}
// ----------------------------------------------------------------------------

} // namespace NANDRAD
