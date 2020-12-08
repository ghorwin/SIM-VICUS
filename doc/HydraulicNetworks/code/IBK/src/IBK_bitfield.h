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

#ifndef IBK_bitfieldH
#define IBK_bitfieldH

#include <string>

namespace IBK {

/*! Convenience function to check if a \a bitfield contains the bit with the index \a bit_idx.
	\code
	// example, check if m_equations contains BE_MOISTURE
	bool moisture_enabled = has_flag(m_equations, BE_MOISTURE);
	\endcode
	\param bitfield An or'd combination of bits.
	\param bit_idx The index of the bit to test.
*/
inline bool has_flag(unsigned int bitfield, unsigned int bit_idx) { return (bitfield & (0x1 << bit_idx)) != 0; }

/*! Convenience function to set a flag with given index \a bit_idx in a \a bitfield.
	\code
	// example, enabled moisture mass balance
	set_flag(m_equations, BE_MOISTURE);
	\endcode
	\param bitfield An or'd combination of bits.
	\param bit_idx The index of the bit to test.
	*/
inline void set_flag(unsigned int & bitfield, unsigned int bit_idx) { bitfield |= 0x1 << bit_idx; }

/*! Convenience function to set a flag with given index \a bit_idx in a \a bitfield.
	\code
	// example, enabled moisture mass balance
	clear_flag(m_equations, BE_MOISTURE);
	\endcode
	\param bitfield An or'd combination of bits.
	\param bit_idx The index of the bit to test.
	*/
inline void clear_flag(unsigned int & bitfield, unsigned int bit_idx) { bitfield &= ~ (0x1 << bit_idx); }

} // namespace IBK

/*! \file IBK_bitfield.h
	\brief Contains utility functions for index-based bitfield operations (low-level replacement
		   for std::bitset)
*/

#endif // IBK_bitfieldH
