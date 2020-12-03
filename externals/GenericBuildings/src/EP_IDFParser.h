#ifndef EP_IDFParserH
#define EP_IDFParserH

#include <map>
#include <vector>
#include <string>
#include "IBK_Path.h"

namespace EP {

/*! A generic (fast) IDF file parser with support for entity-level comments (last comment before a new entity is started). */
class IDFParser {
public:
	/*! Parses the IDF file. */
	void read(const IBK::Path & fname);

	void transferData();

	struct Entity {
		/*! Here the last comment read before start of the entity is stored. */
		std::string	m_comment;
		/*! The tokens (elements) of the entry, including the type identification string (first token). */
		std::vector<std::string> m_tokens;
	};

	/*! Contains the data tables read from the IDF file. */
	std::map<std::string, std::vector<Entity> >	m_tables;
};

} // namespace EP

#endif // EP_IDFParserH
