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

#include "IBK_EOLStreamBuffer.h"

#include <iostream>

namespace IBK {

EOLStreamBuffer::EOLStreamBuffer( std::ios& in )
	: base_type(),
	  m_in(&in),
	  m_buf( in.rdbuf() ),
	  m_lastc1(),
	  m_lastc2()
{
	in.rdbuf(this);
}

EOLStreamBuffer::~EOLStreamBuffer() {
	m_in->rdbuf(m_buf);
}

EOLStreamBuffer::int_type EOLStreamBuffer::underflow() {
	for ( int_type c = m_buf->sgetc();;c = m_buf->sgetc()) {
		if ( traits_type::eq_int_type( c, traits_type::eof()))
			return traits_type::eof();
		if ( c == 0x1e) {
			if(m_lastc1 == c) {
				m_lastc1 = 0;
				continue;
			}
			else {
				m_lastc1 = c;
				return '\n';
			}
		}
		// \r\n -> skip character
		if (c == '\n' && m_lastc1 == '\r' ) {
			m_lastc1 = 0;
			continue;
		}
		// \n\r -> skip character
		if (c == '\r' && m_lastc1 == '\n' ) {
			m_lastc1 = 0;
			continue;
		}
		// \n or \r -> always return \n
		if (c == '\r' || c == '\n' ) {
			m_lastc1 = c;
			return'\n';
		}
		else {
			m_lastc1 = 0;
			return c;
		}
	}
}

EOLStreamBuffer::int_type EOLStreamBuffer::uflow() {
	for ( int_type c = m_buf->sbumpc();;c = m_buf->sbumpc()) {
		if ( traits_type::eq_int_type( c, traits_type::eof()))
			return traits_type::eof();
		if ( c == 0x1e) {
			if(m_lastc1 == c) {
				m_lastc1 = 0;
				continue;
			}
			else {
				m_lastc1 = c;
				return '\n';
			}
		}
		if (c == '\n' && m_lastc1 == '\r' ) {
			m_lastc1 = 0;
			continue;
		}
		if (c == '\r' && m_lastc1 == '\n' ) {
			m_lastc1 = 0;
			continue;
		}
		if (c == '\r' || c == '\n' ) {
			m_lastc1 = c;
			return'\n';
		}
		else {
			m_lastc1 = 0;
			return c;
		}
	}
}

} // namespace IBK
