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

#ifndef IBK_EOLStreamBufferH
#define IBK_EOLStreamBufferH

#include <streambuf>

namespace IBK {

/*! \brief Stream buffer class for filtering different EOL signs.
	This buffer can be used as buffer for input streams in order to convert different EndOfLine sequences
	into one usable ('\n').
	This is useful if one want read text files created at WindowsOS on a MacOS system.
	\code
	// create filestream and open file
	std::ifstream fin(fileName.c_str());
	// set eolfilter as new streambuffer
	IBK::EOLStreamBuffer eolfilter(in);
	// use filestream like before
	std::string text1, text2;
	in << text1 << text2;
	\endcode
	The stream buffer read directly from input device (no additional buffer).
*/
class EOLStreamBuffer : public std::streambuf {
public:
	/*! Standard constructor.
		\param in Input stream to be used
		It stores the old stream buffer and install itself as new streambuffer for in.
	*/
	explicit EOLStreamBuffer( std::ios& in );

	/*! Destructor.
		Sets the old streambuffer, stored in constructor, back to the stream.
	*/
	~EOLStreamBuffer();

protected:
	/*! New underflow function will be called in case of input buffer underflow.
		It makes the necessary conversion. The read pointer will not set back.
	*/
	virtual int_type underflow();

	/*! New uflow function will be called if the stream fetches a new character.
		It makes the necessary conversion.
		It calls underflow in case of a buffer underflow and sets the buffer read pointer to next position.
	*/
	virtual int_type uflow();

private:
	/*! Private copy constructor in order to prevent copy.*/
	EOLStreamBuffer(const EOLStreamBuffer& src);

	/*! Private copy assignment operator in order to prevent copy.*/
	EOLStreamBuffer& operator=(const EOLStreamBuffer&);

	/*! Typedef to identify base class type
		\todo Remove this type and use std::streambuf unless needed.
	*/
	typedef std::streambuf base_type;

	std::ios*		m_in;		///< Reference to input stream.
	std::streambuf* m_buf;		///< Pointer to original buffer from input stream.
	int_type		m_lastc1;	///< Last read char in underflow, initialized to 0 on construction.
	int_type		m_lastc2;	///< Last read char in uflow, initialized to 0 on construction.
};

} // namespace IBK

/*! \file IBK_EOLStreamBuffer.h
	\brief Contains declaration of class EOLStreamBuffer.
*/

#endif // IBK_EOLStreamBufferH
