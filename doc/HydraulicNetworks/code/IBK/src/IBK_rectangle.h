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

#ifndef IBK_rectangleH
#define IBK_rectangleH

#include <iostream>
#include <sstream>

#include "IBK_Exception.h"
#include "IBK_FormatString.h"

namespace IBK {

/*! A type that represents a rectangle.
	The class can be used whenever coordinates describing a rectangle have to
	be stored together, such as for screen coordinates or when describing a
	geometrical shape.
*/
template <class T>
class rectangle {
  public:
	/*! Default contructor (initialises the rectangle with default value of type T). */
	rectangle() : left(T()), top(T()), right(T()), bottom(T()) {}
	/*! Constructor (initialises the rectangle with given parameters for the coordinates).
		\param newLeft		Left coordinate.
		\param newTop		Top coordinate.
		\param newRight		Right coordinate.
		\param newBottom	Bottom coordinate.
	*/
	rectangle(T newLeft, T newTop, T newRight, T newBottom) {
		set(newLeft, newTop, newRight, newBottom);
	}

	/*! Virtual destructor, so that destructor of derived classes is called. */
	virtual ~rectangle(){}

	/*! Initializes a rectangle.
		\param newLeft		New left coordinate.
		\param newTop		New top coordinate.
		\param newRight		New right coordinate.
		\param newBottom	New bottom coordinate.
	*/
	virtual void set(T newLeft, T newTop, T newRight, T newBottom) {
		left=newLeft;
		top=newTop;
		right=newRight;
		bottom=newBottom;
	}

	/*! With of the rectangle.
		\note Might be negative.
		\return Difference between the coordinates 'right' and 'left'.
	*/
	T width()   const   { return right - left; }

	/*! Height of the rectangle.
		\note Might be negative.
		\return Difference between the coordinates 'bottom' and 'top'.
	*/
	T height()  const   { return bottom - top; }

	/*! Prints the rectangle range in format "<left> <top> <right> <bottom>"
		\param out	 Stream buffer for data output.
	*/
	virtual std::ostream& output(std::ostream& out) const {
		return out << left << " " << top << " " << right << " " << bottom;
	}

	/*! Formatted output of the rectangle.
		\return String with format: "[<left>, <top>] - [<right>, <bottom>]".
	*/
	std::string formattedString() const {
		std::stringstream strm;
		strm << "[" << left << "," << top << "] - [" << right << "," << bottom << "]";
		return strm.str();
	}

	/*! String conversion in format "<left> <top> <right> <bottom>".
		\return The range as string with all coordinates separated by a single space.
	*/
	std::string toString() const {
		std::stringstream strm;
		strm << *this;
		return strm.str();
	}

	/*! Checks if a point is inside the rectangle (inside but not at the boundary).
		\param	x	point x-coordinate.
		\param	y	point y-coordinate.
		\return	'true' if coordinate pair x,y is in the rectangle, 'false' otherwise.
	*/
	bool contains(T x, T y) const {
		if (left < right) {
			if (left > x || right < x) return false;
		}
		else
			if (right > x || left < x) return false;
		if (top < bottom) {
			if (top > y || bottom < y) return false;
		}
		else
			if (bottom > y || top < y) return false;
		return true;
	}

	T left;     ///< The left coordinate.
	T top;      ///< The top coordinate.
	T right;    ///< The right coordinate.
	T bottom;   ///< The bottom coordinate.
};

/*! Inline function that writes the coordinates to a stream.
	(the coordinates are written in the following order: left, top, right, bottom).
	\param out	Stream buffer for rectangle output.
	\param rect	Rectangle to be printed.
*/
template <class T>
std::ostream& operator << (std::ostream& out, const rectangle<T>& rect)
{
	return rect.output(out);
}

/*! Inline function that reads the coordinates from a stream.
	(the coordinates are expected in the following order: left, top, right, bottom).
	The coordinates of 'rect' are only modified if ALL coordinates could be read
	correctly.
	\param in	Input stream for rectangle coodinates, in format "<left> <top> <right> <bottom>".
	\param rect	Rectangle to be read.
*/
template <class T>
inline std::istream& operator>>(std::istream& in, rectangle<T>& rect) {
	rectangle<T> tmp;
	if (in >> tmp.left >> tmp.top >> tmp.right >> tmp.bottom)
		std::swap(tmp,rect);
	return in;
}

/*! Compares two rectangles.
	\param	lhs First rectangle to compare
	\param	rhs Second rectangle to compare
	\return 'true' if they have the same coordinates, 'false' otherwise.
*/
template <class T>
inline bool operator==(const rectangle<T>& lhs, const rectangle<T>& rhs) {
	return (lhs.left == rhs.left && lhs.top==rhs.top && lhs.right==rhs.right && lhs.bottom==rhs.bottom); }

} // namespace IBK

/*! \file IBK_rectangle.h
	\brief Contains the declaration of class rectangle.
*/

#endif // IBK_rectangleH
