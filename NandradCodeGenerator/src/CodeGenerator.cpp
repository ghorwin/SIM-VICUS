#include "CodeGenerator.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <limits>
#include <map>

#ifdef WIN32

#undef UNICODE
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Windows.h>
#include <cstring>

#else // WIN32

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#endif // WIN32

#include <IBK_messages.h>
#include <IBK_assert.h>

#include "constants.h"

bool listHeaders(const std::string & dir, std::vector<std::string> & files);

bool write_keyword_data(const std::string& output_file, const std::vector<ClassInfo::Keyword>& keywordlist);


void CodeGenerator::handleArguments(const char * const argv[]) {
	// first argument is namespace
	m_namespace = argv[1];
	// second argument is encoded list of input directories
	std::string src_dir = argv[2];
	std::replace(src_dir.begin(), src_dir.end(), '\\', '/');

	std::string part;

	// extract ":" separated folders
	size_t pos, lastPos;
	while ( (pos = src_dir.find("+")) != std::string::npos ) {
		lastPos = 0;
		part = src_dir.substr( lastPos, (pos-lastPos) );
		if (part.empty())
			continue; // skip empty parts (i.e. src_dir starts with a ;)
		if (part[part.size()-1] == '/')
			part = part.substr(0, part.size()-1);
		lastPos = pos+1;
		m_inputDirectories.push_back(part);
		src_dir = src_dir.substr( lastPos );
	}

	// add single or last directory
	if (src_dir.find_first_not_of(" \t") != std::string::npos)
		m_inputDirectories.push_back(src_dir);

	if (m_inputDirectories.empty()) {
		m_inputDirectories.push_back(".");
	}

	// now all source dir are stored in the vector of src directories

	// third argument is path to tr() version
	m_isQtOutputRequired = false;
	std::string isOt = argv[3];
	if ( isOt == "1")
		m_isQtOutputRequired = true;

	// fourth argument is file prefix
	m_prefix = argv[4];

	// target directory for ncg files
	m_ncgOutputDir = argv[5];
}


bool CodeGenerator::parseDirectories() {
	FUNCID(CodeGenerator::parseDirectories);

	// parse all directories
	unsigned int lastCount = 0;
	for (std::vector< std::string >::const_iterator it = m_inputDirectories.begin(),
		end = m_inputDirectories.end();
		it != end;
		++it)
	{
		// get list of header files
		std::vector< std::string> hfiles;
		if (!listHeaders(*it, hfiles)) {
			IBK::IBK_Message("Please check the supplied list of input directories!", IBK::MSG_ERROR, FUNC_ID);
			return false; // something wrong with the input directories
		}

		try {
			// parse files

			std::cout << "Parsing " << hfiles.size() << " header files" << std::endl;
			for (const std::string & h : hfiles) {
				ClassInfo c;
				if (!c.parse(IBK::Path(*it) / h)) {
					std::cerr << "Error while parsing input files." << std::endl;
					return false;
				}
				m_classInfo.push_back(c);
				m_keywordlist.insert(m_keywordlist.end(), c.m_keywords.begin(), c.m_keywords.end());
			}
			std::cout << m_keywordlist.size() - lastCount << " keywords in " << *it << std::endl;
			lastCount = m_keywordlist.size();
		}
		catch (std::exception & ex) {
			std::cerr << ex.what();
			return false;
		}

	} // for all directories in list

	// we need to do some processing here

#if 0
	// some enums may not have a NUM_ line, so we manually need to determine the maximum index/enum count by
	// counting the keywords for these category
	// we do this by iterating over all keywords, trying to find the matching enuminfo - and if missing, we add it
	// to the list
	for (const ClassInfo::Keyword & kw : m_keywordlist) {
		// get max count from enum info
		const ClassInfo::EnumInfo * einfoptr = enumInfo(kw.category);
		if (einfoptr==nullptr) {
			IBK::IBK_Message(IBK::FormatString("Adding enum info for '%1'\n").arg(kw.category), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			ClassInfo::EnumInfo einfo;
			einfo.count = 0; // for now
			einfo.categoryName = kw.category;
			einfo.enumNUM = ""; // does not exist
			insertEnumInfo(einfo);
			einfoptr = enumInfo(kw.category);
		}
		// update enum info
		if (einfoptr->count < kw.index+1)
			const_cast<ClassInfo::EnumInfo *>(einfoptr)->count = kw.index+1;
	}
#endif

	return true;
}


void CodeGenerator::generateKeywordList() {
	FUNCID(CodeGenerator::generateKeywordList);

	CPP_HEADER = CPP_HEADER_TEMPLATE;
	HEADER = HEADER_TEMPLATE;

	QT_CPP_HEADER = QT_CPP_HEADER_TEMPLATE;
	QT_CPP_FOOTER = QT_CPP_FOOTER_TEMPLATE;
	QT_HEADER = QT_HEADER_TEMPLATE;

	// replace two occurrances of namespace placeholder
	CPP_HEADER.replace(CPP_HEADER.find("${NAMESPACE}"),12, m_namespace);
	CPP_HEADER.replace(CPP_HEADER.find("${PREFIX}"),9, m_prefix);

	// replace 6 namespace placeholder
	HEADER.replace(HEADER.find("${NAMESPACE}"),12, m_namespace);
	HEADER.replace(HEADER.find("${NAMESPACE}"),12, m_namespace);
	HEADER.replace(HEADER.find("${PREFIX}"),9, m_prefix);
	HEADER.replace(HEADER.find("${PREFIX}"),9, m_prefix);
	HEADER.replace(HEADER.find("${PREFIX}"),9, m_prefix);
	HEADER.replace(HEADER.find("${PREFIX}"),9, m_prefix);


	// replace two occurrances of namespace placeholder
	QT_HEADER.replace(QT_HEADER.find("${NAMESPACE}"),12, m_namespace);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, m_prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, m_prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, m_prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, m_prefix);
	QT_HEADER.replace(QT_HEADER.find("${PREFIX}"),9, m_prefix);
	QT_CPP_HEADER.replace(QT_CPP_HEADER.find("${NAMESPACE}"),12, m_namespace);
	QT_CPP_HEADER.replace(QT_CPP_HEADER.find("${PREFIX}"),9, m_prefix);

	// write qt header
	if ( m_isQtOutputRequired ) {
		IBK::Path outputPath(m_inputDirectories[0] + "/../srcTranslations");
		if (!outputPath.exists())
			throw IBK::Exception(IBK::FormatString("Output directory '%1' doesn't exist.").arg(outputPath), FUNC_ID);
		IBK::Path fnameQt = outputPath / (m_prefix +"_KeywordListQt.h");
		if (!fnameQt.exists())
			generateHeaderCode( fnameQt, true );
	}

	// write header
	IBK::Path fname = IBK::Path(m_inputDirectories[0]) / (m_prefix +"_KeywordList.h");
	if (!fname.exists())
		generateHeaderCode( fname, false );

	// now process all source directories and parse input files

	try {
		// generate code
		IBK::Path fnameCPP = IBK::Path(m_inputDirectories[0]) / (m_prefix +"_KeywordList.cpp");
		generateKeywordlistCode(fnameCPP);
		// generate qt code
		if ( m_isQtOutputRequired ){
			IBK::Path outputPath(m_inputDirectories[0] + "/../srcTranslations");
			IBK::Path fnameQt = outputPath / m_prefix +"_KeywordListQt.cpp";
			generateKeywordlistCodeQt(fnameQt);
		}
		std::cout << m_keywordlist.size() << " keywords extracted!" << std::endl;
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error generating keyword list.", FUNC_ID);
	}
	catch (std::exception & ex) {
		std::cerr << ex.what() << std::endl;
		throw IBK::Exception("Error generating keyword list.", FUNC_ID);
	}
}


void CodeGenerator::generateReadWriteCode() {
	FUNCID(CodeGenerator::generateReadWriteCode);

	// the algorithm works like the following

	// - process all class infos objects
	// - skip all that do not require read/write stuff -> if no, skip
	// - check for existance of target file
	// - if it exists, check if file date is newer than h file -> if yes, skip
	// - if parent dir doesn't exist -> create it
	// - start writing file by writing header
	// - write writeXML() or writeXMLPrivate() function
	for (const ClassInfo & ci : m_classInfo) {
		try {

			// skip files where code generation is not necessary
			if (ci.m_xmlInfo.empty() && !ci.m_requireComparisonFunction)
				continue;

#if 0
			// development hack - only deal with SerializationTest for now
			if (ci.m_className != "SerializationTest")
				continue;
#endif

			// compose path to target file
			IBK::Path parentDir;
			if (m_ncgOutputDir.isAbsolute())
				parentDir = m_ncgOutputDir;
			else
				parentDir = ci.m_sourceHeaderFile.parentPath() / m_ncgOutputDir;
			IBK::Path targetFile = parentDir / "ncg_" + ci.m_sourceHeaderFile.filename().withoutExtension().str() + ".cpp";

			// check if it exists
			if (targetFile.exists()) {
#define DISABLE_TIME_CHECK
#ifndef DISABLE_TIME_CHECK
				// skip, if target file is already newer than source file
				if (targetFile.fileTime() > ci.m_sourceHeaderFile.fileTime()) {
					IBK::IBK_Message(IBK::FormatString("Target file '%1' is newer than '%2', skipped.\n")
									 .arg(targetFile).arg(ci.m_sourceHeaderFile), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
					continue;
				}
#endif // DISABLE_TIME_CHECK
			}

			// check if parent dir exits
			if (!parentDir.exists()) {
				if (!IBK::Path::makePath(parentDir))
					throw IBK::Exception(IBK::FormatString("Cannot create target path '%1'.").arg(parentDir), FUNC_ID);
			}

			// now start writing the file
			std::string writeCode;
			std::set<std::string> includes;

			// *** Generate writeXML() content ****

			writeCode += IBK::replace_string(CPP_WRITEXML, "${CLASSNAME}", ci.m_className);
			if (ci.m_requirePrivateReadWrite)
				writeCode = IBK::replace_string(writeCode, "${PRIVATE}", "Private");
			else
				writeCode = IBK::replace_string(writeCode, "${PRIVATE}", "");

			std::string attribs;
			// generate attribute write code
			for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
				if (xmlInfo.element) continue;
				std::string attribName = xmlInfo.varName;
				// plain data types are always just written as attributes
				if (xmlInfo.typeStr == "int" ||
					xmlInfo.typeStr == "unsigned int" ||
					xmlInfo.typeStr == "double" ||
					xmlInfo.typeStr == "bool")
				{
					attribs += "	e->SetAttribute(\""+attribName+"\", IBK::val2string<"+xmlInfo.typeStr+">(m_"+attribName+"));\n";
				}
				else if (xmlInfo.typeStr == "IBK::Path") {
					if (xmlInfo.notEmpty) {
						attribs += "	if (m_" + attribName + ".isValid())\n	";
					}
					attribs += "	e->SetAttribute(\""+attribName+"\", m_" + attribName + ".str());\n";
				}
				else if (xmlInfo.typeStr == "std::string") {
					if (xmlInfo.notEmpty) {
						attribs += "	if (!m_" + attribName + ".empty())\n	";
					}
					attribs += "	e->SetAttribute(\""+attribName+"\", m_" + attribName + ");\n";
				}
				else if (xmlInfo.typeStr == "IBK::Unit") {
					attribs += "	e->SetAttribute(\""+attribName+"\", m_" + attribName + ".name());\n";
				}
				else {
					// check for enum types
					bool hadEnumType = false;
					for (const ClassInfo::EnumInfo & einfo : ci.m_enumInfo) {
						if (einfo.enumType() == xmlInfo.typeStr) {
							hadEnumType = true;
							// generate write code for enum type
							attribs +=
								"	if (m_" + attribName + " != "+einfo.enumNUM+")\n"
								"		e->SetAttribute(\""+attribName+"\", KeywordList::Keyword(\""+ einfo.categoryName + "\",  m_" + attribName + "));\n";
							includes.insert("NANDRAD_KeywordList.h");
						}
					}
					if (hadEnumType) continue;

					// other special cases

					throw IBK::Exception(IBK::FormatString("(Still) unsupported XML variable type '%1' of variable '%2'.")
										 .arg(xmlInfo.typeStr).arg(xmlInfo.varName), FUNC_ID);
				}
			}



			// now the elements
			std::string elements;

			for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
				if (!xmlInfo.element) continue;
				std::string varName = xmlInfo.varName;
				std::string tagName = char(toupper(varName[0])) + varName.substr(1);
				// we have special handling for:
				// - simple tags (no attributes) for PODs, int, unsigned int, double, bool
				// - std::string
				// - IBK::Path
				// - IBK::Parameter
				// - IBK::Flag
				// - IBK::Parameter[NUM_xxx]
				// - IBK::IntPara[NUM_xxx]
				// - std::vector<xxx>
				// - enumTypes

				if (xmlInfo.typeStr == "int" ||
					xmlInfo.typeStr == "unsigned int" ||
					xmlInfo.typeStr == "double" ||
					xmlInfo.typeStr == "bool")
				{
					elements += "	TiXmlElement::appendSingleAttributeElement(e, \""+tagName+"\", nullptr, std::string(), IBK::val2string<"+xmlInfo.typeStr+">(m_"+varName+"));\n";
				}
				else if (xmlInfo.typeStr == "std::string") {
					if (xmlInfo.notEmpty) {
						elements += "	if (!m_" + varName + ".empty())\n	";
					}
					elements += "	TiXmlElement::appendSingleAttributeElement(e, \""+tagName+"\", nullptr, std::string(), m_"+varName+");\n";
				}
				else if (xmlInfo.typeStr == "IBK::Unit") {
					elements += "	TiXmlElement::appendSingleAttributeElement(e, \""+tagName+"\", nullptr, std::string(), m_"+varName+".name());\n";
				}
				else if (xmlInfo.typeStr == "IBK::Path") {
					elements += "	TiXmlElement::appendSingleAttributeElement(e, \""+tagName+"\", nullptr, std::string(), m_"+varName+".str());\n";
				}
				else if (xmlInfo.typeStr == "IBK::LinearSpline") {
					includes.insert("NANDRAD_Utilities.h");
					elements += "	writeLinearSplineElement(e, \""+tagName+"\", m_"+varName+", std::string(), std::string());\n";
				}
				else if (xmlInfo.typeStr == "IBK::Parameter") {
					// check for array syntax
					std::string::size_type pos1 = varName.find("[");
					if (pos1 != std::string::npos) {
						// extract NUM type
						std::string::size_type pos2 = varName.find("]");
						std::string numType = varName.substr(pos1+1, pos2-pos1-1);
						varName = varName.substr(0, pos1);

						elements += "\n"
							"	for (unsigned int i=0; i<"+numType+"; ++i) {\n"
							"		if (!m_"+varName+"[i].name.empty())\n"
							"			TiXmlElement::appendIBKParameterElement(e, m_"+varName+"[i].name, m_"+varName+"[i].IO_unit.name(), m_"+varName+"[i].get_value());\n"
							"	}\n";
					}
					else {
						elements += "	TiXmlElement::appendIBKParameterElement(e, m_"+varName+".name, m_"+varName+".IO_unit.name(), m_"+varName+".get_value());\n";
					}
				}
				else if (xmlInfo.typeStr == "IBK::IntPara") {
					// check for array syntax
					std::string::size_type pos1 = varName.find("[");
					if (pos1 != std::string::npos) {
						// extract NUM type
						std::string::size_type pos2 = varName.find("]");
						std::string numType = varName.substr(pos1+1, pos2-pos1-1);
						varName = varName.substr(0, pos1);

						elements += "\n"
							"	for (unsigned int i=0; i<"+numType+"; ++i) {\n"
							"		if (!m_"+varName+"[i].name.empty())\n"
							"			TiXmlElement::appendSingleAttributeElement(e, \"IBK:IntPara\", \"name\", m_"+varName+"[i].name, IBK::val2string(m_"+varName+"[i].value));\n"
							"	}\n";
					}
					else {
						elements += "	TiXmlElement::appendSingleAttributeElement(e, \"IBK:IntPara\", \"name\", m_"+varName+".name, IBK::val2string(m_"+varName+".value));\n";
					}
				}
				else if (xmlInfo.typeStr == "IBK::Flag") {
					// check for array syntax
					std::string::size_type pos1 = varName.find("[");
					if (pos1 != std::string::npos) {
						// extract NUM type
						std::string::size_type pos2 = varName.find("]");
						std::string numType = varName.substr(pos1+1, pos2-pos1-1);
						varName = varName.substr(0, pos1);
						elements += "\n"
							"	for (int i=0; i<"+numType+"; ++i) {\n"
							"		if (!m_"+varName+"[i].name().empty())\n"
							"			TiXmlElement::appendSingleAttributeElement(e, \"IBK:Flag\", \"name\", m_"+varName+"[i].name(), m_"+varName+"[i].isEnabled() ? \"true\" : \"false\");\n"
							"	}\n";
					}
					else {
						elements += "	TiXmlElement::appendSingleAttributeElement(e, \"IBK:Flag\", \"name\", m_"+varName+".name(), m_"+varName+".isEnabled() ? \"true\" : \"false\");\n";
					}
				}
				else if (xmlInfo.typeStr.find("std::vector<") == 0) {
					// extract subtype
					std::string::size_type pos1 = xmlInfo.typeStr.find("<");
					std::string::size_type pos2 = xmlInfo.typeStr.find(">");
					std::string childType = xmlInfo.typeStr.substr(pos1+1, pos2-pos1-1);
					// we generate the parent element and afterwards the loop
					elements += "\n"
							"	if (!m_"+varName+".empty()) {\n"
							"		TiXmlElement * child = new TiXmlElement(\""+tagName+"\");\n"
							"		e->LinkEndChild(child);\n"
							"\n"
							"		for (std::vector<"+childType+">::const_iterator ifaceIt = m_"+varName+".begin();\n"
							"			ifaceIt != m_"+varName+".end(); ++ifaceIt)\n"
							"		{\n"
							"			ifaceIt->writeXML(child);\n"
							"		}\n"
							"	}\n"
							"\n";

				}
				else {
					bool hadEnumType = false;
					for (const ClassInfo::EnumInfo & einfo : ci.m_enumInfo) {
						if (einfo.enumType() == xmlInfo.typeStr) {
							hadEnumType = true;
							// generate write code for enum type

							elements += "\n"
										"	if (m_" + varName + " != "+einfo.enumNUM+")\n"
										"		TiXmlElement::appendSingleAttributeElement(e, \""+tagName+"\", nullptr, std::string(), KeywordList::Keyword(\""+ einfo.categoryName + "\",  m_" + varName + "));\n";

							includes.insert("NANDRAD_KeywordList.h");
						}
					}
					if (hadEnumType) continue;

					// we now assume a complex type, i.e. "Interface" where we can call writeXML() ourselves.

					// TODO : add check for "not-default"
					elements += "\n	m_" + varName + ".writeXML(e);\n";
				}
			}


			// insert code into writeXML() function block
			writeCode = IBK::replace_string(writeCode, "${ATTRIBUTES}", attribs, IBK::ReplaceFirst);
			writeCode = IBK::replace_string(writeCode, "${CHILD_ELEMENTS}", elements, IBK::ReplaceFirst);



			// *** Generate readXML() content ****

			std::string readCode;
			readCode += IBK::replace_string(CPP_READXML, "${CLASSNAME}", ci.m_className);
			if (ci.m_requirePrivateReadWrite)
				readCode = IBK::replace_string(readCode, "${PRIVATE}", "Private");
			else
				readCode = IBK::replace_string(readCode, "${PRIVATE}", "");

			attribs.clear();
			elements.clear();

			// check if we have any attributes to read
			bool haveAttribs = false;
			for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
				if (xmlInfo.element) continue;
				haveAttribs = true;
				break;
			}

			// generate code for attributes
			std::string elseStr;

			if (haveAttribs) {
				for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
					if (xmlInfo.element || !xmlInfo.required) continue;
					std::string attribName = xmlInfo.varName;

					attribs +=
						"		// search for mandatory attributes\n"
						"		if (!TiXmlAttribute::attributeByName(element, \""+attribName+"\"))\n"
						"			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(\n"
						"				IBK::FormatString(\"Missing required '"+attribName+"' attribute.\") ), FUNC_ID);\n"
						"\n";
				}

				attribs +=
						"		const TiXmlAttribute * attrib = element->FirstAttribute();\n"
						"		while (attrib) {\n"
						"			const std::string & attribName = attrib->NameStr();\n";

				for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
					if (xmlInfo.element) continue;
					std::string attribName = xmlInfo.varName;
					attribs += "			"+elseStr+"if (attribName == \""+attribName+"\")\n";
					// now type specific code
					if (xmlInfo.typeStr == "int" ||
						xmlInfo.typeStr == "unsigned int" ||
						xmlInfo.typeStr == "double" ||
						xmlInfo.typeStr == "bool")
					{
						attribs +=
							"				m_"+attribName+" = readPODAttributeValue<"+xmlInfo.typeStr+">(element, attrib);\n";
						includes.insert("NANDRAD_Utilities.h");
					}
					else if (xmlInfo.typeStr == "IBK::Path") {
						attribs +=
							"				m_"+attribName+" = IBK::Path(attrib->ValueStr());\n";
					}
					else if (xmlInfo.typeStr == "IBK::Unit") {
						attribs +=
							"				try {\n"
							"					m_"+attribName+" = IBK::Unit(attrib->ValueStr());\n"
							"				}\n"
							"				catch (IBK::Exception & ex) {\n"
							"					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(\n"
							"						IBK::FormatString(\"Error converting '\"+attrib->ValueStr()+\"' attribute (unknown unit).\") ), FUNC_ID);\n"
							"				}\n";
					}
					else if (xmlInfo.typeStr == "std::string") {
						attribs +=
							"				m_"+attribName+" = attrib->ValueStr();\n";
					}
					else {
						// check for enum types
						bool hadEnumType = false;
						for (const ClassInfo::EnumInfo & einfo : ci.m_enumInfo) {
							if (einfo.enumType() == xmlInfo.typeStr) {
								hadEnumType = true;
								// generate write code for enum type
								attribs +=
									"			try {\n"
									"				m_"+attribName+" = ("+einfo.enumType()+")KeywordList::Enumeration(\""+einfo.categoryName+"\", attrib->ValueStr());\n"
									"			}\n"
									"			catch (IBK::Exception & ex) {\n"
									"				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(\n"
									"					IBK::FormatString(\"Invalid or unknown keyword '\"+attrib->ValueStr()+\"'.\") ), FUNC_ID);\n"
									"			}\n";
								includes.insert("NANDRAD_KeywordList.h");
							}
						}
						if (hadEnumType) continue;
						throw IBK::Exception(IBK::FormatString("(Still) unsupported XML attrib type '%1' for variable '%2'.")
											 .arg(xmlInfo.typeStr).arg(xmlInfo.varName), FUNC_ID);
					}
					elseStr = "else ";
				} // end attribute reading loop
				attribs +=
						"			else {\n"
						"				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);\n"
						"			}\n"
						"			attrib = attrib->Next();\n"
						"		}\n";
			}

			// check if we have any attributes to read
			bool haveTags = false;
			for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
				if (!xmlInfo.element) continue;
				haveTags = true;
				break;
			}

			haveTags = false;
			if (haveTags) {
				// now read-code for elements/tags
				for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
					if (!xmlInfo.element || !xmlInfo.required) continue;
					std::string varName = xmlInfo.varName;
					std::string tagName = char(toupper(varName[0])) + varName.substr(1);

					elements +=
						"		// search for mandatory elements\n"
						"		if (!element->FirstChildElement(\""+tagName+"\")\n"
						"			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(\n"
						"				IBK::FormatString(\"Missing required '"+tagName+"' element.\") ), FUNC_ID);\n"
						"\n";
				}

				elements +=
						"		const TiXmlElement * c = element->FirstChildElement();\n"
						"		while (c) {\n"
						"			const std::string & cName = c->ValueStr();\n";

				// first scan for all group-type xml tags
				std::set<std::string> groupTags;
				for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
					if (!xmlInfo.element) continue;
					if (xmlInfo.typeStr == "IBK::Parameter" ||
						xmlInfo.typeStr == "IBK::LinearSpline" ||
						xmlInfo.typeStr == "IBK::Flag" ||
						xmlInfo.typeStr == "IBK::IntPara")
					{
						groupTags.insert(xmlInfo.typeStr);
					}
				}

				elseStr.clear();
				for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
					if (!xmlInfo.element) continue;
					std::string varName = xmlInfo.varName;
					std::string tagName = char(toupper(varName[0])) + varName.substr(1);
					// now type specific code
					if (xmlInfo.typeStr == "int" ||
						xmlInfo.typeStr == "unsigned int" ||
						xmlInfo.typeStr == "double" ||
						xmlInfo.typeStr == "bool")
					{
						includes.insert("NANDRAD_Utilities.h");
						elements +=
							"			"+elseStr+"if (cName == \""+tagName+"\")\n"
							"				m_"+varName+" = readPODElement<"+xmlInfo.typeStr+">(c, cName);\n";
					}
					else if (xmlInfo.typeStr == "std::string") {
						elements +=
							"			"+elseStr+"if (cName == \""+tagName+"\")\n"
							"				m_"+varName+" = c->GetText();\n";
					}
					else if (xmlInfo.typeStr == "IBK::Unit") {
						includes.insert("NANDRAD_Utilities.h");
						elements +=
							"			"+elseStr+"if (cName == \""+tagName+"\")\n"
							"				m_"+varName+" = readUnitElement(c, cName);\n";
					}
					else if (xmlInfo.typeStr == "IBK::Path") {
						elements +=
							"			"+elseStr+"if (cName == \""+tagName+"\")\n"
							"				m_"+varName+" = IBK::Path(c->GetText());\n";
					}

					// for the group types, we generate the code only once and switch the parameter in an internal loop

					else if (xmlInfo.typeStr == "IBK::LinearSpline" && groupTags.find("IBK::LinearSpline") != groupTags.end()) {
						groupTags.erase(groupTags.find("IBK::LinearSpline")); // only generate code once
						includes.insert("NANDRAD_Utilities.h");

						elements +=
							"			"+elseStr+"if (cName == \"IBK:LinearSpline\") {\n"
							"				IBK::LinearSpline spl;\n"
							"				std::string name;\n"
							"				readLinearSplineElement(c, cName, spl, name, nullptr, nullptr);\n";
						// now loop all linear spline variables and generate code for assigning these
						std::string caseElse;
						for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
							if (!xmlInfo.element || xmlInfo.typeStr != "IBK::LinearSpline") continue;
							elements += "				"+caseElse+"if (name == \""+tagName+"\")		m_"+varName+" = spl;\n";
							caseElse = "else ";
						}
						elements +=
							"				else {\n"
							"					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(name).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);\n"
							"				}\n"
							"			}\n";
					}
					else if (xmlInfo.typeStr == "IBK::Parameter" && groupTags.find("IBK::Parameter") != groupTags.end()) {
						groupTags.erase(groupTags.find("IBK::Parameter")); // only generate code once
						includes.insert("NANDRAD_Utilities.h");

						elements +=
							"			"+elseStr+"if (cName == \"IBK:Parameter\") {\n"
							"				IBK::Parameter p;\n"
							"				readParameterElement(c, cName, p);\n";
						// now loop all IBK::Parameter variables and generate code for assigning these
						std::string caseElse;
						for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
							if (!xmlInfo.element || xmlInfo.typeStr != "IBK::Parameter") continue;
							// now determine if this is a scalar parameter or a para[xxx] variant
							elements +=	"				"+caseElse+"if (p.name == \""+tagName+"\") {\n";
							elements +=		"				}\n";
							caseElse = "else ";
						}
						elements +=
							"				else {\n"
							"					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);\n"
							"				}\n"
							"			}\n";
					}
					else if (xmlInfo.typeStr == "IBK::IntPara" && groupTags.find("IBK::IntPara") != groupTags.end()) {
						groupTags.erase(groupTags.find("IBK::IntPara")); // only generate code once
						includes.insert("NANDRAD_Utilities.h");

						elements +=
							"			"+elseStr+"if (cName == \"IBK:IntPara\") {\n"
							"				IBK::Parameter p;\n"
							"				readParameterElement(c, cName, p);\n";
						// now loop all IBK::Parameter variables and generate code for assigning these
						std::string caseElse;
						for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
							if (!xmlInfo.element || xmlInfo.typeStr != "IBK::Parameter") continue;
							// now determine if this is a scalar parameter or a para[xxx] variant
							elements +=	"				"+caseElse+"if (p.name == \""+tagName+"\") {\n";
							elements +=		"				}\n";
							caseElse = "else ";
						}
						elements +=
							"				else {\n"
							"					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);\n"
							"				}\n"
							"			}\n";
					}
					else if (xmlInfo.typeStr == "IBK::Flag" && groupTags.find("IBK::Flag") != groupTags.end()) {
						groupTags.erase(groupTags.find("IBK::Flag")); // only generate code once
						includes.insert("NANDRAD_Utilities.h");

						elements +=
							"			"+elseStr+"if (cName == \"IBK:Flag\") {\n"
							"				IBK::Flag f;\n"
							"				readFlagElement(c, cName, f);\n";
						// now loop all IBK::Flag variables and generate code for assigning these
						std::string caseElse;
						for (const ClassInfo::XMLInfo & xmlInfo : ci.m_xmlInfo) {
							if (!xmlInfo.element || xmlInfo.typeStr != "IBK::Flag") continue;
							// now determine if this is a scalar parameter or a flag[xxx] variant
							elements +=	"				"+caseElse+"if (f.name() == \""+tagName+"\") {\n";
							elements +=		"				}\n";
							caseElse = "else ";
						}
						elements +=
							"				else {\n"
							"					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(f.name()).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);\n"
							"				}\n"
							"			}\n";
					}
					else {
						IBK::IBK_Message(IBK::FormatString("(Still) unsupported XML attrib type '%1' for variable '%2'.").arg(xmlInfo.typeStr).arg(xmlInfo.varName), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
						continue;
					}
					elseStr = "else ";
				} // end element reading loop
				elements +=
						"			else {\n"
						"				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);\n"
						"			}\n"
						"			c = c->NextSiblingElement();\n"
						"		}\n";
			}

			// insert code into readXML() function block
			readCode = IBK::replace_string(readCode, "${ATTRIBUTES}", attribs, IBK::ReplaceFirst);
			readCode = IBK::replace_string(readCode, "${CHILD_ELEMENTS}", elements, IBK::ReplaceFirst);


			// *** Add header and footer and write file ****

			std::string extraIncludes;
			for (const std::string & inc : includes)
				extraIncludes += "#include <"+inc+">\n";

			std::string fileHeader = IBK::replace_string(CPP_READWRITE_HEADER, "${HEADER_FILE}", ci.m_sourceHeaderFile.filename().str(), IBK::ReplaceFirst);
			fileHeader = IBK::replace_string(fileHeader, "${OTHER_INCLUDES}", extraIncludes, IBK::ReplaceFirst);

			IBK::IBK_Message(IBK::FormatString("Generating file '%1'.\n").arg(targetFile), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			std::ofstream out(targetFile.c_str(), std::ios_base::trunc);
			out << fileHeader;
			out << readCode << std::endl;
			out << writeCode << std::endl;

			out << "} // namespace NANDRAD\n";
			out.close();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error generating code for file '%1'").arg(ci.m_sourceHeaderFile.filename()), FUNC_ID);
		}
	}
}



// ********* Private Utility functions **************

bool CodeGenerator::listHeaders(const std::string & dir, std::vector<std::string> & files) {
#ifdef WIN32
	WIN32_FIND_DATA fd;
	std::string tmpdir = dir + "/*";
	HANDLE fh = FindFirstFile((LPCSTR)tmpdir.c_str(), &fd);
	if (fh == INVALID_HANDLE_VALUE) {
		std::cerr << "Cannot open/read directory: '"<< dir<< "'" << std::endl;
		return false;
	}
	do {
		std::string f = fd.cFileName;
		if (f.size() > 2 && f.find(".h") == f.size()-2)
			files.push_back(f);
	}
	while(FindNextFile(fh, &fd));
	FindClose(fh);
#else
	DIR *dp;
	struct dirent *ep;
	dp = opendir(dir.c_str());
	if (dp != NULL) {
		while ( (ep = readdir (dp)) ) {
			std::string f = ep->d_name;
			if (f.size() > 2 && f.find(".h") == f.size()-2)
				files.push_back(f);
		}
		closedir(dp);
	}
	else {
		std::cerr << "Cannot open/read directory: '"<< dir<< "'" << std::endl;
	}
#endif // WIN32
	std::sort(files.begin(), files.end());
	return true;
}


// ********* Code Generation Functions **************

void CodeGenerator::generateHeaderCode(const IBK::Path & headerFilePath, bool qtHeader) const {
	FUNCID(CodeGenerator::generateHeaderCode);
	std::ofstream hpp(headerFilePath.c_str());
	if (!hpp)
		throw IBK::Exception(IBK::FormatString("Cannot create '%1'.").arg(headerFilePath), FUNC_ID);
	std::cout << "Writing '"<< headerFilePath.str() << "'" << std::endl;
	// write file header
	if (qtHeader){
		hpp << QT_HEADER;
	} else {
		hpp << HEADER;
	}
}


void CodeGenerator::generateKeywordlistCode(const IBK::Path & keywordListCpp) {
	FUNCID(CodeGenerator::generateKeywordlistCode);
	// first we generate the header file
	std::ofstream cpp(keywordListCpp.c_str());
	if (!cpp)
		throw IBK::Exception(IBK::FormatString("Cannot create '%1'.").arg(keywordListCpp), FUNC_ID);
	std::cout << "Writing '"<< keywordListCpp.str() << "'" << std::endl;
	// write file header
	cpp << CPP_HEADER;
	cpp <<
		"	/*! Holds the string to print as error when an invalid keyword is encountered. */\n"
		"	const char * const INVALID_KEYWORD_INDEX_STRING = \"KEYWORD_ERROR_STRING: Invalid type index\";\n\n";

	// get category list
	std::string lastCategory;
	std::vector<std::string> catnames;
	for (unsigned int i=0; i<m_keywordlist.size(); ++i) {
		// new category = new declaration
		if (m_keywordlist[i].category != lastCategory) {
			lastCategory = m_keywordlist[i].category;
			catnames.push_back(lastCategory);
		}
	}
	if (catnames.empty())
		throw IBK::Exception("No categories extracted!", FUNC_ID);

	// write category array
	cpp <<
		"	/*! Holds a list of all enum types/categories. */\n"
		"	const char * const ENUM_TYPES["<< catnames.size() << "] = {\n";
	for (unsigned int i=0; i<catnames.size()-1; ++i) {
		cpp << "\t\t\"" + catnames[i] + "\",\n";
	}
	cpp << "\t\t\"" + catnames.back() + "\"\n";
	cpp << "\t};\n\n";

	// write middle part
	cpp <<
		"	/*! Converts a category string to respective enumeration value. */\n"
		"	int enum2index(const std::string & enumtype) {\n"
		"		for (int i=0; i<"<< catnames.size() << "; ++i) {\n"
		"			if (enumtype == ENUM_TYPES[i]) return i;\n"
		"		}\n"
		"		//std::cerr << \"Unknown enumeration type '\" << enumtype<< \"'.\" << std::endl;\n"
		"		return -1;\n"
		"	}\n"
		"	\n\n";

	// *** write theKeyword() function ***
	cpp <<
		"	/*! Returns a keyword string for a given category (typenum) and type number t. */\n"
		"	const char * theKeyword(int typenum, int t) {\n"
		"		switch (typenum) {\n";

	lastCategory.clear();
	int switchIndex = 0;
//	int localSwitchIndex = 0;
	for (unsigned int i=0; i<m_keywordlist.size(); ++i) {
		// new category = new switch clause
		if (m_keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break;\n";
				++switchIndex;
			}
			lastCategory = m_keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<"\n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) {\n";
		}
		std::string kw = m_keywordlist[i].keyword;
		size_t space = kw.find_first_of(" \t");
		if (space != std::string::npos)
			kw = kw.substr(0,space);
		cpp << "				case " << m_keywordlist[i].index << " : return \"" << kw << "\";\n";
	}
	// close last switch clause and close switch statement
	cpp << "			} break;\n";
	cpp << "		} // switch\n";
	cpp <<	"		return INVALID_KEYWORD_INDEX_STRING;\n"
			"	}\n\n";

	// *** write allKeywords() function ***
	cpp <<
		"	/*! Returns all keywords including deprecated for a given category (typenum) and type number (t). */\n"
		"	const char * allKeywords(int typenum, int t) {\n"
		"		switch (typenum) {\n";

	lastCategory.clear();
	switchIndex = 0;
	for (unsigned int i=0; i<m_keywordlist.size(); ++i) {
		// new category = new switch clause
		if (m_keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break;\n";
				++switchIndex;
			}
			lastCategory = m_keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<"\n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) {\n";
		}
		cpp << "				case " << m_keywordlist[i].index << " : return \"" << m_keywordlist[i].keyword << "\";\n";
	}
	// close last switch clause and close switch statement
	cpp << "			} break;\n";
	cpp << "		} // switch\n";
	cpp <<	"		return INVALID_KEYWORD_INDEX_STRING;\n"
			"	}\n\n";

	// *** write description function ***
	cpp <<	"	const char * KeywordList::Description(const char * const enumtype, int t, bool * no_description) {\n"
			"		if (no_description != NULL)\n"
			"			*no_description = false; // we are optimistic\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	for (unsigned int i=0; i<m_keywordlist.size(); ++i) {
		// new category = new switch clause
		if (m_keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break;\n";
				++switchIndex;
			}
			lastCategory = m_keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<"\n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) {\n";
		}

		std::string desc = m_keywordlist[i].description;
		if (desc.empty()) {
			cpp << "				case " << m_keywordlist[i].index << " : if (no_description != NULL) *no_description = true; return \"" << m_keywordlist[i].keyword << "\";\n";
		}
		else {
			cpp << "				case " << m_keywordlist[i].index << " : return \"" << desc << "\";\n";
		}
	}

	cpp <<	"			} break;\n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Cannot determine description for enumeration type '%1' and index '%2'.\")\n"
			"			.arg(enumtype).arg(t), \"[KeywordList::Description]\");\n"
			"	}\n\n";


	// *** write unit() function ***
	cpp <<	"	const char * KeywordList::Unit(const char * const enumtype, int t) {\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	for (unsigned int i=0; i<m_keywordlist.size(); ++i) {
		// new category = new switch clause
		if (m_keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break;\n";
				++switchIndex;
			}
			lastCategory = m_keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<"\n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) {\n";
		}
		cpp << "				case " << m_keywordlist[i].index << " : return \"" << m_keywordlist[i].unit << "\";\n";
	}

	cpp <<	"			} break;\n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Cannot determine default unit for enumeration type '%1' and index '%2'.\")\n"
			"			.arg(enumtype).arg(t), \"[KeywordList::Unit]\");\n"
			"	}\n\n";


	// *** write color() function ***
	cpp <<	"	const char * KeywordList::Color(const char * const enumtype, int t) {\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	for (unsigned int i=0; i<m_keywordlist.size(); ++i) {
		// new category = new switch clause
		if (m_keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break;\n";
				++switchIndex;
			}
			lastCategory = m_keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<"\n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) {\n";
		}
		cpp << "				case " << m_keywordlist[i].index << " : return \"" << m_keywordlist[i].color << "\";\n";
	}

	cpp <<	"			} break;\n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Cannot determine color for enumeration type '%1' and index '%2'.\")\n"
			"			.arg(enumtype).arg(t), \"[KeywordList::Color]\");\n"
			"	}\n\n";


	// *** write defaulValue() function ***
	cpp <<	"	double KeywordList::DefaultValue(const char * const enumtype, int t) {\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	for (unsigned int i=0; i<m_keywordlist.size(); ++i) {
		// new category = new switch clause
		if (m_keywordlist[i].category != lastCategory) {
			// if last category wasn't empty, close last switch clause
			if (!lastCategory.empty()) {
				cpp << "			} break;\n";
				++switchIndex;
			}
			lastCategory = m_keywordlist[i].category;
			// open new switch clause
			cpp << "			// "<< lastCategory <<"\n";
			cpp << "			case "<< switchIndex <<" :\n";
			cpp << "			switch (t) {\n";
		}
		if ( m_keywordlist[i].defaultValue == m_keywordlist[i].defaultValue ) {
			cpp << "				case " << m_keywordlist[i].index << " : return " << m_keywordlist[i].defaultValue << ";\n";
		} else {
			cpp << "				case " << m_keywordlist[i].index << " : return std::numeric_limits<double>::quiet_NaN();\n";
		}
	}

	cpp <<	"			} break;\n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Cannot determine default value for enumeration type '%1' and index '%2'.\")\n"
			"			.arg(enumtype).arg(t), \"[KeywordList::DefaultValue]\");\n"
			"	}\n\n";


	// *** write Count() function ***
	cpp <<	"	// number of entries in a keyword list\n";
	cpp <<	"	unsigned int KeywordList::Count(const char * const enumtype) {\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	unsigned int categoryCount = 0;
	for (unsigned int i=0; i<m_keywordlist.size(); ++i) {

		// new category = new entry in switch
		if (m_keywordlist[i].category != lastCategory) {

			// if last category wasn't empty, close last case clause
			if (!lastCategory.empty()) {
				cpp << categoryCount << ";\n";
				++switchIndex;
				categoryCount = 0;
			}

			lastCategory = m_keywordlist[i].category;
			// open new case clause
			cpp << "			// "<< lastCategory <<"\n";
			cpp << "			case "<< switchIndex <<" : return ";
		}
		++categoryCount;
	}

	cpp <<	categoryCount << ";\n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Invalid enumeration type '%1'.\")\n"
			"			.arg(enumtype), \"[KeywordList::Count]\");\n"
			"	}\n\n";

	cpp <<	"	// max index for entries sharing a category in a keyword list\n";
	cpp <<	"	int KeywordList::MaxIndex(const char * const enumtype) {\n"
			"		switch (enum2index(enumtype)) {\n";
	lastCategory.clear();
	switchIndex = 0;
	categoryCount = 0;
	for (unsigned int i=0; i<m_keywordlist.size(); ++i) {

		// new category = new entry in switch
		if (m_keywordlist[i].category != lastCategory) {

			// if last category wasn't empty, close last case clause
			if (!lastCategory.empty()) {
//				// get max count from enum info
//				const ClassInfo::EnumInfo * einfo = enumInfo(lastCategory);
//				if (einfo==nullptr)
//					throw IBK::Exception(IBK::FormatString("Enumeration type '%1' not recorded.").arg(lastCategory), FUNC_ID);
				cpp << m_keywordlist[i-1].index+1 << ";\n";
				++switchIndex;
				categoryCount = 0;
			}

			lastCategory = m_keywordlist[i].category;
			// open new case clause
			cpp << "			// "<< lastCategory <<"\n";
			cpp << "			case "<< switchIndex <<" : return ";
		}
		++categoryCount;
	}

//	const ClassInfo::EnumInfo * einfo = enumInfo(lastCategory);
//	if (einfo==nullptr)
//		throw IBK::Exception(IBK::FormatString("Enumeration type '%1' not recorded.").arg(lastCategory), FUNC_ID);
	cpp << m_keywordlist[m_keywordlist.size()-1].index+1 << ";\n"
			"		} // switch\n"
			"		throw IBK::Exception(IBK::FormatString(\"Invalid enumeration type '%1'.\")\n"
			"			.arg(enumtype), \"[KeywordList::MaxIndex]\");\n"
			"	}\n\n";

	// write file footer
	cpp << CPP_FOOTER;
	cpp << "} // namespace " << m_namespace << std::endl;
}


void CodeGenerator::generateKeywordlistCodeQt(const IBK::Path & keywordListCpp) {
	FUNCID(CodeGenerator::generateKeywordlistCodeQt);
	// first we generate the header file
	std::ofstream cpp(keywordListCpp.c_str());
	if (!cpp)
		throw IBK::Exception(IBK::FormatString("Cannot create '%1'.").arg(keywordListCpp), FUNC_ID);

	std::cout << "Writing '"<< keywordListCpp.str() << "'" << std::endl;
	// write file header
	cpp << QT_CPP_HEADER;

	// for all description place tr macros here
	for (unsigned int i=0; i<m_keywordlist.size(); ++i) {
		std::string desc = m_keywordlist[i].description;
		if (desc.empty()) {
			cpp << "	tr(\"" << m_keywordlist[i].keyword << "\");\n";
		}
		else {
			cpp << "	tr(\"" << desc << "\");\n";
		}
	}

	// write footer
	cpp << QT_CPP_FOOTER;
}


const ClassInfo::EnumInfo * CodeGenerator::enumInfo(const std::string & categoryName) const {
	for (const ClassInfo & ci : m_classInfo) {
		for (const ClassInfo::EnumInfo & ei : ci.m_enumInfo) {
			if (ei.categoryName == categoryName)
				return &ei;
		}
	}
	return nullptr;
}


void CodeGenerator::insertEnumInfo(const ClassInfo::EnumInfo & einfo) {
	// extract class name
	std::string::size_type pos = einfo.categoryName.find("::");
	IBK_ASSERT(pos != std::string::npos);
	std::string className = einfo.categoryName.substr(0, pos);

//	for (ClassInfo & ci : m_classInfo) {
//		if (ci.)
//	}

}


std::ostream& operator<<(std::ostream& os, const ClassInfo::Keyword& kw) {
	return os << "   \"" << kw.category << "  " << std::setw(2) << kw.index  << "  " << kw.keyword << "\\n\"" << std::endl;
}


bool write_keyword_data(const std::string& output_file, const std::vector<ClassInfo::Keyword>& keywordlist) {
	std::string outfile = output_file;
	replace(outfile.begin(), outfile.end(), '\\', '/');
	std::ofstream out( outfile.c_str());
	// open our template file
	std::ifstream tmp( (outfile+".in").c_str() );
	if (!tmp) {
		std::cerr << "Cannot open template file '"<< (outfile+".in") << "'!." << std::endl;
		return false;
	}
	std::string line;
	bool done = false;
	out << "// WARNING: This is an automatically generated file based on the template file\n"
		<< "//          '" << (outfile+".in") << "'\n"
		<< "//          All changes made to this file will be lost during the next build!\n\n";
	while (getline(tmp, line)) {
		if (!done && line.find("### KEYWORDDATA ###")!=std::string::npos) {
			out << "const char * const KEYWORD_DATA =\n";
			copy(keywordlist.begin(), keywordlist.end(), std::ostream_iterator<ClassInfo::Keyword>(out));
			out << "    ;\n";
			done = true;
		}
		else {
			out << line << '\n';
		}
	}
	return out.good();
}
