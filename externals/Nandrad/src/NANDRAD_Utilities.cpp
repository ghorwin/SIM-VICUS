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
#include <IBK_Flag.h>
#include <IBK_Unit.h>
#include <IBK_Parameter.h>
#include <IBK_IntPara.h>

#include "NANDRAD_Constants.h"

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
		return nullptr; // empty file?
	std::string rootnode = xmlElem->Value();
	if (rootnode != parentXmlTag)
		throw IBK::Exception( IBK::FormatString("Expected '%1' as root node in XML file.").arg(parentXmlTag), FUNC_ID);

	return xmlElem;
}


void readLinearSplineElement(const TiXmlElement * element,
							 IBK::LinearSpline & spl, std::string & name, IBK::Unit * xunit, IBK::Unit * yunit)
{
	FUNCID(NANDRAD::readLinearSplineElement);
	std::string xunitstr, yunitstr, interpolationMethod;
	std::vector<double> x,y;
	try {
		TiXmlElement::readIBKLinearSplineElement(element, name, interpolationMethod, xunitstr, x, yunitstr, y);
	}
	catch (std::runtime_error & ex) {
		throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			IBK::FormatString("Error reading 'IBK:LinearSpline' tag.") ), FUNC_ID);
	}
	try {
		if (xunit != nullptr)
			*xunit = IBK::Unit(xunitstr);
		if (yunit != nullptr)
			*yunit = IBK::Unit(yunitstr);
		spl.setValues(x,y);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			 IBK::FormatString("Error reading 'IBK:LinearSpline' tag.") ), FUNC_ID);
	}
}


void writeLinearSplineElement(TiXmlElement * parent, const std::string & eName, const IBK::LinearSpline & spl, const std::string & xunit, const std::string & yunit) {
	TiXmlElement::appendIBKLinearSplineElement(parent, eName, "", xunit, spl.x(), yunit, spl.y());
}


void readParameterElement(const TiXmlElement * element, IBK::Parameter & p) {
	FUNCID(NANDRAD::readParameterElement);
	const TiXmlAttribute* attrib = TiXmlAttribute::attributeByName(element, "name");
	if (attrib == nullptr)
		throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg("Missing 'name' attribute in IBK:Parameter element."), FUNC_ID);
	const std::string & namestr = attrib->ValueStr();
	attrib = TiXmlAttribute::attributeByName(element, "unit");
	if (attrib == nullptr)
		throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg("Missing 'unit' attribute in IBK:Parameter element."), FUNC_ID);
	const std::string & unit = attrib->ValueStr();
	IBK::Unit u;
	try {
		u.set(unit);
	} catch (...) {
		throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg("Unknown/invalid unit '"+unit+"' in IBK:Parameter element."), FUNC_ID);
	}
	const char * const str = element->GetText();
	try {
		double val = IBK::string2val<double>(str);
		p.set(namestr, val, u);
	} catch (...) {
		throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg("Cannot read value in IBK:Parameter element."), FUNC_ID);
	}
}


void readIntParaElement(const TiXmlElement * element, IBK::IntPara & p) {
	FUNCID(NANDRAD::readIntParaElement);
	const TiXmlAttribute* attrib = TiXmlAttribute::attributeByName(element, "name");
	if (attrib == nullptr)
		throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg("Missing 'name' attribute in IBK:IntPara element."), FUNC_ID);
	const std::string & namestr = attrib->ValueStr();
	const char * const str = element->GetText();
	try {
		int val = IBK::string2val<int>(str);
		p.set(namestr, val);
	} catch (...) {
		throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg("Cannot read value in IBK:IntPara element."), FUNC_ID);
	}
}


void readFlagElement(const TiXmlElement * element, IBK::Flag & f) {
	FUNCID(NANDRAD::readFlagElement);
	std::string namestr;
	std::string valueStr;
	TiXmlElement::readSingleAttributeElement(element, "name", namestr, valueStr);
	if (valueStr == "true" || valueStr == "1")
		f.set(namestr, true);
	else if (valueStr == "false" || valueStr == "0")
		f.set(namestr, false);
	else {
		throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			IBK::FormatString("Error reading 'IBK:Flag' tag, expected 'true' or 'false' as value.") ), FUNC_ID);
	}

}


IBK::Unit readUnitElement(const TiXmlElement * element, const std::string & eName) {
	FUNCID(NANDRAD::readUnitElement);
	try {
		return IBK::Unit(element->GetText());
	} catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			IBK::FormatString("Error reading '"+eName+"' tag, invalid/unknown unit'"+element->GetText()+"'.") ), FUNC_ID);
	}
}


IBK::Time readTimeElement(const TiXmlElement * element, const std::string & eName) {
	FUNCID(NANDRAD::readTimeElement);
	IBK::Time t = IBK::Time::fromDateTimeFormat(element->GetText());
	if (!t.isValid())
		throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			IBK::FormatString("Error reading '"+eName+"' tag, invalid date/time format '"+element->GetText()+"', expected 'dd.MM.yyyy hh:mm:ss'.") ), FUNC_ID);
}


} // namespace NANDRAD
