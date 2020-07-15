#include "ClassInfo.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <IBK_StringUtils.h>


bool ClassInfo::parse(const IBK::Path & headerFilePath) {
	FUNCID(ClassInfo::parse);

	// store time stamp of input file

	// open file
	std::ifstream in(headerFilePath.c_str());
	if (!in) {
		std::cerr << "Error opening file '" << headerFilePath.str() << "'!"<< std::endl;
		throw std::runtime_error("read error");
	}

	m_sourceHeaderFile = headerFilePath;

	std::string line, enumname;
	std::size_t pos;
	bool inEnum = false;
	unsigned int enumIdx = 0;

	// read line by line until end of file is found
	while (getline(in, line)) {
		try {
			if (line.find("NANDRAD_READWRITE_PRIVATE") != std::string::npos) {
				m_requirePrivateReadWrite = true;
				continue;
			}
			if (line.find("NANDRAD_COMP") != std::string::npos) {
				m_requireComparisonFunction = false;
				continue;
			}

			// get class declaration and remember in which class we are
			pos = line.find("class");
			if (pos != std::string::npos && pos+6 < line.size()) {
				// check that there is nothing but whitespace before the class
				if (line.find_first_not_of(" \t") == pos) {
					if (!m_className.empty())
						throw IBK::Exception("Multiple class declarations in source file are not supported.", FUNC_ID);
					std::string cname = line.substr(pos+6); // might be "ThisAndThat: public Parent {"
					pos = cname.find_first_of(" \t{:/\n");
					// guard against pure forward-declaration, in which case the first char after the class will be a ;
					cname = cname.substr(0, pos);
					IBK::trim(cname);
					if (cname.back() == ';')
						continue; // skip this line
					m_className = cname;
		//			std::cout << line << std::endl;
		//			std::cout << "class " << classname << std::endl;
					continue;
				}
			}

			// get enum name
			pos = line.find("enum ");
			if (pos != std::string::npos) {
				enumname = line.substr(pos+4);
				IBK::trim(enumname);
				pos = enumname.find_first_of(" \t{/\n");
				enumname = enumname.substr(0, pos);
//				std::cout << line << std::endl;
//				std::cout << "enum " << classname << "::" << enumname << std::endl;
				inEnum = true;
				enumIdx = 0;
				continue;
			}

			// now look for comments of certain type
			pos = line.find("// Keyword:");
			if (pos != std::string::npos) {
				// this line holds an enumeration value, split the line at the position
				std::string kwenum = line.substr(0, pos);
				// trim the string and remove a trailing , if present
				IBK::trim(kwenum, " \t,");
				std::string kwpara = line.substr(pos + 11);
				IBK::trim(kwpara);
				// the parameter line looks like:
				// KEYWORD1 KEYWORD2  [unit] <#FFEECC> {defaultValue} 'description'
				// unit, color and default value are optional

				// we parse this back to front - and use the extractToken function
				std::string description, defaultValue, colorHash, unit;
				colorHash = "#FFFFFF"; // default color is black
				extractToken(kwpara, '\'', description); // modifies kwpara and description
				extractToken(kwpara, '{', defaultValue);
				extractToken(kwpara, '<', colorHash);
				extractToken(kwpara, '[', unit);

				// remaining tokens are keywords
				IBK::trim(kwpara);
				Keyword kw;
				kw.unit = unit;
				kw.color = colorHash;
				kw.description = description;
				kw.keyword = kwpara;
				try {
					if (!defaultValue.empty())
						kw.defaultValue = IBK::string2val<double>(defaultValue);
				} catch (...) {
					std::cerr << "Error converting default value '"<< defaultValue << "' to number." << std::endl;
					throw std::runtime_error("error");
				}
				kw.category = m_className + "::" + enumname;
				kw.index = enumIdx;
				m_keywords.push_back(kw);
				++enumIdx;
				// if line has the token NUM_ fall through to handle this in the next section
				if (line.find("NUM_") == std::string::npos)
					continue;
			}

			// if we are still in an enumeration section, check for NUM_xxx tokens - usually without Keyword
			pos = line.find("NUM_");
			if (inEnum && pos != std::string::npos) {
				// check that NUM_ does not have any leading [ or any other char
				if (line.find_first_not_of(" \t") == pos) {
					std::string::size_type pos2 = line.find_first_of(" \t\r\n}", pos);
					std::string enumName = line.substr(pos, pos2-pos);

					inEnum = false;
					EnumInfo einfo;
					einfo.categoryName = m_className + "::" + enumname;
					einfo.count = enumIdx;
					einfo.enumNUM = enumName;
					m_enumInfo.push_back(einfo);
					// remember NUM_ dummy keyword to be used to determine corrent enum for read/write blocks
//					std::cout << enumName << ":" << enumIdx << std::endl;
					continue;
				}
			}

			// now check for xml read/write annotations
			pos = line.find("// XML:");
			if (pos != std::string::npos && pos+7 < line.size()) {
				// line has format: <type> m_<varName>; // XML:<specs>
				std::string::size_type pos2 = line.find_first_not_of(" \t");
				std::string typeDeclaration = line.substr(pos2, pos-pos2);
				// split type declaration at the "m_"
				pos2 = typeDeclaration.find("m_");
				if (pos2 == std::string::npos) {
					std::cerr << "XML-spec unsupported in this line or bad format of variable declaration\n"
							  << "in line '" << line << "'" << std::endl;
					throw std::runtime_error("error");
				}

				std::string typeStr = typeDeclaration.substr(0, pos2);
				IBK::trim(typeStr);

				std::string varName = typeDeclaration.substr(pos2+2); // without m_
				IBK::trim(varName, " \t;");
				// varName may now be of format "m_path = IBK::Path("/tmp")" - we need to split the assignment operator
				// for C++11 support
				std::string::size_type equalPos = varName.find("=");
				if (equalPos != std::string::npos)
					varName = IBK::trim_copy(varName.substr(0, equalPos));

				std::string xmlSpec = line.substr(pos+7);
				// we support only a subset of type declarations
//				std::cout << typeStr << " : " << varName << " : " << xmlSpec << std::endl;

				// explode XML-spec into tokens
				std::vector<std::string> tokens;
				IBK::explode(xmlSpec, tokens, ":", IBK::EF_NoFlags);

				// store xml spec
				XMLInfo xmlInfo;
				// first token must be either A or E
				if (tokens[0] == "A")
					xmlInfo.element = false;
				else if (tokens[0] == "E")
					xmlInfo.element = true;
				else {
					std::cerr << "Invalid XML-spec '"<< xmlSpec << "' in line '"<< line << "'" << std::endl;
					throw std::runtime_error("error");
				}
				xmlInfo.typeStr = typeStr;
				xmlInfo.varName = varName;

				// process flags
				for (unsigned int i=1; i<tokens.size(); ++i) {
					if (tokens[i] == "required") {
						xmlInfo.required = true;
					}
					else {
						std::cerr << "Invalid XML-spec '"<< xmlSpec << "' in line '"<< line << "'" << std::endl;
						throw std::runtime_error("error");
					}
				}

				m_xmlInfo.push_back(xmlInfo);
			}


		} catch (...) {
			std::cerr << "Parse error in line '"<< line << "'" << std::endl;
		}
	}
	return true;
}


void ClassInfo::extractToken(std::string & kwpara, char delimiter, std::string & token) {
	// look for delimiter from end of string

	char rightDelim = delimiter;
	switch (delimiter) {
		case '<' : rightDelim = '>'; break;
		case '[' : rightDelim = ']'; break;
		case '{' : rightDelim = '}'; break;
	}
	std::string::size_type pos = kwpara.rfind(rightDelim);
	if (pos == std::string::npos)
		return; // no delim found

	std::string::size_type pos2 = kwpara.rfind(delimiter, pos-1);
	if (pos2 == std::string::npos)
		return; // no second delim found

	// extract string between delimiters
	token = kwpara.substr(pos2+1, pos-pos2-1);
	IBK::trim(token);
	kwpara = kwpara.substr(0, pos2);
	IBK::trim(kwpara);
}



