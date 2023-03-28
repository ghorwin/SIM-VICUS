/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
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
#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

TiXmlElement * openXMLFile(const std::map<std::string,IBK::Path> & pathPlaceHolders, const IBK::Path & filename,
	const std::string & parentXmlTag, TiXmlDocument & doc)
{
	FUNCID(NANDRAD::openXMLFile);
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

TiXmlElement * openXMLText(const std::string & xmltext,
						   const std::string & parentXmlTag, TiXmlDocument & doc) {
	FUNCID(NANDRAD::openXMLFile);
	if (!doc.Parse(xmltext.c_str(), nullptr, TIXML_ENCODING_UTF8)) {
		throw IBK::Exception(IBK::FormatString("Error in line %1 of project text \n%2")
				.arg(doc.ErrorRow())
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
	std::string pathStr;
	try {
		TiXmlElement::readIBKLinearSplineParameterElement(element, name, interpolationMethod,
														  xunitstr, x, yunitstr, y, pathStr);
	}
	catch (std::runtime_error & ex) {
		throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			IBK::FormatString("Error reading 'IBK:LinearSpline' tag.") ), FUNC_ID);
	}
	// initialize spline already, but only if not a TSV-file is given
	if (pathStr.empty()) {
		try {
			if (xunit != nullptr) {
				if (xunitstr.empty())
					throw IBK::Exception("Missing x value unit.", FUNC_ID);
				*xunit = IBK::Unit(xunitstr);
			}
			if (yunit != nullptr) {
				if (yunitstr.empty())
					throw IBK::Exception("Missing y value unit.", FUNC_ID);
				*yunit = IBK::Unit(yunitstr);
			}
			spl.setValues(x,y);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				 IBK::FormatString("Error reading 'IBK:LinearSpline' tag.") ), FUNC_ID);
		}
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
	return t;
}


TiXmlElement * writeVector3D(TiXmlElement * parent, const std::string & name, const std::vector<IBKMK::Vector3D> & vec) {
	if (!vec.empty()) {
		TiXmlElement * child = new TiXmlElement(name);
		parent->LinkEndChild(child);

		std::stringstream vals;
		for (unsigned int i=0; i<vec.size(); ++i) {
			vals << vec[i].m_x << " " << vec[i].m_y << " " << vec[i].m_z;
			if (i<vec.size()-1)  vals << ", ";
		}
		TiXmlText * text = new TiXmlText( vals.str() );
		child->LinkEndChild( text );
		return child;
	}
	return nullptr;
}


void readVector3D(const TiXmlElement * element, const std::string & name, std::vector<IBKMK::Vector3D> & vec) {
	FUNCID(NANDRAD::readVector3D);
	std::string text = element->GetText();
	text = IBK::replace_string(text, ",", " ");
	std::vector<double> vals;
	try {
		IBK::string2valueVector(text, vals);
		// must have n*3 elements
		if (vals.size() % 3 != 0)
			throw IBK::Exception("Mismatching number of values.", FUNC_ID);
		if (vals.empty())
			throw IBK::Exception("Missing values.", FUNC_ID);
		vec.resize(vals.size() / 3);
		for (unsigned int i=0; i<vec.size(); ++i)
			vec[i].set(vals[i*3], vals[i*3+1], vals[i*3+2]);

	} catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			IBK::FormatString("Error reading vector element '%1'.").arg(name) ), FUNC_ID);
	}
}

template<>
void readVector<double>(const TiXmlElement * element, const std::string & name, std::vector<double> & vec) {
	FUNCID(NANDRAD::readVector);
	std::string text = element->GetText();
	text = IBK::replace_string(text, ",", " ");
	try {
		IBK::string2valueVector(text, vec);
	} catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			IBK::FormatString("Error reading vector element '%1'.").arg(name) ), FUNC_ID);
	}
}


template<>
void readPoint2D<double>(const TiXmlElement * element, const std::string & name, IBK::point2D<double> & p) {
	FUNCID(NANDRAD::readVector);
	std::string text = element->GetText();
	try {
		std::vector<double> vec;
		IBK::string2valueVector(text, vec);
		if (vec.size() != 2)
			throw IBK::Exception("Size mismatch, expected 2 numbers separated by , .", FUNC_ID);
		p.m_x = vec[0];
		p.m_y = vec[1];
	} catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			IBK::FormatString("Error reading point2D element '%1'.").arg(name) ), FUNC_ID);
	}
}


void setParameter(IBK::Parameter para[], const char * const enumtype, int n, const double &val) {
	para[n] = IBK::Parameter(NANDRAD::KeywordList::Keyword(enumtype, n), val, NANDRAD::KeywordList::Unit(enumtype, n));
}

} // namespace NANDRAD
