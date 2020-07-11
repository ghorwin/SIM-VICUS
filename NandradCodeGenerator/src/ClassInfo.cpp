#include "ClassInfo.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <IBK_StringUtils.h>

void trimLastBracket(std::string& str) {
	size_t l = str.find_last_not_of("{");
	if (l!=std::string::npos)
		str = str.substr(0,l+1);
}


std::string stripSpaces(const std::string & input) {
	std::string tmp;

	for (unsigned int inputStringIndex = 0, stringSize = input.size();
		inputStringIndex < stringSize; inputStringIndex++ )
	{

		if (input[inputStringIndex] != ' ' && input[inputStringIndex] != '\t'){
			tmp.push_back(input[inputStringIndex]);
		}

	}
	return tmp;
}


bool ClassInfo::parse(const IBK::Path & headerFilePath) {

	// store time stamp of input file

	// open file
	std::ifstream in(headerFilePath.c_str());
	if (!in) {
		std::cerr << "Error opening file '" << headerFilePath.str() << "'!"<< std::endl;
		throw std::runtime_error("read error");
	}

	m_sourceHeaderFile = headerFilePath;

	std::string line, classname, enumname;
	std::size_t pos;
	bool inEnum = false;
	unsigned int enumIdx = 0;

	// read line by line until end of file is found
	while (getline(in, line)) {
		try {

			// get class declaration and remember in which class we are
			pos = line.find("class");
			if (pos != std::string::npos && pos+6 < line.size()) {
				// check that there is nothing but whitespace before the class
				if (line.find_first_not_of(" \t") == pos) {
					classname = line.substr(pos+6); // might be "ThisAndThat: public Parent {"
					pos = classname.find_first_of(" \t{:/\n");
					// guard against pure forward-declaration, in which case the first char after the class will be a ;
					classname = classname.substr(0, pos);
					IBK::trim(classname);
					if (classname.back() == ';')
						continue; // skip this line
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
	//			std::cout << line << std::endl;
				std::cout << "enum " << classname << "::" << enumname << std::endl;
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
				kw.category = classname + "::" + enumname;
				m_keywords.push_back(kw);
				++enumIdx;
				continue;
			}

			// if we are still in an enumeration section, check for NUM_xxx tokens - usually without Keyword
			pos = line.find("NUM_");
			if (inEnum && pos != std::string::npos) {
				// check that NUM_ does not have any leading [ or any other char
				if (line.find_first_not_of(" \t") != pos)
					continue; // not a line starting with "NUM_"
				std::string::size_type pos2 = line.find_first_of(" \t\r\n}", pos);
				std::string enumName = line.substr(pos, pos2-pos);

				inEnum = false;
				EnumInfo einfo;
				einfo.categoryName = classname + "::" + enumname;
				einfo.count = enumIdx;
				einfo.enumNUM = enumName;
				m_enumInfo.push_back(einfo);
				// remember NUM_ dummy keyword to be used to determine corrent enum for read/write blocks
				std::cout << enumName << ":" << enumIdx << std::endl;
				continue;
			}

			// now check for xml read/write annotations


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




#if 0

// ********* Header File Parser **************
bool parse_headers(const std::string& src_dir, const std::vector<std::string>& hfiles, std::vector<Keyword>& keywordlist) {

	//unsigned int lastnum;
	for (unsigned int i=0; i<hfiles.size(); ++i) {

		std::string fname = src_dir + '/' + hfiles[i];

		//lastnum = static_cast<unsigned int>(keywordlist.size());
		std::ifstream in(fname.c_str());
		if (!in) {
			std::cerr << "Error opening file '" << fname << "'!"<< std::endl;
			std::cerr << "For hfile '" << hfiles[i] << "'!"<< std::endl;
			return false;
		}

		//        cout << left << setw(30) << hfiles[i] << endl;    // output current file
		std::string line, classname;
		std::size_t pos;

		// Loop until end of file is reached
		while (in){

			// seek forward until the starting sequence is found
			while (getline(in, line) && line.find("***KEYWORDLIST-START***")==std::string::npos) {

				pos = line.find("class");
				if (pos != std::string::npos && (pos+6<line.size()) ) {
					classname = line.substr(pos+6);
					IBK::trim(classname);
					trimLastBracket(classname);
					if (classname.empty()) {
						in >> classname;
						continue;
					}
					pos = classname.find_first_of(" \t\n");
					if (pos==std::string::npos) continue;
					classname.erase(pos);
				}

			}

			// trim : from classname
			size_t f = classname.find_first_not_of(":");
			size_t l = classname.find_last_not_of(":");
			if (f!=std::string::npos && l!=std::string::npos)
				classname = classname.substr(f,l-f+1);

			// no starting sequence found, skip the rest
			if (!in) {
				//                if (keywordlist.size() - lastnum==0)
				//                    cout << " - nix" << endl;
				continue;
			}

			int keycount=0;
			std::string category;

			// necessary for resolving CONSTANT_RESOLVE lines
			bool	replacementCompileTimeConstantFound = false;
			std::map< std::string, std::string > constantsMappings;
			std::map< std::string, int > mapCompileTimeConstantToValue;

			// loop until ending sequence
			while (getline(in, line) && line.find("***KEYWORDLIST-END***")==std::string::npos) {

				if (line.empty())
					continue;

				// found new enumeration?
				pos=line.find("***CONSTANT_RESOLVE ");
				if (pos!=std::string::npos) {

					replacementCompileTimeConstantFound = true;

					// cut first part
					std::string partLine = line.substr(pos+20, std::string::npos);

					// trim both ends of spaces
					IBK::trim(partLine);

					// split line at space
					size_t posSpace = partLine.find_first_of(" \t");

					// devide into parts
					std::string constant = partLine.substr(0, posSpace );
					std::string path = partLine.substr(posSpace, std::string::npos);

					// trim again
					IBK::trim(constant);
					IBK::trim(path);

					// assign
					constantsMappings[constant] = path;
					continue;

				}

				// now that we have all the repleacement string we need to resolve them
				if (replacementCompileTimeConstantFound){

					for (
							std::map< std::string, std::string >::const_iterator it = constantsMappings.begin(),
							end = constantsMappings.end();
							it != end;
							++it
						)
					{

						// open file for replacements

						std::string relativePath =  src_dir + "/" + it->second;
						std::cout << "Sourcing constant Keywords from file: " << relativePath << std::endl;
						std::ifstream inIncludeFile( relativePath.c_str() );
						if (!inIncludeFile) {
							std::cerr << "Error opening file '" << it->second << "',"<< std::endl;
							std::cerr << "referenced in ***CONSTANT_RESOLVE line from file '"<< fname << "'." << std::endl;
							throw std::runtime_error( "Error processing CONSTANT_RESOLVE directive." );
						}

						// now we have to search for a class pattern, and a constant value in an enum
						// possible values NAMESPACE::CLASS::ENUM_VALUE; CLASS::ENUM_VALUE

						size_t lastQuad = it->first.find_last_of(':');
						size_t firstQuad = it->first.find_first_of(':');

						if ( (lastQuad == std::string::npos) || (firstQuad == std::string::npos) ){
//							std::cerr << "Invald target to replace, syntax: NAMESPACE::CLASS::ENUM_VALUE or CLASS::ENUM_VALUE expected." << std::endl;
							throw std::runtime_error( "Invalid target to replace, syntax: NAMESPACE::CLASS::ENUM_VALUE or CLASS::ENUM_VALUE expected." );
						}

						std::string searchEnumValue = it->first.substr(lastQuad+1, std::string::npos);
						std::string searchClass;
						if ( firstQuad == lastQuad-1 ){
							// CLASS::ENUM_VALUE
							searchClass = it->first.substr( 0, firstQuad );
						} else {
							// NAMESPACE::CLASS::ENUM_VALUE
							searchClass = it->first.substr( 0, lastQuad-1 );
							// NAMESPACE::CLASS
							pos = searchClass.find_last_of(':');
							if (pos != std::string::npos){
								searchClass = searchClass.substr( pos+1, std::string::npos );
							} else {
//								std::cerr << "Invald target to replace, syntax: NAMESPACE::CLASS::ENUM_VALUE expected." << std::endl;
								throw std::runtime_error( "Invald target to replace, syntax: NAMESPACE::CLASS::ENUM_VALUE expected." );
							}
						}
						std::string includeLine, includeClassName;

//						// search replacement
//						while(inIncludeFile){

							// seek forward until a class is found
							while ( getline(inIncludeFile, includeLine) ) {

								pos = includeLine.find("class");
								if (pos != std::string::npos && (pos+6<includeLine.size()) ) {
									includeClassName = includeLine.substr(pos+6);
									IBK::trim(includeClassName);
									trimLastBracket(includeClassName);
									if (includeClassName.empty()) {
										inIncludeFile >> includeClassName;
										continue;
									}
									pos = includeClassName.find_first_of(" \t\n");
									if (pos==std::string::npos) continue;
									includeClassName.erase(pos);
								}

								// found it so we go over to search the enum value
								if ( searchClass == includeClassName ){

									// now seek forward to find enum, just one overloaded enum value is allowed at a time
									int counterValue = 0;
									while ( getline(inIncludeFile, includeLine) )
									{
										// check if we left the class, if we are done we can leave
										pos=includeLine.find("***KEYWORDLIST-END***");
										if (pos != std::string::npos)
											break;

										pos=includeLine.find("enum ");
										if (pos != std::string::npos){

											counterValue = 0;
											while ( getline(inIncludeFile, includeLine) ) {


												// check if we finaly have found the constant we looking for
												size_t posEnumValue = includeLine.find( searchEnumValue );
												if ( posEnumValue != std::string::npos ){

													// store index in map
													mapCompileTimeConstantToValue[ it->first ] = counterValue;
													break;
												}


												// calc start value
												// if a line contains a =
												// search for a = to fix enumeration
												size_t posEqual = includeLine.find("=");
												size_t pos = includeLine.find("Keyword:");

												// we found an enumeration now we have to extract it
												if (posEqual<pos){

													unsigned int base;
													int resultI = 0;
													unsigned int size;

													// get substring containing only the number
													std::string number;
													size_t posNumberEnd = includeLine.find(",");
													if ( posNumberEnd != std::string::npos ) {
														number = includeLine.substr( posEqual+1, posNumberEnd-posEqual-1 );
													} else {
														size_t posNumberEnd = includeLine.find("/");
														number = includeLine.substr( posEqual+1, posNumberEnd-posEqual-1 );
													}

													// strip all spaces
													number = stripSpaces(number);

													// hex or no hex
													posEqual = number.find("0x");
													if ( posEqual == std::string::npos ){

														// unsigned int or int found
														posEqual = number.find("-");
														base = 1;
														if ( posEqual == std::string::npos ){

															// unsigned int found
															// copy number in reverse
															// convert hex string to unsigned int
															size = number.size();
															while (size--){

																// is character a number
																if ( number[size] < 58 && number[size] > 47 ){
																	resultI += (number[size]-48) * base;
																	base *= 10;
																} else {
																	throw std::runtime_error("Invalid decimal format in line '"+includeLine+"'.");
																}

															} // while (size--){


														} else {

															// int found
															throw std::runtime_error("Invalid int format not supported yet.");

														}

													} else {

														// copy number in reverse
														// hexadecimal number found
														// convert hex string to unsigned int

														// strip hex preface
														number = number.substr( posEqual+2 );
														// strip spaces
														size = number.size();
														base = 1;

														while (size--){

															// is character a number
															if ( number[size] < 58 && number[size] > 47 ){

																resultI += (number[size]-48) * base;
																base *= 16;

															} else {

																// upper case ascii hex
																if ( number[size] < 71 && number[size] > 64 ){

																	resultI += (number[size]-55) * base;
																	base *= 16;

																} else {

																	// lower case ascii hex
																	if ( number[size] < 103 && number[size] > 96 ){

																		resultI += (number[size]-87) * base;
																		base *= 16;

																	} else {

																		// error during parsing
																		/// \todo print some error here
																		throw std::runtime_error("Invalid hexadecimal format.");

																	} // else if (number[size]){

																} // else if (number[size]){

															}  // else if ( number[size] < 58 && number[size] > 47 ){

														} // while (size--) {

													} // else if ( posEqual == std::string::npos ){

													counterValue = resultI;

												} else { // if (posEqual<pos){

													// just increment line count
													counterValue++;

												} // if (posEqual<pos){


											} // while ( getline(inIncludeFile, includeLine) )

										} // if (pos != std::string::npos){


									} // while ( getline(inIncludeFile, includeLine) )

									break;

								} // if ( searchClass == includeClassName ){

							} // // seek forward until a class is found



//						} // while(inIncludeFile){

					} // for all mappings

					// reset all resolve mappings
					constantsMappings.clear();
					replacementCompileTimeConstantFound = false;

				} // if (replacementCompileTimeConstantFound){


				/**********************
					*
					* start new enum field
					*
				***********************/



				// found new enumeration?
				pos=line.find("enum ");
				if (pos!=std::string::npos) {
					category=line.substr(pos+5, std::string::npos);
					pos = category.find("{");
					if (pos==std::string::npos) {
						std::cerr << " - error in enumeration '" << line << "'. '{' is missing!" << std::endl;
						return false;
					}
					category.erase( pos );
					IBK::trim(category);
					keycount=0;

					continue;
				}

				// we require the string 'Keyword:', this string cannot be used in the description or
				// any other place in the code
				pos=line.find("Keyword:");
				if (pos != std::string::npos){

					Keyword kw;
					try {
						kw = parse_keyword_line( line, mapCompileTimeConstantToValue );
					}
					catch (std::exception & ex) {
						std::cerr << ex.what() << std::endl;
						throw std::runtime_error("Error in file '"+fname+"'");
					}
					if ( kw.index==-1 )
						continue;  // no keyword found

					// set index according to user wishes and store it to the keycounter
					// increment the
					if ( kw.index==-2 )
						kw.index = keycount++;
					else
						keycount = kw.index + 1;

					if ( classname.empty() )
						kw.category = category;
					else
						kw.category = (classname + "::" + category);
					keywordlist.push_back(kw);

					for ( std::vector< Keyword >::iterator it = keywordlist.begin(); it != keywordlist.end(); it++) {
						if ( (*it).category == kw.category ) {
							(*it).maxIndexInCategory = kw.index;
						};
					}

				} // if we found a new keyword

			} // while (getline(in, line) && line.find("***KEYWORDLIST-END***")==std::string::npos) {


		}  // over all loop
	}
	return true;
}


#endif
