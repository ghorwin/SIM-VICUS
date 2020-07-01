/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#include "IBK_configuration.h"
#include "IBK_messages.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <iterator>
#include <algorithm>

#include "IBK_InputOutput.h"
#include "IBK_StringUtils.h"
#include "IBK_FormatString.h"

using namespace std;

namespace IBK {

bool read_vector(const std::string& data, std::vector<double>& vec, std::string * errmsg) {
	std::stringstream strm(data);
	double para;
	vec.clear();
	while (strm >> para)
		vec.push_back(para);
	// check if stream end was really reached (otherwise an error had occured while reading)
	if(!strm.eof() ) {
		*errmsg = FormatString("Couldn't read a vector from string '%1': string contains invalid data.").arg(data.c_str()).str();	
		return false;
	}
	if (vec.size() < 2)  {
		if (errmsg != NULL) {
			*errmsg = FormatString("Couldn't read a vector from string '%1'.").arg(data.c_str()).str();
		}
		return false;
	}
	return true;
}
// ----------------------------------------------------------------------------

void write_vector(std::ostream & out, unsigned int n, const double * v, unsigned int width) {
	for (unsigned int i=0; i<n; ++i)
		out << std::setw(width) << std::right << v[i];
	out << std::endl;
}
// ----------------------------------------------------------------------------

std::ostream& write_vector(std::ostream& out, const std::string& timestr, const std::vector<double>& vec) {
	out << std::left << timestr << '\t';
	std::copy(vec.begin(), vec.end(), std::ostream_iterator<double>(out, "\t") );
	return out;
}
// ----------------------------------------------------------------------------

void write_vector(std::ostream & out, const char * name, unsigned int n, const double * v, unsigned int width) {
	out << std::left << name << '\t';
	for (unsigned int i=0; i<n; ++i)
		out << std::setw(width) << std::right << v[i];
	out << std::endl;
}
// ----------------------------------------------------------------------------

void dump_vector(std::ostream & out, const char * name, unsigned int n, const double * v, unsigned int width) {
	out << name << ":" << endl;
	for (unsigned int i=0; i<n; ++i)
		out << "[ " << std::setw(width) << std::right << v[i] << " ]" << std::endl;
}
// ----------------------------------------------------------------------------


void read_string_binary(std::istream& in, std::string& str, uint32_t max_len) {
	uint32_t len;
	in.read(reinterpret_cast<char *>(&len), sizeof(uint32_t));
	if (len > max_len)
		throw IBK::Exception("String length exceeds given maximum!", "[read_string_binary]");
	// single character (len = 0) means empty string, there is no \0 character
	if (len > 0) {
		str.resize(len);
		in.read(&str[0], len);
	}
	else{
		str.clear();
	}
}
// ----------------------------------------------------------------------------


void read_string_binary_with_termination_character(std::istream& in, std::string& str, uint32_t max_len) {
	uint32_t len;
	in.read(reinterpret_cast<char *>(&len), sizeof(uint32_t));
	if (len > max_len)
		throw IBK::Exception("String length exceeds given maximum!", "[read_string_binary_with_termination_character]");
	if (len == 0)
		throw IBK::Exception("Invalid string length, must be > 0!", "[read_string_binary_with_termination_character]");
	--len; // subtract one, because we do not read the additional \0 into the string
	str.resize(len);
	in.read(&str[0], len);
	char dummy;
	// read termination character and check if it is correct
	in.read(&dummy, 1);
	if (dummy != '\0')
		throw IBK::Exception("Invalid string format, missing termination character!", "[read_string_binary_with_termination_character]");
}
// ----------------------------------------------------------------------------


void write_string_binary(std::ostream& out, const std::string& str) {
	uint32_t len = static_cast<uint32_t>(str.size());
	out.write(reinterpret_cast<char *>(&len), sizeof(uint32_t));
	if (len > 0)
		out.write(str.c_str(), len); // do not write termination character
}
// ----------------------------------------------------------------------------


void write_string_binary_with_termination_character(std::ostream& out, const std::string& str) {
	uint32_t len = static_cast<uint32_t>(str.size()+1); // add 1 to include termination character
	out.write(reinterpret_cast<char *>(&len), sizeof(uint32_t));
	out.write(str.c_str(), len);
}
// ----------------------------------------------------------------------------


void read_string_list_binary( std::istream &in,
							  const std::vector<std::string> & lastLineTokens,
							  std::list<std::string> & strList,
							  IBK::NotificationHandler * notify)
{
	unsigned int currentPosition = static_cast<unsigned int>(in.tellg());
	in.seekg( 0, std::ios_base::end );
	unsigned int endPosition = static_cast<unsigned int>(in.tellg());
	in.seekg( currentPosition );
	unsigned int size_left = endPosition - currentPosition;

	const unsigned int CHUNK_SIZE = 1 << 18;
	char block[CHUNK_SIZE+1];									// contains the block of read data
	unsigned int total_chunks = endPosition / CHUNK_SIZE+1;
	unsigned int chunk_count = 0;
	bool isBrokenString = false;
	std::string line;

	while (size_left != 0) {

		// determine how much to read an do read block
		unsigned int block_size = std::min<unsigned int>(size_left, CHUNK_SIZE);
		in.read(block, block_size);
		++chunk_count;
		if (!in) {
			throw IBK::Exception( FormatString("Couldn't read string something is wrong."), "[read_string_list_binary]");
		}

		// now parse block and split it at line breaks
		unsigned int i_linestart = 0;

		for (unsigned int i=0; i<=block_size; ++i) {

			// if we found a line ending or the eof
			// we have to extract a string and append it to the list
			if ((block[i] == '\n') || (size_left == 0) )
			{
				// remove new line break
				block[i] = 0;

				// also remove preceeding '\r'
				if (i>0 && block[i-1] == '\r')
					block[i-1] = 0;


				// do we have unfinished string to fix?
				if ( isBrokenString ){

					// append string at the end
					line += std::string(&block[i_linestart]);
					isBrokenString = false;

				} else {

					// replace last string with
					// composed line from block remainder and new line piece
					line = std::string(&block[i_linestart]);

				}

				// move line start forward
				i_linestart = i+1;


				// if line doesn't contain end marker append to list
				// otherwise break execution now
				// append string to the string list
				unsigned int comparisonLimiter = lastLineTokens.size();
				for ( unsigned int r = 0; r < comparisonLimiter; ++r ){

					std::string::size_type pos = line.find( lastLineTokens[r] );
					if ( pos == std::string::npos ) {

						// nothing found go on parsing...
						continue;

					}
					else if ( pos == 0 ){

						// correct break found from function
						// notify the world what we have done everything
						// append last line and finish
						strList.push_back( line );

						// seek input file stream back to end of last line
						in.seekg( (chunk_count-1)*CHUNK_SIZE + i_linestart);

						return;

					}
					else {

						/// \todo error handling
						// some comment or error in file found...
						//IBK::IBK_Message( FormatString("Parse error in appended line: '%1' \n If it's just a comment please ignore this warning.").arg(line), MSG_WARNING, "IBK::InputOutput::read_string_list_binary");

					}

				}

				// append line to output list
				strList.push_back( line );

			// END OF if ((block[i] == '\n') || (size_left == 0) )
			// we now check if the complete block was finished/reworked
			}
			else if ( i == block_size ){


				//we found the end of a block
				isBrokenString = true;

				// store first string part
				line = std::string(&block[i_linestart],&block[block_size]);

				// notify the world what we have done, size_left -= block_size;
				if (notify){
					notify->notify( double (chunk_count) / total_chunks);
				}


			}

/*			else {
				// do some error handling here
				IBK::IBK_Message( FormatString("Seriouse error while parsing line: '%'.").arg(line), MSG_ERROR, "IBK::InputOutput::read_string_list_binary");
			}
*/
		} // for (unsigned int i=0; i<=block_size; ++i) {

		size_left -= block_size;

	} // while size left

}
// ----------------------------------------------------------------------------


void read_string_vector_binary( std::istream &in,
							  const std::vector<std::string> & lastLineTokens,
							  std::vector<std::string> & strList,
							  IBK::NotificationHandler * notify)
{
	unsigned int currentPosition = static_cast<unsigned int>(in.tellg());
	in.seekg( 0, std::ios_base::end );
	unsigned int endPosition = static_cast<unsigned int>(in.tellg());
	in.seekg( currentPosition );
	unsigned int size_left = endPosition - currentPosition;

	const unsigned int CHUNK_SIZE = 1 << 18;
	char block[CHUNK_SIZE+1];									// contains the block of read data
	unsigned int total_chunks = endPosition / CHUNK_SIZE+1;
	unsigned int chunk_count = 0;
	bool isBrokenString = false;
	std::string line;

	while (size_left != 0) {

		// determine how much to read an do read block
		unsigned int block_size = std::min<unsigned int>(size_left, CHUNK_SIZE);
		in.read(block, block_size);
		++chunk_count;
		if (!in) {
			throw IBK::Exception( FormatString("Couldn't read string something is wrong."), "[read_string_list_binary]");
		}

		// now parse block and split it at line breaks
		unsigned int i_linestart = 0;

		for (unsigned int i=0; i<=block_size; ++i) {

			// if we found a line ending or the eof
			// we have to extract a string and append it to the list
			if ((block[i] == '\n') || (size_left == 0) )
			{
				// remove new line break
				block[i] = 0;

				// also remove preceeding '\r'
				if (i>0 && block[i-1] == '\r')
					block[i-1] = 0;


				// do we have unfinished string to fix?
				if ( isBrokenString ){

					// append string at the end
					line += std::string(&block[i_linestart]);
					isBrokenString = false;

				} else {

					// replace last string with
					// composed line from block remainder and new line piece
					line = std::string(&block[i_linestart]);

				}

				// move line start forward
				i_linestart = i+1;


				// if line doesn't contain end marker append to list
				// otherwise break execution now
				// append string to the string list
				unsigned int comparisonLimiter = lastLineTokens.size();
				for ( unsigned int r = 0; r < comparisonLimiter; ++r ){

					std::string::size_type pos = line.find( lastLineTokens[r] );
					if ( pos == std::string::npos ) {

						// nothing found go on parsing...
						continue;

					}
					else if ( pos == 0 ){

						// correct break found from function
						// notify the world what we have done everything
						// append last line and finish
						strList.push_back( line );

						// seek input file stream back to end of last line
						in.seekg( (chunk_count-1)*CHUNK_SIZE + i_linestart);

						return;

					}
					else {

						/// \todo error handling
						// some comment or error in file found...
						//IBK::IBK_Message( FormatString("Parse error in appended line: '%1' \n If it's just a comment please ignore this warning.").arg(line), MSG_WARNING, "IBK::InputOutput::read_string_list_binary");

					}

				}

				// append line to output list
				strList.push_back( line );

			// END OF if ((block[i] == '\n') || (size_left == 0) )
			// we now check if the complete block was finished/reworked
			}
			else if ( i == block_size ){


				//we found the end of a block
				isBrokenString = true;

				// store first string part
				line = std::string(&block[i_linestart],&block[block_size]);

				// notify the world what we have done, size_left -= block_size;
				if (notify){
					notify->notify( double (chunk_count) / total_chunks);
				}


			}

/*			else {
				// do some error handling here
				IBK::IBK_Message( FormatString("Seriouse error while parsing line: '%'.").arg(line), MSG_ERROR, "IBK::InputOutput::read_string_list_binary");
			}
*/
		} // for (unsigned int i=0; i<=block_size; ++i) {

		size_left -= block_size;

	} // while size left

}
// ----------------------------------------------------------------------------


} // namespace IBK
