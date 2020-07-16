#include "NANDRAD_DataTable.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {

bool DataTable::operator!=(const DataTable & other) const {
	return m_values != other.m_values;
}


void DataTable::readXML(const TiXmlElement * element) {
	FUNCID(DataTable::readXML);
}


TiXmlElement * DataTable::writeXML(TiXmlElement * parent) const {
	if (m_values.empty()) return nullptr;

	TiXmlElement * e = new TiXmlElement("DataTable");
	parent->LinkEndChild(e);

	// encode data table
	std::stringstream strm;
	for (const std::pair<std::string, std::vector<double> > & column : m_values) {
		strm << IBK::trim_copy(column.first) << ":";
		for (unsigned int i=0; i<column.second.size(); ++i) {
			strm << column.second[i];
			if (i < column.second.size()-1)
				strm << ",";
		}
		strm << ";";
	}
	TiXmlText * text = new TiXmlText( strm.str() );
	e->LinkEndChild( text );

	return e;
}

}
