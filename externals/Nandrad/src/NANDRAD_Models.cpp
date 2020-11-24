/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "NANDRAD_Models.h"

#include <tinyxml.h>
#include <IBK_messages.h>

namespace NANDRAD {

// NOTE: we implement readXML and writeXML ourselves, so that
//       we have only one level of model hierarchy, instead of another
//       sublevel for each group of models.

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
		else if (name == "InternalLoadsModel") {
			InternalLoadsModel model;
			try {
				model.readXML(e);
			} catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, "Error reading InternalLoadsModel in Models tag.", FUNC_ID);
			}
			m_internalLoadsModels.push_back(model);
		}
		else if (name == "ShadingControlModel") {
			ShadingControlModel model;
			try {
				model.readXML(e);
			} catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, "Error reading ShadingControlModel in Models tag.", FUNC_ID);
			}
			m_shadingControlModels.push_back(model);
		}
		else {
			IBK::IBK_Message(IBK::FormatString(
					"Unknown element '%1' in Models section.").arg(name), IBK::MSG_WARNING);
		}

	}
}


TiXmlElement * Models::writeXML(TiXmlElement * parent) const {

	// write nothing if all containers are empty
	if (m_naturalVentilationModels.empty() &&
		m_shadingControlModels.empty())
		return nullptr;

	TiXmlComment::addComment(parent, "Model parameterization blocks");

	TiXmlElement * e1 = new TiXmlElement( "Models" );
	parent->LinkEndChild( e1 );

	// now write all models as they are defined
	for (auto m : m_naturalVentilationModels)
		m.writeXML(e1);
	for (auto m : m_internalLoadsModels)
		m.writeXML(e1);
	for (auto m : m_shadingControlModels)
		m.writeXML(e1);

	TiXmlComment::addSeparatorComment(parent);
	return e1;
}


/*! Test function that checks that all objects in the given vector have different m_id parameters. */
template <typename T>
void checkForUniqueModelIDs(const std::vector<T> & vec, std::set<unsigned int> & usedIDs) {
	FUNCID(NANDRAD::checkForUniqueIDs);

	for (const T & t : vec) {
		if (usedIDs.find(t.m_id) != usedIDs.end())
			throw IBK::Exception(IBK::FormatString("Duplicate model ID #%1.")
								 .arg(t.m_id), FUNC_ID);
		usedIDs.insert(t.m_id);
	}
}


void Models::checkForUniqueIDs() const {
	FUNCID(Models::checkForUniqueIDs);

	std::set<unsigned int> usedIDs;

	try {
		// check all natural ventilation models for unique ids and store
		// it in usedIDs container
		checkForUniqueModelIDs(m_naturalVentilationModels, usedIDs);
		// check all internal loads model ids against each other and against
		// all entries in usedIDs container
		checkForUniqueModelIDs(m_internalLoadsModels, usedIDs);
		// the same for shading control models
		checkForUniqueModelIDs(m_shadingControlModels, usedIDs);
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Duplicate ID found in model parameter blocks.", FUNC_ID);
	}
}

} // namespace NANDRAD
