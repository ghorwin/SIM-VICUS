#ifndef NANDRAD_CodeGenMacrosH
#define NANDRAD_CodeGenMacrosH

#define NANDRAD_READWRITE \
	void readXML(const TiXmlElement * element); \
	TiXmlElement * writeXML(TiXmlElement * parent) const;

#define NANDRAD_READWRITE_PRIVATE \
	void readXMLPrivate(const TiXmlElement * element); \
	TiXmlElement * writeXMLPrivate(TiXmlElement * parent) const;

#define NANDRAD_COMP(X) \
	bool operator!=(const X & other) const; \
	bool operator==(const X & other) { return !operator!=(other); }

#endif // NANDRAD_CodeGenMacrosH
