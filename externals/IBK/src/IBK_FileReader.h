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

#ifndef IBK_FileReaderH
#define IBK_FileReaderH

#include <fstream>
#include <string>
#include <vector>

#include "IBK_NotificationHandler.h"
#include "IBK_Path.h"

namespace IBK {

/*! \brief Abstract base class for data processor used in FileReader.
	It processes one line.
*/
class AbstractFileReaderDataProcessor {
public:
	/*! Standard constructor.*/
	AbstractFileReaderDataProcessor() {}

	/*! Standard destructor.*/
	virtual ~AbstractFileReaderDataProcessor() {}

	/*! Processes one line.
		\param line Line for processing.
		\todo Fix return value and enforce proper exception handling.
	*/
	virtual void processLine(const std::string& line) = 0;

	/*! Returns the count of lines in the internal buffer (if exists).
		This function is used for function in FileReader with maxlines parameter.
		It should return -1 if line count is not possible.
	*/
	virtual int lineCount() const = 0;

private:
	AbstractFileReaderDataProcessor(const AbstractFileReaderDataProcessor&) {}
	AbstractFileReaderDataProcessor& operator=(const AbstractFileReaderDataProcessor&) { return *this; }
};


/*! \brief Data processor used in FileReader for handling lines in vector.
	It adds each line to the line vector (m_lines).

	\code
	std::vector<std::string> lines;

	// create line processor
	FileReaderDataProcessorLinesVector lineProcessor(lines);
	// read some part of the file
	FileReader reader("some/file.txt");
	reader.readFirst(20000);

	// now 'lines' contains the data from the file.
	\endcode
*/
class FileReaderDataProcessorLinesVector : public AbstractFileReaderDataProcessor {
public:
	/*! Standard constructor. Takes a reference of an external line vector.
		\param lines Line vector.
	*/
	FileReaderDataProcessorLinesVector(std::vector<std::string>& lines);

	/*! Adds a line to the line vector. Error is only possible in case of unsufficent memory.
		\param line Line for processing.
	*/
	virtual void processLine(const std::string& line);

	/*! Returns the size of lines vector.
		This function is used for function in FileReader with maxlines parameter.
	*/
	virtual int lineCount() const;
private:
	std::vector<std::string> & m_lines;	///< Reference to a line vector.
};


/*! \brief Class for fast reading and evaluating files.
	The class is mainly created for fast evaluating of text files.
	It uses internally binary reading.

	Typical use case, read complete file:
	\code
	// 1. reading a complete file into a buffer

	// create target storage location
	std::vector<std::string> lines;

	// specify tokens to stop reading
	std::vector<std::string> tokens; // none

	// use static readAll function, skipBytes = 0 (read from start), notify = NULL
	long long lastPos = IBK::FileReader::readAll(IBK::Path("/home/user/mylongfile.txt"), lines, tokens, 0, NULL);
	// lastPos == filesize when reading was successful
	// in case of error, an IBK::Exception is thrown
	\endcode
*/
class FileReader {
public:
	/*! Possible values for BOM type. */
	enum BOMType {
		UTF8,		///< UTF-8 encoding
		UTF16_BE,	///< UTF-16 little endian
		UTF16_LE,	///< UTF-16 big endian
		UTF32_BE,	///< UTF-32 little endian
		UTF32_LE,	///< UTF-32 big endian
		UTF7,		///< UTF-7 (for non 8-bit systems)
		UTF1,		///< UTF-1 (transforming Unicode to byte streams)
		UTF_EBCDIC,	///< UTF-8 version for EBCDIC format from IBM
		SCSU,		///< Standard compression scheme for unicode
		BOCU1_3,	///< Binary ordered compression for unicode (3 byte encoding)
		BOCU1_4,	///< Binary ordered compression for unicode (4 byte encoding)
		GB18030,	///< Chinese coding standard
		NoBOM		///< No BOM found
	};

	/*! Return value of functions readFirst() and readNext(). */
	enum ReadState {
		FR_Good,	///< Stream in good state. Ready for further reading.
		FR_Bad,		///< Error, no further reading possible.
		FR_Eof		///< End of file reached. No further reading possible.
	};

	/*! Size of a given BOM encoding.
		\param	type BOM encoding type (\sa BOMType).
		\return number of bytes for the given BOM (byte order mark).
	*/
	static unsigned int sizeForBOM(BOMType type);

	/*! Decodes a BOM type.
		\param first4Bytes BOM encoding at file stream begin ()
		\return the type of BOM (byte order mark) (\sa BOMType) if some exist otherwise NoBOM.
	*/
	static BOMType getBOM(const std::vector<unsigned char>& first4Bytes);

	/*! Standard constructor, only takes the filename and chunksize but does not open the file yet.
		\param filename Name of the file.
		\param chunksize Size of internal buffer for binary reading.
	*/
	FileReader(const Path& filename, unsigned int chunksize = 4096);

	/*! Tries to open the given file and checks if that was successful.
		If file was opened successfully, the file stream remains open.
		\return 'true' if successful, 'false' otherwise.
	*/
	bool valid();

	/*! Reads a given number of bytes from the beginning of the file.
		Throws an exception of file cannot be opened or if file is smaller than the requested number of bytes.
		\param size Number of bytes for reading.
		\return Vector of read bytes. Can be smaller than given number if file is smaller.

		\note This function calls valid() internally, also the file stream is positioned at the
			  begin of the file once bytes have been read.
	*/
	std::vector<unsigned char> firstBytes(unsigned int size);

	/*! Reads the given file as text line by line.
		Use streamPosition() to request the number of bytes that were successfully read.
		The lines are read using the FileReaderDataProcessorLinesVector() and stored in the file reader object.
		Use lines() to get access to the lines read so far.
		\param maxlines Maximum number of lines for reading (complete file if 0).
		\param skipBytes Number of bytes which will be skipped at beginning of file, for example to skip a file header.
		\param notify Optional pointer to notification handler (use for large files).
		\return Returns a read state (\sa ReadState).
	*/
	ReadState readFirst(	unsigned int maxlines = 0, unsigned int skipBytes = 0,
							NotificationHandler * notify = 0);

	/*! Reads from the given file as text line by line from the last position.
		The new lines will be added to the existing ones. Access to the data with lines().
		\param maxlines Maximum number of lines for reading (complete file if 0).
		\param notify Optional pointer to notification handler (use for large files).
		\return Returns a read state (\sa ReadState).
	*/
	ReadState readNext( unsigned int maxlines, NotificationHandler * notify = 0);

	/*! Returns the lines vector.*/
	const std::vector<std::string>& lines() const { return m_lines; }

	/*! Clears the internal buffer and sets file position to 0.
		Stream will be closed. Filename will keep the same. Line buffer in object is cleared.
	*/
	void clear();

	/*! Clears only the internal buffer (lines vector).
		Can be used to read only a certain number of lines, access the data, and start with
		a fresh buffer/line vector for the next lines.
		Filename and stream position will be kept the same.
	*/
	void clearBuffer();

	/*! Returns the current filename.*/
	const Path& filename() const;

	/*! Set a new filename. All internal buffers will be cleared and stream closed. */
	void setFilename(const Path& filename);

	/*! Returns the current stream position.*/
	std::streampos streamPosition() const { return m_streamPos; }

	/*! Reads the given file as text line by line.
		Throws an IBK::Exception in case of error.
		\param filename Name of the file.
		\param process Pointer to data processor.
		\param lastLineTokens If one of the strings in this list exists in a line this is the last line.
		\param skipBytes Number of bytes which will be skipped at beginning of file.
		\param notify Optional pointer to notification handler (use for large files).
		\return Returns current stream position or -1 of reading was not successful.
	*/
	static long long readAll(const Path& filename, AbstractFileReaderDataProcessor* process,
							 const std::vector<std::string> & lastLineTokens,
							 unsigned int skipBytes = 0, NotificationHandler * notify = 0);

	/*! Reads the given file as text line by line. This function is provided as convenience and uses
		a FileReaderDataProcessorLinesVector with the provided vector of strings as storage member.
		Throws an IBK::Exception in case of error.
		\param filename Name of the file.
		\param lines Vector of lines as storage location (strings are appended to existing vector).
		\param lastLineTokens If one of the strings in this list exists in a line this is the last line.
		\param skipBytes Number of bytes which will be skipped at beginning of file.
		\param notify Optional pointer to notification handler (use for large files).
		\return Returns current stream position or -1 of reading was not successful.
	*/
	static long long readAll(const Path& filename, std::vector<std::string>& lines,
							 const std::vector<std::string> & lastLineTokens,
							 unsigned int skipBytes = 0, NotificationHandler * notify = 0);

private:
	Path						m_filename;		///< Name of the file
	std::vector<std::string>	m_lines;		///< Buffer for read lines.
	const unsigned int			CHUNKSIZE;		///< Size for internal buffer

	std::vector<char>			internalBuffer;	///< Internal read buffer
	std::ifstream				m_inputStream;	///< Binary stream for reading
	std::streampos				m_streamPos;	///< Last position of stream.

	/*! Opens the file if its closed or set file pointer to begin if its already opened.*/
	void open();
};

} // namespace IBK

/*! \file IBK_FileReader.h
	\brief Contains declaration of class FileReader.
*/

#endif // IBK_FileReaderH
