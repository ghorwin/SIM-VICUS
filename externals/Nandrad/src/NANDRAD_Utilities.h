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

#ifndef NANDRAD_UtilitiesH
#define NANDRAD_UtilitiesH

#include <string>
#include <map>
#include <IBK_Path.h>

class TiXmlDocument;
class TiXmlElement;

namespace NANDRAD {

/*! Attempts to open an XML file, hereby substituting placeholders in the file name and checking if
	the top-level XML tag matches the requested tag name.
*/
TiXmlElement * openXMLFile(const std::map<std::string,IBK::Path>  &pathPlaceHolders, const IBK::Path & filename,
	const std::string & parentXmlTag, TiXmlDocument & doc);

} // namespace NANDRAD

#endif // NANDRAD_UtilitiesH
