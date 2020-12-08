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

#ifndef IBK_CSVReaderH
#define IBK_CSVReaderH

#include <vector>
#include <string>

#include "IBK_Path.h"

namespace IBK {

/*! A class for simplified reading of tab/csv separated double values in a column format.
	\todo Speed up reading through use of IBK::FileReader
*/
class CSVReader {
public:
	CSVReader() : m_separationCharacter('\t'), m_nColumns(0), m_nRows(0) {}

	/*! Reads table from a file.
		\param filename Input file name.
	*/
	void read(const IBK::Path & filename, bool headerOnly = false, bool extractUnits = false);

	/*! Convenience function to extract data of a given column (colIndex starts with index 0). */
	std::vector<double> colData(unsigned int colIndex) const;

	/*! Utility function to test if a file has tab-separated values or is written in Excel flavor, with , and quotation characters. */
	static bool haveTabSeparationChar(const IBK::Path & filename);

	/*! Separation character for different tabulator columns. */
	char								m_separationCharacter;
	/*! Tabulator captions: columns of the first line. */
	std::vector<std::string>			m_captions;
	/*! If read() was call with extractUnits=true, the captions are parsed and if they contain
		[<unit>], the unit strings will be stored in m_units and the captions will be truncated
		accordingly.
	*/
	std::vector<std::string>			m_units;
	/*! Data values sorted by row and column, access via m_values[row][column] or use the convenience function colData(colIndex). */
	std::vector<std::vector<double> >	m_values;
	/*! Number of tabulator columns. */
	unsigned int						m_nColumns;
	/*! Number of tabulator rows. */
	unsigned int						m_nRows;
};

} // namespace IBK

/*! \file IBK_CSVReader.h
	\brief A simple CSV reader class.
*/

#endif // IBK_CSVReaderH
