#include "SVColorMap.h"

#include <tinyxml.h>

#include <IBK_StringUtils.h>


SVColorMap::SVColorMap() {
}


void SVColorMap::writeXML(TiXmlElement * parent) const {

	TiXmlElement * e = new TiXmlElement("ColorStops");
	parent->LinkEndChild(e);

	for (unsigned int i=0; i<m_linearColorStops.size(); ++i ) {

		QColor col = m_linearColorStops.at(i).m_color;
		double value = m_linearColorStops.at(i).m_pos;

		TiXmlElement * es = new TiXmlElement("ColorStop");
		e->LinkEndChild(es);

		es->SetAttribute("index", IBK::val2string(i) );
		es->SetAttribute("position", IBK::val2string<double>(value));
		es->SetAttribute("color", col.name().toStdString() );
	}
}


void SVColorMap::interpolateColor(const double & val, QColor & color) const {
	// we expect stop values to be monotonic increasing

	if (val <= m_linearColorStops[0].m_pos) {
		color = m_linearColorStops[0].m_color;
		return;
	}

	bool interpolateBetweenSteps = m_linearColorStops.size() < 50;

	for (unsigned int i=0; i<m_linearColorStops.size()-1; ++i) {
		if (val>m_linearColorStops[i].m_pos && val<=m_linearColorStops[i+1].m_pos ) {
			if (interpolateBetweenSteps) {
				double hMax, sMax, vMax, hMin, sMin, vMin;
				m_linearColorStops[i].m_color.getHsvF(&hMin, &sMin, &vMin);
				m_linearColorStops[i+1].m_color.getHsvF(&hMax, &sMax, &vMax);
				double val_relative = (val - m_linearColorStops[i].m_pos) / (m_linearColorStops[i+1].m_pos - m_linearColorStops[i].m_pos);
				// only hue is interpolated
				double hNew = hMin + val_relative * (hMax - hMin);
				color.setHsvF(hNew, sMax, vMax);
			}
			else
				color = m_linearColorStops[i].m_color;
			return;
		}
	}

	color = m_linearColorStops.back().m_color;
}


void SVColorMap::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[SVColorMap::readXML]";
	try {

		const TiXmlElement * xmlElem = element->FirstChildElement( "ColorStops" );
		if (xmlElem != nullptr) {
			m_linearColorStops.clear();

			// read sub-elements
			for (const TiXmlElement * e = xmlElem->FirstChildElement(); e; e = e->NextSiblingElement()) {

				std::string ename = e->Value();
				if (ename == "ColorStop") {

					const TiXmlAttribute * index = TiXmlAttribute::attributeByName(e, "index");
					if (!index)
						throw IBK::Exception(IBK::FormatString("Expected 'index' attribute in Series."), FUNC_ID);

					const TiXmlAttribute * position = TiXmlAttribute::attributeByName(e, "position");
					if (!position)
						throw IBK::Exception(IBK::FormatString("Expected 'position' attribute in Series."), FUNC_ID);

					double positionValue = IBK::string2val<double>(position->Value());

					const TiXmlAttribute * color = TiXmlAttribute::attributeByName(e, "color");
					if (!color)
						throw IBK::Exception(IBK::FormatString("Expected 'color' attribute in Series."), FUNC_ID);

					QColor col = QColor();
					col.setNamedColor(color->Value());

					m_linearColorStops.push_back( ColorStop( positionValue, col ) );
				}
			}
		}

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading ColorMap."), FUNC_ID);
	}
}

