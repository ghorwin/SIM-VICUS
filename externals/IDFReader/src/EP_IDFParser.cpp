#include "EP_IDFParser.h"

#include <fstream>
#include <iostream>

#include <IBK_Exception.h>
#include <IBK_FileReader.h>
#include <IBK_FileUtils.h>
#include <IBK_Version.h>

namespace EP {

// CHUNK Size
#define CHUNK_SIZE 4096

void IDFParser::read(const IBK::Path & fname) {
	FUNCID(EP_IDFParser::read);
	// open file in binary mode and check if file exists
	std::ifstream in;
	if (!IBK::open_ifstream(in, fname, std::ios_base::binary))
		throw IBK::Exception( IBK::FormatString("Climate data file '%1' does not exist or is not accessible.")
							  .arg(fname), FUNC_ID);

	// look for a BOM and skip it if a BOM exists
	std::vector<unsigned char> first4(4, 0);
	in.read(reinterpret_cast<char*>(&first4[0]), 4);
	// set stream back to begin
	in.clear();
	in.seekg(0, std::ios_base::beg);
	IBK::FileReader::BOMType bomtype = IBK::FileReader::getBOM(first4);
	if (bomtype != IBK::FileReader::NoBOM) {
		unsigned int skipBytes = IBK::FileReader::sizeForBOM(bomtype);
		in.seekg(skipBytes);
	}

	// read entire file chunk-by-chunk
	try {
		std::vector<char> bufferVect(CHUNK_SIZE + 1, 0);
		char* buffer = &bufferVect[0];
		// loop until end of file or last line token has been found
		std::streamsize readSize;

		bool inComment = false;
		std::string lastComment;
		std::string currentComment;
		Entity entity;
		for (;;) {
			// try to read 'chunksize' bytes
			if (in.read(buffer, CHUNK_SIZE))
				readSize = CHUNK_SIZE;
			else // if failed, get number of read bytes
				readSize = in.gcount();

			// chp points to the current position in the buffer
			char * chp = buffer - 1; // subtract one, since the while loop starts with ++chp
			char * bufEnd = buffer + readSize;

			// now read byte-for-byte until end of chunk
			while (++chp != bufEnd) {

				// switch over different special characters
				switch (*chp) {
					// we skip \r
					case '\r':
						continue;

					// comment-toggle on?
					case '!':
						if (!inComment) {
							inComment = true;
							currentComment.clear();
							continue;
						}
					break;

					// comment-toggle off?
					case '\n':
						if (inComment) {
							inComment = false;
//							std::cout << "!" << currentComment << std::endl;
						}
						continue;

					// end of data entry?
					case ';' :
						if (!inComment) {
							if (entity.m_tokens.size() != 0) {
								entity.m_comment = lastComment;
								entity.m_tokens[0] = IBK::tolower_string(entity.m_tokens[0]);
								IBK::trim(entity.m_tokens.back()); // trim last token
								m_tables[entity.m_tokens[0]].push_back(entity);
//								std::cout << "[" << entity.m_tokens[0] << "] " << lastComment << std::endl;
								lastComment.clear();
								currentComment.clear();
								entity.m_tokens.clear();
							}
							continue;
						}
					break;

				} // switch

				if (inComment)
					currentComment += *chp;
				else {
					// start a new data entry?
					if (entity.m_tokens.empty()) {
						// remember last comment before this entity starts
						lastComment = currentComment;
						entity.m_tokens.resize(1);
					}
					// , separator?
					if (*chp == ',') {
						IBK::trim(entity.m_tokens.back());
						entity.m_tokens.push_back(std::string());
					}
					else {
						entity.m_tokens.back() += *chp;
					}

				}
			}

			// look for end of stream or maximum line number
			if (in.eof())
				break;

		} // for (;;)

		// extract version number
		std::vector<Entity> versionData = m_tables["version"];
		if (versionData.size() != 1 || versionData[0].m_tokens.size() != 2)
			throw IBK::Exception("There must be exactly one 'version' definition with one data item in the IDF file!", FUNC_ID);

		const std::string & str = versionData[0].m_tokens[1];
		unsigned int majorNumber, minorNumber;
		if (IBK::Version::extractMajorMinorVersionNumber(str, majorNumber, minorNumber))
			m_version = (Version)(0x0100*majorNumber + minorNumber);
		else
			m_version = NUM_VN; // this should be an error, right? Or should we set some meaningful default version?
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading '%1'.").arg(fname), FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception(IBK::FormatString("%2\nError reading '%1'.").arg(fname).arg(ex.what()), FUNC_ID);
	}
}


} // namespace EP
