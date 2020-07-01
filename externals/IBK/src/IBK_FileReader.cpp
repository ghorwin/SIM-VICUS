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

#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <algorithm>

#include "IBK_Exception.h"
#include "IBK_FileReader.h"
#include "IBK_assert.h"
#include "IBK_Path.h"
#include "IBK_messages.h"
#include "IBK_FormatString.h"

namespace IBK {

const char CHR = '\r';		///< Character constant for carriage-return
const char CHN = '\n';		///< Character constant for line-feed


FileReaderDataProcessorLinesVector::FileReaderDataProcessorLinesVector(std::vector<std::string>& lines) :
	m_lines(lines)
{}

void FileReaderDataProcessorLinesVector::processLine(const std::string& line) {
	try {
		m_lines.push_back(line);
	}
	catch (std::exception& e) {
		throw IBK::Exception( IBK::FormatString("Error in FileReaderDataProcessorLinesVector::processLine.\n %1").arg(e.what()), "[FileReaderDataProcessorLinesVector::processLine]" );
	}
}

int FileReaderDataProcessorLinesVector::lineCount() const {
	return (int)m_lines.size();
}

/*! Helper function to check if a two-character end-of-line marker is found at current stream position. */
bool isTwoCharEOL(char first, char second) {
	if( first == CHR && second == CHN)
		return true;
	if( first == CHN && second == CHR)
		return true;
	return false;
}

/*! Reader function encapsulates the binary reading from a input stream.
	The input stream must be opened in binary mode and valid.
	\param inputStream Binary opened input stream.
	\param process Data processor for a line.
	\param chunksize Size of internal buffer.
	\param lastLineTokens If one of the strings in this list exists in a line this is the last line.
	\param maxlines Maximum number of lines to be read. If 0 no limit exists.
	\param notify Optional pointer to notification handler.
	\param fsize Optional file size (used for progress notification calls).
*/
long long readFunct(	std::istream& inputStream,
						AbstractFileReaderDataProcessor* process,
						unsigned int chunksize,
						const std::vector<std::string>& lastLineTokens,
						unsigned int maxlines = 0,
						IBK::NotificationHandler * notify = 0,
						long long fsize = 0)
{
	const char * const FUNC_ID = "[FileReader::readFunct]";

	IBK_ASSERT(chunksize>0);

	if( !process) {
		throw IBK::Exception("Data processor not valid", FUNC_ID);
	}

	bool notFinished = false;

	char lastChar = '\0';
	long long currentPos = inputStream.tellg();
	if (currentPos > 0) {
		inputStream.seekg(currentPos - 1);
		inputStream.read(&lastChar, 1);
	}

	// cannot forward to current position or stream in bad state?
	if (inputStream.fail() || currentPos == -1) {
		throw IBK::Exception("Invalid input stream state", FUNC_ID);
	}

	unsigned lastLineTokenSize = (unsigned int)lastLineTokens.size();

	std::vector<char> bufferVect(chunksize + 1, 0);
	char* buffer = &bufferVect[0];
	// set line limit for read operation
	unsigned int lineBound =  maxlines > 0  &&  process->lineCount() != -1 ? maxlines + process->lineCount() : 0;
	std::string line;
	line.reserve(20000000); // 20 Mb memory reserved for each line

	try {
		// loop until end of file or last line token has been found
		std::streamsize readSize;
		for (;;) {
			// try to read 'chunksize' bytes
			if (inputStream.read(buffer, chunksize))
				readSize = chunksize;
			else // if failed, get number of read bytes
				readSize = inputStream.gcount();

			// currentPos holds stream position before last call to read()
			// buffer now holds 'readSize' bytes of data

			// chp points to the current position in the buffer
			char *chp = buffer;
			// chpbegin points to first line in buffer
			char *chpbegin = buffer;
			// check if first char is the end of a two char EOL
			// this might happen, if the last chunk just ended on the first char of a two char EOL
			if (isTwoCharEOL(*chp, lastChar)) {
				++chp;
				++chpbegin;
			}

			// process remaining bytes
			for( ; (chp - buffer) < readSize; ++chp) {

				// end of line found?
				if( *chp == CHR || *chp == CHN) {
					// finish the existing line
					if( notFinished) {
						notFinished = false;
						line.append(chpbegin, chp);
					}
					// add a whole new line
					else {
						line.assign(chpbegin, chp);
					}

					// let line processor do something with the line
					process->processLine(line);

					// check for premature abort because of given last line token
					for (unsigned int i=0; i<lastLineTokenSize; ++i) {
						// Note: "SIDES" is also found in the line "TABLE  SIDES_GEOMETRY"
						// Fix: End line tokens must start at the begin of a line
						// Performance bug: The search for the line end token ruins the good performance
						// of the file reader, especially when lines are long and many end line tokens
						// are specified - better use a function "starts with"
						std::string::size_type tokenPos = line.find(lastLineTokens[i]);
						if (tokenPos != std::string::npos && tokenPos == line.find_first_not_of(" \t")) {
							long long pos = currentPos;
							// add offset in current buffer, chp is positioned after the EOL of the line containing the lastLineToken
							pos += (chp - buffer);
							return pos;
						}
					}
					// clear line
					line.clear();

					if (notify) {
						if (fsize > 0)
							notify->notify((double)(currentPos + readSize)/fsize);
						else
							notify->notify();
					}

					// end of buffer reached?
					if( *(chp + 1) == 0)
						break; // breaks inner for loop
					// if no last line token found read next line
					if( *(chp + 1) == CHR || *(chp + 1) == CHN) {
						++chp;
					}

					// stop if line limit has been exceeded
					if( (lineBound != 0 && static_cast<int>(lineBound) <=  process->lineCount())) {
						// current position before read + offset in buffer + 1
						return currentPos + (chp - buffer) + 1;
					}

					chpbegin = chp + 1;
				}
				// no end of line found
				else {
					// check if we
					if((chp - buffer) == (readSize - 1)) {
						// add whole content to existing line
						if( notFinished) {
							line.append(chpbegin, chp + 1);
						}
						// create a new line with the whole content
						else {
							line.assign(chpbegin, chp + 1);
							notFinished = true;
						}
					}
				}
			}
			currentPos += readSize; // process next chunk

			// look for end of stream or maximum line number
			if( inputStream.eof())
				break;

		} // for (;;)

		return currentPos; // return file size = number of read bytes
	}
	catch (IBK::Exception& ex) {
		throw IBK::Exception(ex, "Error reading file.", FUNC_ID );
	}
	catch(std::exception& e) {
		throw IBK::Exception( e.what(), FUNC_ID );
	}
}

unsigned int FileReader::sizeForBOM(BOMType type) {
	switch(type) {
		case UTF16_BE:
		case UTF16_LE: return 2;
		case UTF8:
		case UTF1:
		case SCSU:
		case BOCU1_3: return 3;
		case UTF32_BE:
		case UTF32_LE:
		case UTF7:
		case UTF_EBCDIC:
		case BOCU1_4:
		case GB18030: return 4;
		case NoBOM: return 0;
		default: return 0;
	}
}

FileReader::BOMType FileReader::getBOM(const std::vector<unsigned char>& first4Bytes) {
	if( first4Bytes.size() < 4)
		return NoBOM;

	if( first4Bytes[0] == 0xEF && first4Bytes[1] == 0xBB && first4Bytes[2] == 0xBF)
		return UTF8;

	if( first4Bytes[0] == 0xFE && first4Bytes[1] == 0xFF)
		return UTF16_BE;

	if( first4Bytes[0] == 0xFE && first4Bytes[1] == 0xFE)
		return UTF16_LE;

	if( first4Bytes[0] == 0x00 && first4Bytes[1] == 0x00 && first4Bytes[2] == 0xFE && first4Bytes[3] == 0xFF)
		return UTF32_BE;

	if( first4Bytes[0] == 0xFF && first4Bytes[1] == 0xFE && first4Bytes[2] == 0x00 && first4Bytes[3] == 0x00)
		return UTF32_LE;

	if( first4Bytes[0] == 0x2B && first4Bytes[1] == 0x2F && first4Bytes[2] == 0x76) {
		if( first4Bytes[3] == 0x38 || first4Bytes[3] == 0x39 || first4Bytes[3] == 0x2B || first4Bytes[3] == 0x2F)
			return UTF7;
	}

	if( first4Bytes[0] == 0xF7 && first4Bytes[1] == 0x64 && first4Bytes[2] == 0x4C)
		return UTF1;

	if( first4Bytes[0] == 0xDD && first4Bytes[1] == 0x73 && first4Bytes[2] == 0x66 && first4Bytes[3] == 0x73)
		return UTF_EBCDIC;

	if( first4Bytes[0] == 0x0E && first4Bytes[1] == 0xFE && first4Bytes[2] == 0xFF)
		return SCSU;

	if( first4Bytes[0] == 0xFB && first4Bytes[1] == 0xEE && first4Bytes[2] == 0x28) {
		if( first4Bytes[3] == 0xFF)
			return BOCU1_4;
		return BOCU1_3;
	}

	if( first4Bytes[0] == 0x84 && first4Bytes[1] == 0x31 && first4Bytes[2] == 0x95 && first4Bytes[3] == 0x33)
		return GB18030;

	return NoBOM;
}


long long FileReader::readAll(	const Path &filename,
								AbstractFileReaderDataProcessor* process,
								const std::vector<std::string> & lastLineTokens,
								unsigned int skipBytes,
								IBK::NotificationHandler * notify)
{
	const char * const FUNC_ID = "[FileReader::readAll]";

	// set internal buffer
	const int CHUNKSIZE = 4096;
	int chunksize = CHUNKSIZE;
	char internalBuffer[CHUNKSIZE] = {0};
	std::ifstream inputStream;
	inputStream.rdbuf()->pubsetbuf(internalBuffer, CHUNKSIZE);

	// get filesize
	long long fsize = filename.fileSize();

#if defined(_WIN32)
	#if defined(_MSC_VER)
			inputStream.open(filename.wstr().c_str(), std::ios_base::binary);
	#else
			std::string filenameAnsi = IBK::WstringToANSI(filename.wstr(), false);
			inputStream.open(filenameAnsi.c_str(), std::ios_base::binary);
	#endif
#else // _WIN32
			inputStream.open(filename.c_str(), std::ios_base::binary);
#endif
	if (!inputStream.is_open()) {
		throw IBK::Exception( IBK::FormatString("Cannot open input file '%1' for reading").arg(filename), FUNC_ID);
	}

	// look for a BOM and skip this bytes if a BOM exists
	std::vector<unsigned char> first4(4, 0);
	inputStream.read(reinterpret_cast<char*>(&first4[0]), 4);
	// set stream back to begin
	inputStream.clear();
	inputStream.seekg(0, std::ios_base::beg);
	FileReader::BOMType bomtype = FileReader::getBOM(first4);
	if( bomtype != FileReader::NoBOM)
		skipBytes = std::max(skipBytes, FileReader::sizeForBOM(bomtype));

	// skip some bytes at begin
	if( skipBytes > 0) {
		inputStream.seekg(skipBytes);
	}

	return readFunct(inputStream, process, chunksize, lastLineTokens, 0, notify, fsize);
}


long long FileReader::readAll(	const Path& filename,
								std::vector<std::string>& lines,
								const std::vector<std::string> & lastLineTokens,
								unsigned int skipBytes,
								IBK::NotificationHandler * notify)
{
	FileReaderDataProcessorLinesVector linesProcessor(lines);
	return readAll(filename, &linesProcessor, lastLineTokens, skipBytes, notify);
}


FileReader::FileReader(const Path &filename, unsigned int chunksize) :
	m_filename(filename),
	CHUNKSIZE(chunksize),
	internalBuffer(chunksize, 0),
	m_streamPos(0)
{
}


bool FileReader::valid() {

	if (!m_filename.isFile())
		return false;

	open();
	return m_inputStream.is_open();
}


std::vector<unsigned char> FileReader::firstBytes(unsigned int size) {
	const char * const FUNC_ID = "[FileReader::firstBytes]";
	if (!valid()) {
		throw IBK::Exception("Invalid file path or error opening file for reading.", FUNC_ID);
	}
	std::vector<unsigned char> result(size);
	if(!m_inputStream.read(reinterpret_cast<char*>(&result[0]), size)) {
		m_inputStream.clear();
		m_inputStream.seekg(0, std::ios_base::beg);
		throw IBK::Exception("Error reading requested number of bytes from file.", FUNC_ID);
	}

	// set stream back to begin
	m_inputStream.clear();
	m_inputStream.seekg(0, std::ios_base::beg);
	return result;
}


FileReader::ReadState FileReader::readFirst( unsigned int maxlines, unsigned int skipBytes, IBK::NotificationHandler * notify ) {
	const char * const FUNC_ID = "[FileReader::readFirst]";

	if (!valid()) {
		throw IBK::Exception(FormatString("Cannot open input file: %1").arg(m_filename), FUNC_ID );
	}

	// look for a BOM and skip this bytes if a BOM exists
	try {
		std::vector<unsigned char> first4 = firstBytes(4);
		BOMType bomtype = FileReader::getBOM(first4);
		if( bomtype != NoBOM)
			skipBytes = std::max(skipBytes, sizeForBOM(bomtype));
	}
	catch (IBK::Exception& ex) {
		throw IBK::Exception(ex, FormatString("Error reading file header from file: %1.").arg(m_filename), FUNC_ID );
	}

	// skip some bytes at begin
	if( skipBytes > 0) {
		std::vector<char> bufferVect(skipBytes, 0);
		char* buffer = &bufferVect[0];
		m_inputStream.read(buffer, skipBytes);

		// an error occured
		if(m_inputStream.bad() ) {
			m_streamPos = -1;
			throw IBK::Exception(FormatString("Error reading %1 first bytes of file: %2")
								 .arg(skipBytes)
								 .arg(m_filename), FUNC_ID );
		}

		// remaining file is empty
		if(m_inputStream.eof() ) {
			IBK_Message( FormatString("No further content after reading %1 first bytes of file: %2")
						 .arg(skipBytes)
						 .arg(m_filename), MSG_WARNING, FUNC_ID);
			m_streamPos = -1;
			return FileReader::FR_Eof;
		}
	}

	try {
		m_lines.clear();
		m_lines.reserve(10000);
		FileReaderDataProcessorLinesVector linesProcessor(m_lines);
		long long pos = readFunct(m_inputStream, &linesProcessor, CHUNKSIZE, std::vector<std::string>(), maxlines, notify);
		m_streamPos = pos;
		if (m_inputStream.eof())
			return FileReader::FR_Eof;

		if (pos == -1) {
			throw IBK::Exception("Wrong position in stream after reading lines from file.", FUNC_ID );
		}

		return FileReader::FR_Good;
	}
	catch (IBK::Exception& ex) {
		throw IBK::Exception(ex, FormatString("Error reading file: %1").arg(m_filename), FUNC_ID );
	}
}


FileReader::ReadState FileReader::readNext(unsigned int maxlines, IBK::NotificationHandler * notify) {
	const char * const FUNC_ID = "[FileReader::readNext]";

	if (!m_inputStream.is_open()) {
		throw IBK::Exception(FormatString("Cannot open input file: %1").arg(m_filename), FUNC_ID );
	}

	try {
		m_inputStream.clear();
		m_inputStream.seekg(m_streamPos);
		FileReaderDataProcessorLinesVector linesProcessor(m_lines);
		long long pos = readFunct(m_inputStream, &linesProcessor, CHUNKSIZE, std::vector<std::string>(), maxlines, notify);
		m_streamPos = pos;
		if (m_inputStream.eof())
			return FileReader::FR_Eof;

		if( pos == -1) {
			throw IBK::Exception("Wrong position in stream after reading lines from file.", FUNC_ID );
		}

		return FileReader::FR_Good;
	}
	catch(IBK::Exception& ex) {
		throw IBK::Exception(ex, FormatString("Error reading file: %1").arg(m_filename), FUNC_ID );
	}
}


void FileReader::open() {
	if( !m_inputStream.is_open()) {
		m_inputStream.rdbuf()->pubsetbuf(&internalBuffer[0], CHUNKSIZE);
#if defined(_WIN32)
	#if defined(_MSC_VER)
			m_inputStream.open(m_filename.wstr().c_str(), std::ios_base::binary);
	#else
			std::string filenameAnsi = IBK::WstringToANSI(m_filename.wstr(), false);
			m_inputStream.open(filenameAnsi.c_str(), std::ios_base::binary);
	#endif
#else // _WIN32
			m_inputStream.open(m_filename.c_str(), std::ios_base::binary);
#endif
	}
	else {
		m_inputStream.clear();
		m_inputStream.seekg(0, std::ios_base::beg);
	}
}

void FileReader::clear() {
	m_lines.clear();
	m_streamPos = 0;
	m_inputStream.close();
}

void FileReader::clearBuffer() {
	m_lines.clear();
}

const IBK::Path& FileReader::filename() const {
	return m_filename;
}

void FileReader::setFilename(const Path &filename) {
	m_filename = filename;
	clear();
}

} // namespace IBK
