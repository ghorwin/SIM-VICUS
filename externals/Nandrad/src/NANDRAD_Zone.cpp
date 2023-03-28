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
	FUNCID(Zone::readXML);

	try {
		// read parameters
		const TiXmlElement * c = element->FirstChildElement();
		// read sub-elements
		while (c) {
			// determine data based on element name
			std::string cname = c->Value();

			// Read view factors, format:
			//
			//			<ViewFactors>
			//						4001 4002:0.5;
			//						4002 4001:0.5
			//			</ViewFactors>
			if (cname == "ViewFactors") {
				std::string content = c->GetText();

				std::vector<std::string> viewFactorTripels;

				IBK::explode(content, viewFactorTripels, ';', false);

				for (std::string tripel : viewFactorTripels) {
					IBK::trim(tripel, "\t\r\n");
					if (tripel.empty())
						continue;

					std::vector<std::string> tokens;
					IBK::explode(tripel, tokens, ':', false);

					std::vector<std::string> ids;
					IBK::explode(tokens[0], ids, ' ', true);
					if (tokens.size() != 2 || ids.size() != 2) {
						throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
							IBK::FormatString("Invalid format for view factor definition line, expected to surface IDs, got '%1'").arg(tripel)
							),FUNC_ID);
					}

					std::vector<unsigned int> intIds;

					intIds.push_back(IBK::string2val<unsigned int>(ids[0]));
					intIds.push_back(IBK::string2val<unsigned int>(ids[1]));

					if (intIds[0] == intIds[1]) {
						throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
							IBK::FormatString("Invalid view factor definition line, got two identical surface IDs: '%1'").arg(tripel)
							),FUNC_ID);
					}

					double viewFactor = IBK::string2val<double>(tokens[1]);
					viewFactorPair idPair(intIds[0],intIds[1]);
					m_viewFactors.push_back(std::make_pair
						(idPair, viewFactor));
				}

				// remove ViewFactor element from parent, to avoid getting spammed with "unknown View3D" warning
				const_cast<TiXmlElement *>(element)->RemoveChild(const_cast<TiXmlElement *>(c));
			}

			// next element
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Zone' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Zone' element.").arg(ex2.what()), FUNC_ID);
	}

	// now read all private elements
	readXMLPrivate(element);
}


TiXmlElement * Zone::writeXML(TiXmlElement * parent) const {

	TiXmlElement * e = writeXMLPrivate(parent);

	// write view factors
	if(!m_viewFactors.empty() ) {
		std::string str = "";
		for(unsigned int i = 0; i < m_viewFactors.size(); ++i) {
			str += IBK::val2string<unsigned int>(m_viewFactors[i].first.first) +" " +
					IBK::val2string<unsigned int>(m_viewFactors[i].first.second) + ":" +
					IBK::val2string<double>(m_viewFactors[i].second) + ";";
		}
		std::vector<std::string> tokens;
		IBK::explode(str, tokens, ';', false);


		TiXmlElement * viewFactors = new TiXmlElement("ViewFactors");
		viewFactors->LinkEndChild(new TiXmlText(str.c_str()));
		e->LinkEndChild(viewFactors);
	}
	return e;
}


} // namespace NANDRAD
