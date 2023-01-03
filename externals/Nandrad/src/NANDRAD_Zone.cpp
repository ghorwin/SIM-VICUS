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

#include "NANDRAD_Zone.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_messages.h>

#include <tinyxml.h>


namespace NANDRAD {

void Zone::checkParameters() const {
	FUNCID(Zone::checkParameters);

	switch (m_type) {
		// require parameter 'Temperature' for Zone attribute 'Constant'
		case ZT_Constant : {
			m_para[P_Temperature].checkedValue("Temperature", "K", "K", 173.15, false, std::numeric_limits<double>::max(), false,
											   "Parameter 'Temperature' must be > -100 C.");
		} break;

		case ZT_Scheduled : {
			// no parameters needed
		} break;

		case ZT_Active : {
			m_para[P_Volume].checkedValue( "Volume", "m3", "m3", 0, false, std::numeric_limits<double>::max(), false,
										   "Volume must be > 0 m3!");

			m_para[P_Area].checkedValue("Area", "m2", "m2", 0, true, std::numeric_limits<double>::max(), true,
										"Zone area must be >= 0 W/m2!");

			// warn if temperature parameter is given in active zone
			if (!m_para[P_Temperature].name.empty())
				IBK::IBK_Message("Temperature parameter in active zone ignored. Using global default initial temperature "
								 "from simulation parameters.", IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);

			// optional parameters
			if (!m_para[P_HeatCapacity].name.empty())
				m_para[P_HeatCapacity].checkedValue( "HeatCapacity", "J/K", "J/K", 0, true, std::numeric_limits<double>::max(), false,
											   "Heat capacity must be >= 0 J/K!");

		} break;

		case ZT_Ground : {
			// TODO : check parameters
		} break;

		case NUM_ZT : ; // just to make compiler happy
	}
}


void Zone::readXML(const TiXmlElement * element) {
	FUNCID("[Zone::readXML]");

	readXMLPrivate(element);

	try {
		// read parameters
		const TiXmlElement * c;
		// read sub-elements
		for ( c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();

			if (cname == "ViewFactors") {
				std::string viewFactorString;
				std::stringstream str(c->GetText());

				// get first line
				while(std::getline(str,viewFactorString,'\n')) {
					IBK::trim(viewFactorString);
					if(viewFactorString.empty())
						continue;
					std::vector<std::string> tokens;
					IBK::explode(viewFactorString, tokens, ' ', true);
					// alterantively search for tab string
					if(tokens.size() != 3)
						IBK::explode(viewFactorString, tokens, '\t', true);
					if (tokens.size() != 3)
						throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
							IBK::FormatString("Wrong ViewFactor format!")
							),FUNC_ID);

					viewFactorPair idPair(IBK::string2val<unsigned int>(tokens[0]),
										IBK::string2val<unsigned int>(tokens[1]));
					m_viewFactors.push_back(std::make_pair
						(idPair, IBK::string2val<double>(tokens[2])) );
				}
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Zone' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Zone' element.").arg(ex2.what()), FUNC_ID);
	}
}


TiXmlElement * Zone::writeXML(TiXmlElement * parent) const {
	writeXMLPrivate(parent);

	TiXmlElement * e = new TiXmlElement("Zone");
	parent->LinkEndChild(e);
	// write view factors
	if(!m_viewFactors.empty() ) {
		std::string str("\n");
		for(unsigned int i = 0; i < m_viewFactors.size(); ++i) {
			str += IBK::val2string<unsigned int>(m_viewFactors[i].first.first) + std::string(" ") +
					IBK::val2string<unsigned int>(m_viewFactors[i].first.second) + std::string(" ") +
					IBK::val2string<double>(m_viewFactors[i].second) + std::string("\n");
		}
		TiXmlElement::appendSingleAttributeElement(e,"ViewFactors", nullptr, std::string(), str);
	}
	return e;
}


} // namespace NANDRAD
