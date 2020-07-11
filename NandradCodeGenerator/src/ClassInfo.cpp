#include "ClassInfo.h"

#include <iostream>
#include <fstream>
#include <stdexcept>


void trim(std::string& str) {
	size_t f = str.find_first_not_of(" \t\r\n");
	size_t l = str.find_last_not_of(" \t\r\n");
	if (f!=std::string::npos && l!=std::string::npos)
		str = str.substr(f,l-f+1);
}

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


/*! Attempts to extract a numerical value from a string.
	\code
	double val = string2val<double>("2.5");
	\endcode
*/
double string2val(const std::string& str) {
	double val;
	if (str=="1.#QNAN")
		return std::numeric_limits<double>::quiet_NaN();
	std::stringstream strm(str);
	if (!(strm >> val))
		throw std::runtime_error( "Could not convert"+str+"into value." );
	return val;
}


ClassInfo::ClassInfo() {

}


void ClassInfo::parse(const IBK::Path & filepath) {

	// store time stamp of input file

	// open file
	std::ifstream in(filepath.c_str());
	if (!in) {
		std::cerr << "Error opening file '" << filepath.str() << "'!"<< std::endl;
		throw std::runtime_error("read error");
	}

	std::string line, classname, enumname;
	std::size_t pos;

	// read line by line until end of file is found
	while (getline(in, line)) {
		// get class declaration and remember in which class we are
		pos = line.find("class");
		if (pos != std::string::npos) {
			classname = line.substr(pos+6); // might be "ThisAndThat: public Parent {"
			pos = classname.find_first_of(" \t{:/\n");
			classname = classname.substr(0, pos);
		}

		// get enum name
		pos = line.find("enum");
		if (pos != std::string::npos) {
			enumname = line.substr(pos+4);
			pos = enumname.find_first_of(" \t{/\n");
			enumname = enumname.substr(0, pos);
		}

		// now look for comments of certain type
		pos = line.find("// Keyword:");
		if (pos != std::string::npos) {
			// this line holds an enumeration value
//			std::string kwenum = line.substr(0, pos);
//			std::string kwline;
//			pos = enumname.find_first_of(" \t{/\n");
//			enumname = enumname.substr(0, pos);
		}
	}

}




/*!
	return type kw.index
	0...32000 setup index
	-1 error
	-2 enumeration without set index
*/
ClassInfo::Keyword ClassInfo::parseKeywordLine(const std::string& line, std::map< std::string, int > mapReplacementsToInt ) {

	Keyword kw;
	kw.index = -2;
	unsigned int base;
	int resultI = 0;
	unsigned int size;


	// search for a = to fix enumeration
	size_t posEqual = line.find("=");
	size_t pos = line.find("Keyword:");

	// we found an enumeration now we have to extract it
	if (posEqual<pos){

		// get substring containing only the number
		std::string number;
		size_t posNumberEnd = line.find(",");
		if ( posNumberEnd != std::string::npos ) {
			number = line.substr( posEqual+1, posNumberEnd-posEqual-1 );
		} else {
			size_t posNumberEnd = line.find("/");
			number = line.substr( posEqual+1, posNumberEnd-posEqual-1 );
		}

		// strip all spaces
		number = stripSpaces(number);

		// check if number needs to be replaced by integer representing value
		for (
			 std::map< std::string, int >::const_iterator it = mapReplacementsToInt.begin(),
			 end = mapReplacementsToInt.end();
			 it != end;
			 ++it
			)
		{

			// check if we can find th complete replacement code
			size_t posReplacements = number.find( it->first );
			if ( posReplacements != std::string::npos ){
				// found it so we assign the kw index and go
				kw.index = it->second;
				goto noParsingNeeded;
			}

		}


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
						throw std::runtime_error("Invalid decimal format in line '"+line+"'.");
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

		kw.index = resultI;

	} // if (posEqual!=std::string::npos){


	// this is ugly I know but we can shortcut a lot of code here
	noParsingNeeded:

	if (pos==std::string::npos){
		kw.index = -1;
		return kw; // index==-1  -> no Keyword extracted
	}

	std::string remaining_line = line.substr(pos+9, std::string::npos);
	trim(remaining_line);

	if (remaining_line.empty()){
		kw.index = -1;
		return kw; // index==-1  -> no Keyword extracted
	}

	// we parse the string back to front, starting with the description text
	size_t substart = remaining_line.find('\'');
	size_t subend = remaining_line.rfind('\'');
	if (substart != std::string::npos && subend != std::string::npos) {
		if (subend == substart)
			throw std::runtime_error("Found only on ' in keyword line.");
		kw.description = remaining_line.substr(substart+1, subend - substart - 1);
		trim(kw.description);
		remaining_line = remaining_line.substr(0, substart);
		trim(remaining_line);
	}


	// do we have a default value?
	kw.defaultValue = std::numeric_limits<double>::quiet_NaN();
	subend = remaining_line.rfind('}');
	substart = remaining_line.rfind('{', subend);
	if (substart != std::string::npos) {
		if (subend == std::string::npos)
			throw std::runtime_error("Missing closing } in keyword line.");
		std::string defaultValue = remaining_line.substr(substart+1, subend - substart - 1);
		trim(defaultValue);
		// convert
		try {
			kw.defaultValue=string2val(defaultValue);
		} catch(...){
				throw std::runtime_error("Double conversion failed.");
		}
		remaining_line = remaining_line.substr(0, substart);
		trim(remaining_line);
	}
	else {
		if (subend != std::string::npos)
			throw std::runtime_error("Found closing } without opening { in keyword line.");
	}


	// do we have a color value?
	kw.color = "#FFFFFF";
	subend = remaining_line.rfind('>');
	substart = remaining_line.rfind('<', subend);
	if (substart != std::string::npos) {
		if (subend == std::string::npos)
			throw std::runtime_error("Missing closing > in keyword line.");
		std::string colorValue = remaining_line.substr(substart+1, subend - substart - 1);
		trim(colorValue);
		kw.color = colorValue;
		remaining_line = remaining_line.substr(0, substart);
		trim(remaining_line);
	}
	else {
		if (subend != std::string::npos)
			throw std::runtime_error("Found closing > without opening < in keyword line.");
	}


	// do we have a default unit?
	subend = remaining_line.rfind(']');
	substart = remaining_line.rfind('[', subend);
	if (substart != std::string::npos) {
		if (subend == std::string::npos)
			throw std::runtime_error("Missing closing ] in keyword line.");
		kw.unit = remaining_line.substr(substart+1, subend - substart - 1);
		trim(kw.unit);
		remaining_line = remaining_line.substr(0, substart);
		trim(remaining_line);
	}
	else {
		if (subend != std::string::npos)
			throw std::runtime_error("Found closing ] without opening [ in keyword line.");
	}


	if (remaining_line.empty())
		throw std::runtime_error("Missing keyword before unit or description.");
	kw.keyword = remaining_line; // may contain multiple keywords

	return kw;
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
					trim(classname);
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
					trim(partLine);

					// split line at space
					size_t posSpace = partLine.find_first_of(" \t");

					// devide into parts
					std::string constant = partLine.substr(0, posSpace );
					std::string path = partLine.substr(posSpace, std::string::npos);

					// trim again
					trim(constant);
					trim(path);

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
									trim(includeClassName);
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
					trim(category);
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
