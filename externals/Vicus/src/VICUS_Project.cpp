/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others ... :-)

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

#include "VICUS_Project.h"

#include <algorithm>
#include <set>

#include <IBK_messages.h>
#include <IBK_assert.h>

#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

#include "VICUS_Constants.h"

namespace VICUS {

Project::Project() {
	// build test buildin

}


void Project::readXML(const IBK::Path & filename) {
	FUNCID(Project::readXML);

	TiXmlDocument doc;
	IBK::Path filenamePath(filename);
	std::map<std::string,IBK::Path> pathPlaceHolders; // only dummy for now, filenamePath does not contain placeholders
	TiXmlElement * xmlElem = NANDRAD::openXMLFile(pathPlaceHolders, filenamePath, "VicusProject", doc);
	if (!xmlElem)
		return; // empty project, this means we are using only defaults

	// we read our subsections from this handle
	TiXmlHandle xmlRoot = TiXmlHandle(xmlElem);

	try {
		xmlElem = xmlRoot.FirstChild("Project").Element();
		if (xmlElem) {
			readXML(xmlElem);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading project '%1'.").arg(filename), FUNC_ID);
	}
}


void Project::writeXML(const IBK::Path & filename) const {
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "VicusProject" );
	doc.LinkEndChild(root);

	root->SetAttribute("fileVersion", VERSION);

	writeXML(root);


	// other files

	doc.SaveFile( filename.c_str() );

}

void Project::clean() {

}

} // namespace VICUS
