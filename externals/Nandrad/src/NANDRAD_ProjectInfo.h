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

#ifndef NANDRAD_ProjectInfoH
#define NANDRAD_ProjectInfoH

class TiXmlElement;

#include <IBK_Parameter.h>
#include <IBK_Flag.h>

namespace NANDRAD {

/*! Contains meta-information about the project, including GUI settings.
*/
class ProjectInfo  {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***
	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);
	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent) const;

	/*! Comments about the project. */
	std::string							m_comment;
	std::string							m_created;
	std::string							m_lastEdited;
	std::string							m_version;
};

} // namespace NANDRAD

#endif // ProjectH
