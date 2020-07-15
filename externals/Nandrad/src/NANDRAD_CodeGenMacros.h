#ifndef NANDRAD_CodeGenMacrosH
#define NANDRAD_CodeGenMacrosH

class TiXmlElement;

#define NANDRAD_READWRITE \
	void readXML(const TiXmlElement * element); \
	TiXmlElement * writeXML(TiXmlElement * parent) const;

#define NANDRAD_READWRITE_IFNOTEMPTY(X) \
	void readXML(const TiXmlElement * element) { readXMLPrivate(element); } \
	TiXmlElement * writeXML(TiXmlElement * parent) const { if (*this != X()) return writeXMLPrivate(parent); else return nullptr; }

#define NANDRAD_READWRITE_PRIVATE \
	void readXMLPrivate(const TiXmlElement * element); \
	TiXmlElement * writeXMLPrivate(TiXmlElement * parent) const;

#define NANDRAD_COMP(X) \
	bool operator!=(const X & other) const; \
	bool operator==(const X & other) const { return !operator!=(other); }

#define NANDRAD_COMPARE_WITH_ID \
	bool operator==(unsigned int x) const { return m_id == x; }

#define NANDRAD_COMPARE_WITH_NAME \
	bool operator==(const std::string & name) const { return m_name == name; }

#endif // NANDRAD_CodeGenMacrosH
