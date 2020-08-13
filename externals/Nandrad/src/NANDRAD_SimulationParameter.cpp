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

#include "NANDRAD_SimulationParameter.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {


void SimulationParameter::initDefaults() {

	m_para[SP_INITIAL_TEMPERATURE].set( KeywordList::Keyword("SimulationParameter::para_t", SP_INITIAL_TEMPERATURE),	 20, IBK::Unit("C"));
	m_para[SP_INITIAL_RELATIVE_HUMIDITY].set( KeywordList::Keyword("SimulationParameter::para_t", SP_INITIAL_RELATIVE_HUMIDITY),	 50, IBK::Unit("%"));
	m_para[SP_RADIATION_LOAD_FRACTION].set( KeywordList::Keyword("SimulationParameter::para_t", SP_RADIATION_LOAD_FRACTION),	 0.5, IBK::Unit("---"));
	m_para[SP_USERTHERMALRADIATIONFRACTION].set( KeywordList::Keyword("SimulationParameter::para_t", SP_USERTHERMALRADIATIONFRACTION),	 0.3, IBK::Unit("---"));
	m_para[SP_EQUIPMENTTHERMALLOSSFRACTION].set( KeywordList::Keyword("SimulationParameter::para_t", SP_EQUIPMENTTHERMALLOSSFRACTION),	 0.1, IBK::Unit("---"));
	m_para[SP_EQUIPMENTTHERMALRADIATIONFRACTION].set( KeywordList::Keyword("SimulationParameter::para_t", SP_EQUIPMENTTHERMALRADIATIONFRACTION),	 0.3, IBK::Unit("---"));
	m_para[SP_LIGHTINGVISIBLERADIATIONFRACTION].set( KeywordList::Keyword("SimulationParameter::para_t", SP_LIGHTINGVISIBLERADIATIONFRACTION),	 0.18, IBK::Unit("---"));
	m_para[SP_LIGHTINGTHERMALRADIATIONFRACTION].set( KeywordList::Keyword("SimulationParameter::para_t", SP_LIGHTINGTHERMALRADIATIONFRACTION),	 0.72, IBK::Unit("---"));
	m_para[SP_DOMESTICWATERHEATGAINFRACTION].set(KeywordList::Keyword("SimulationParameter::para_t", SP_DOMESTICWATERHEATGAINFRACTION), 0.0, IBK::Unit("---"));

	m_intpara[SIP_YEAR].set( KeywordList::Keyword("SimulationParameter::intpara_t", SIP_YEAR), 2001);

	// setting flags to false is normally not necessary, since the function Flag::isEnabled() returns false for undefined flags anyway
//	m_flags[SF_ENABLE_MOISTURE_BALANCE].set(KeywordList::Keyword("SimulationParameter::flag_t", SF_ENABLE_MOISTURE_BALANCE), false);
//	m_flags[SF_ENABLE_CO2_BALANCE].set(KeywordList::Keyword("SimulationParameter::flag_t", SF_ENABLE_CO2_BALANCE), false);
//	m_flags[SF_ENABLE_JOINT_VENTILATION].set(KeywordList::Keyword("SimulationParameter::flag_t", SF_ENABLE_JOINT_VENTILATION), false);
//	m_flags[SF_EXPORT_CLIMATE_DATA_FMU].set(KeywordList::Keyword("SimulationParameter::flag_t", SF_EXPORT_CLIMATE_DATA_FMU), false);

	m_interval.m_para[NANDRAD::Interval::IP_START]		= IBK::Parameter("Start", 0, "d");
	m_interval.m_para[NANDRAD::Interval::IP_END]		= IBK::Parameter("End", 365, "d");
}


#if 0
void SimulationParameter::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[SimulationParameter::readXML]";
	// read all parameters
	const TiXmlElement * c;
	try {

		// read sub-elements
		for ( c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {

			// determine data based on element name
			const std::string & cname = c->ValueStr();
			if (cname == "IBK:Parameter") {

				// use utility function to read parameter
				std::string namestr, unitstr;
				double value;
				TiXmlElement::readIBKParameterElement(c, namestr, unitstr, value);
				// determine type of parameter

				if(!KeywordList::KeywordExists("SimulationParameter::para_t", namestr) )
				{
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Unknown parameter '%1'.").arg(namestr)
						), FUNC_ID);
				}

				para_t t = (para_t)KeywordList::Enumeration("SimulationParameter::para_t", namestr);

				if (!unitstr.empty()) {
					m_para[t].set(namestr, value, unitstr);

					// check parameter unit
					std::string paraUnit = KeywordList::Unit("SimulationParameter::para_t", t);
					if (unitstr != paraUnit) {
						try {
							m_para[t].get_value(paraUnit);
						}
						catch (IBK::Exception &ex) {
							throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
								IBK::FormatString("Invalid unit '#%1' of parameter #%2!")
								.arg(paraUnit)
								.arg(namestr)
								), FUNC_ID);
						}
					}
				}
				else
					m_para[t].set(namestr, value);

			}
			else if (cname == "IBK:String") {

				const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(c, "name");
				if (!attrib)
				{
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Missing string parameter name.")
						), FUNC_ID);
				}

				std::string nameStr = attrib->Value();
				std::string valueStr = c->GetText();
				// determine type of parameter

				if(!KeywordList::KeywordExists("SimulationParameter::stringPara_t", nameStr) )
				{
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Unknown parameter '%1'.").arg(nameStr)
						), FUNC_ID);
				}

				stringPara_t t = (stringPara_t)KeywordList::Enumeration("SimulationParameter::stringPara_t", nameStr);
				// store value
				m_stringPara[t] = valueStr;
			}
			else if (cname == "Interval") {

				m_interval.readXML(c);
			}
			else if(cname == "IBK:Flag") {
				std::string namestr;
				bool value;
				const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(c, "name");

				if (!attrib)
				{
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Missing 'name' attribute in IBK:Flag.")
						), FUNC_ID);
				}

				namestr = attrib->Value();

				if(!KeywordList::KeywordExists("SimulationParameter::flag_t", namestr) )
				{
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Unknown flag '%1'.").arg(namestr)
						), FUNC_ID);
				}

				flag_t t = (flag_t)KeywordList::Enumeration("SimulationParameter::flag_t", namestr);

				value = (c->GetText() == std::string("true"));
				m_flags[t] = IBK::Flag(namestr,value);
			}
			else if (KeywordList::KeywordExists("SimulationParameter::intpara_t", cname)) {
				intpara_t p = (intpara_t)KeywordList::Enumeration("SimulationParameter::intpara_t", cname);
				int val;
				std::stringstream strm(c->GetText());
				if (strm >> val)
					m_intpara[p].set(cname, val);
				else {
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid parameter value for '%1' property.").arg(cname)
						), FUNC_ID);
				}
			}
			else {
				throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
					IBK::FormatString("Unknown XML tag '%1'.").arg(cname)
					), FUNC_ID);
			}
		} // for ( c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'SimulationParameter' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'SimulationParameter' element.").arg(ex2.what()), FUNC_ID);
	}
}


void SimulationParameter::writeXML(TiXmlElement * parent) const {

	SimulationParameter tmp;
	tmp.initDefaults( );
	if (*this == tmp)
		return;

	TiXmlComment::addComment(parent,
		"Simulation settings.");
	TiXmlElement * e = new TiXmlElement("SimulationParameter");
	parent->LinkEndChild(e);

	for (unsigned int i=0; i<NUM_SIP; ++i) {
		if(m_intpara[i].name.empty()) continue;
		if(m_intpara[i] == tmp.m_intpara[i]) continue;
		if (detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("SimulationParameter::intpara_t",i));
		TiXmlElement::appendIBKParameterElement(e, m_intpara[i].name, std::string(), m_intpara[i].value, true);
	}
	for (unsigned int i=0; i<NUM_SP; ++i) {
		if(m_para[i].name.empty()) continue;
		if(m_para[i] == tmp.m_para[i]) continue;
		if(detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("SimulationParameter::para_t",i));
		if (m_para[i].IO_unit.name() == std::string("undefined") )
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, std::string(), m_para[i].get_value());
		else
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}

	for (unsigned int i=0; i<NUM_SSP; ++i) {
		if(m_stringPara[i].empty()) continue;
		if(m_stringPara[i] == tmp.m_stringPara[i]) continue;
		if(detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("SimulationParameter::m_stringPara_t",i));
		TiXmlElement::appendSingleAttributeElement(e,"IBK:String","name",
			KeywordList::Keyword("SimulationParameter::stringPara_t",i),m_stringPara[i]);
	}

	for (unsigned int i=0; i<NUM_SF; ++i) {
		if(m_flags[i].name().empty()) continue;
		if(m_flags[i] == tmp.m_flags[i]) continue;
		if(detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("SimulationParameter::flag_t",i));
		if(m_flags[i].isEnabled())
			TiXmlElement::appendSingleAttributeElement(e,"IBK:Flag","name",
				KeywordList::Keyword("SimulationParameter::flag_t",i),"true");
		else
			TiXmlElement::appendSingleAttributeElement(e,"IBK:Flag","name",
				KeywordList::Keyword("SimulationParameter::flag_t",i),"false");
	}

	if (m_interval != tmp.m_interval)
		m_interval.writeXML( e );

	TiXmlComment::addSeparatorComment(parent);
}
#endif


bool SimulationParameter::operator!=(const SimulationParameter & other) const {
	for (unsigned int i=0; i<NUM_SP; ++i) {
		if (m_para[i] != other.m_para[i])
			return true;
	}
	if (m_interval != other.m_interval) return true;

	for (unsigned int i=0; i<NUM_SF; ++i)
		if (m_flags[i] != other.m_flags[i]) return true;
	for (unsigned int i=0; i<NUM_SP; ++i)
		if (m_para[i] != other.m_para[i]) return true;
	for (unsigned int i=0; i<NUM_SIP; ++i)
		if (m_intpara[i] != other.m_intpara[i]) return true;

	return false; // this and other hold the same data
}


} // namespace NANDRAD

