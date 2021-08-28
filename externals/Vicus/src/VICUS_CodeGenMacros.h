/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#ifndef VICUS_CodeGenMacrosH
#define VICUS_CodeGenMacrosH

class TiXmlElement;

// IDType is used instead of unsigned int for special serialization feature
typedef unsigned int IDType;

#define VICUS_READWRITE \
	void readXML(const TiXmlElement * element); \
	TiXmlElement * writeXML(TiXmlElement * parent) const;

#define VICUS_READWRITE_OVERRIDE \
	void readXML(const TiXmlElement * element) override; \
	TiXmlElement * writeXML(TiXmlElement * parent) const override;

#define VICUS_READWRITE_IFNOTEMPTY(X) \
	void readXML(const TiXmlElement * element) { readXMLPrivate(element); } \
	TiXmlElement * writeXML(TiXmlElement * parent) const { if (*this != X()) return writeXMLPrivate(parent); else return nullptr; }

#define VICUS_READWRITE_IFNOT_INVALID_ID \
	void readXML(const TiXmlElement * element) override { readXMLPrivate(element); } \
	TiXmlElement * writeXML(TiXmlElement * parent) const { if (m_id != INVALID_ID) return writeXMLPrivate(parent); else return nullptr; }

#define VICUS_READWRITE_PRIVATE \
	void readXMLPrivate(const TiXmlElement * element); \
	TiXmlElement * writeXMLPrivate(TiXmlElement * parent) const;

#define VICUS_COMP(X) \
	bool operator!=(const X & other) const; \
	bool operator==(const X & other) const { return !operator!=(other); }

#define VICUS_COMPARE_WITH_ID \
	bool operator==(unsigned int x) const { return m_id == x; }

#define VICUS_COMPARE_WITH_NAME \
	bool operator==(const std::string & name) const { return m_name == name; }

#endif // VICUS_CodeGenMacrosH
