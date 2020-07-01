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

#ifndef IBK_ColorH
#define IBK_ColorH

#include <string>

#include "IBK_configuration.h"

namespace IBK {

/*! Represents an RGB color with alpha blending information. The class supports various
	color formats: Borland, Qt and HTML color. For decoding the conversion functions
	fromTColor(), fromQRgb() and fromHtml() should be used. Encode the color string
	by the backward conversion functions toQRgb(), toTColor(), toHtmlString().
	\todo Re-write class to use single uint32 as storage member.
*/
class Color {
public:
	/*! Default constructor, creates a fully transparent black. */
	Color() : m_red(0), m_green(0), m_blue(0), m_alpha(0) {}
	/*! Constructor, from components */
	Color( unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha=255 ) :
		m_red(red), m_green(green), m_blue(blue), m_alpha(alpha)
	{}

	/*! Constructor, from encoded rgba unsigned int value. */
	Color( unsigned int rgba ) : m_red((rgba & 0xFF000000) >> 24),m_green((rgba & 0x00FF0000) >> 16),m_blue((rgba & 0x0000FF00)>>8),m_alpha((rgba & 0xFF)) {}
//	Color( unsigned int argb ):m_red((argb & 0xFF0000) >> 16),m_green((argb & 0xFF00) >> 8),m_blue(argb & 0xFF),m_alpha((argb & 0xFF000000) >> 24) {}

public:
	/*! Reads the color data from a string.
		The function throws an IBK::Exception if an error occurs.
		\param data Section of an encoded string including the
		keyword 'COLOR' or 'COLOUR'.
	*/
	void read(const std::string& data);

	/*! Writes the keyword COLOUR followed by the color as HTML-string into the stream 'out'.
		\param out Stream buffer for the COLOUR data section.
		\param indent number of line indent spaces for the 'COLOUR data section.
	*/
	void write(std::ostream& out, unsigned int indent=0) const;

	/*! Converts from Borland format into IBK::Color.
		\param tcolor Borland TColor-value.
	*/
	static Color fromTColor(int tcolor);

	/*! Converts from Qt format into IBK::Color.
		\param qrgb Qt QRgb-value.
	*/
	static Color fromQRgb(unsigned int qrgb);

	/*! Converts a HTML-color code into IBK::Color if code string starts with # character.
		Otherwise, attempts to read an unsigned int value from string. If that uint is larger
		than a signed integer, it tries to convert from QRgb, otherwise tries to convert from TColor.
		Throws an IBK::Exception if string reading/conversion fails.

		\note When reading HTML-color code strings, two formats are supported: #aarrggbb and #rrggbb.
			  In case of the shorter variant, the color is set to be fully opaque (alpha value set to 255).
		\param html HTML-color code string.
	*/
	static Color fromHtml(const std::string &html);

	/*! Converts IBK::Color into an unsigned int equivalent to QRgb.
		\return Qt QRgb-value.
	*/
	unsigned int toQRgb() const;

	/*! Converts IBK::Color into an int equivalent to Borland's TColor.
		\return Borland TColor-value.
	*/
	int toTColor() const;

	/*! Converts IBK::Color into HTML color.
		\return HTML-color code string, beginning with a # character.
	*/
	std::string toHtmlString() const;

	/*! Comparison operator, colors are equal if all components are equal. */
	friend bool operator==(const Color& lhs, const Color& rhs) {
		return lhs.m_red == rhs.m_red && lhs.m_green == rhs.m_green && lhs.m_blue == rhs.m_blue && lhs.m_alpha == rhs.m_alpha;
	}
	/*! Comparison operator 'not equal'. */
	friend bool operator!=(const Color& lhs, const Color& rhs) {
		return !(lhs == rhs);
	}

	/*! Data element name, used mark a color as invalid, or to label a color.
		\todo This should not be part of the base IBK::Color class, but belongs to a derived
		class "ExtendedColor" or "NamedColor"...
	*/
	std::string		m_name;

	unsigned int	m_red;		///< Red channel, 0..255
	unsigned int	m_green;	///< Green channel, 0..255
	unsigned int	m_blue;		///< Blue channel, 0..255
	unsigned int	m_alpha;	///< Alpha channel, 0..255,0 - fully transparent, 255 - fully opaque

};

}  // namespace IBK

/*! \file IBK_Color.h
	\brief Contains declaration for class Color.
*/

#endif // IBK_ColorH
