/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#include "NANDRAD_Utilities.h"

#include <tinyxml.h>

#include <IBK_StringUtils.h>
#include <IBK_FileUtils.h>
#include <IBK_Path.h>

namespace NANDRAD {

TiXmlElement * openXMLFile(const std::map<std::string,IBK::Path> & pathPlaceHolders, const IBK::Path & filename,
	const std::string & parentXmlTag, TiXmlDocument & doc)
{
	const char * const FUNC_ID = "[NANDRAD::openXMLFile]";
	// replace path placeholders
	IBK::Path fname = filename.withReplacedPlaceholders( pathPlaceHolders );

	if ( !fname.isFile() )
		throw IBK::Exception(IBK::FormatString("File '%1' does not exist or cannot be opened for reading.")
				.arg(fname), FUNC_ID);

	if (!doc.LoadFile(fname.str().c_str(), TIXML_ENCODING_UTF8)) {
		throw IBK::Exception(IBK::FormatString("Error in line %1 of project file '%2':\n%3")
				.arg(doc.ErrorRow())
				.arg(filename)
				.arg(doc.ErrorDesc()), FUNC_ID);
	}

	// we use a handle so that NULL pointer checks are done during the query functions
	TiXmlHandle xmlHandleDoc(&doc);

	// read root element
	TiXmlElement * xmlElem = xmlHandleDoc.FirstChildElement().Element();
	if (!xmlElem)
		return NULL; // empty file?
	std::string rootnode = xmlElem->Value();
	if (rootnode != parentXmlTag)
		throw IBK::Exception( IBK::FormatString("Expected '%1' as root node in XML file.").arg(parentXmlTag), FUNC_ID);

	return xmlElem;
}


} // namespace NANDRAD
