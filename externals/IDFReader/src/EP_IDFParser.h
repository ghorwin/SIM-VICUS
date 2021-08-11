#ifndef EP_IDFParserH
#define EP_IDFParserH

#include <map>
#include <vector>
#include <string>

#include <IBK_Path.h>
#include <IBK_Exception.h>

#include "EP_Version.h"

namespace EP {

/*! A generic (fast) IDF file parser with support for entity-level comments (last comment before a new entity is started). */
class IDFParser {
public:

	struct Entity {
		/*! Here the last comment read before start of the entity is stored. */
		std::string	m_comment;
		/*! The tokens (elements) of the entry, including the type identification string (first token). */
		std::vector<std::string> m_tokens;
	};

	/*! Parses the IDF file. */
	void read(const IBK::Path & fname);

	template <class T>
	void readClassObj(const std::string & sectionName, std::vector<T> &objects) const {
		FUNCID(IDFParser::readClassObj);
		auto it = m_tables.find(sectionName);
		if (it == m_tables.end())
			return; // nothing to read
		for (auto e : it->second) {
			// process idf data
			T newObject;
			try {
				newObject.read(e.m_tokens, m_version);
			} catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error reading IDF item of type '%1'.").arg(sectionName), FUNC_ID);
			}
			objects.push_back(newObject);
		}
	}

	/*! Contains the data tables read from the IDF file. */
	std::map<std::string, std::vector<Entity> >	m_tables;

	/*! The file version, stored and cached for convenience. This variable is
		updated in read().
	*/
	Version										m_version = NUM_VN;
};

} // namespace EP

#endif // EP_IDFParserH
