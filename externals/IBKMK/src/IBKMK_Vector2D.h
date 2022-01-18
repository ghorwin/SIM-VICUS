/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

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

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.

*/

#ifndef IBKMK_Vector2DH
#define IBKMK_Vector2DH

#include <IBK_point.h>
#include <IBK_Exception.h>

namespace IBKMK {

/*! A vector class in 2D, extends the IBK::point2D<> template class with computation
	functionality.
*/
class Vector2D : public IBK::point2D<double> {
public:

	/*! Default constructor, initializes default point/vector. */
	Vector2D() {}

	/*! Initializing constructor for base class. */
	Vector2D(const IBK::point2D<double> &p) : IBK::point2D<double>(p.m_x, p.m_y)
	{
	}

	/*! Convenience constructor. */
	Vector2D(double a, double b) : IBK::point2D<double>(a,b)
	{
	}

	/*! returns distance to other vector */
	double distanceTo(const Vector2D &v){
		return Vector2D(*this - v).magnitude();
	}

	/*! Returns the magnitude of the vector. */
	double magnitude() const {
		return std::sqrt(m_x*m_x + m_y*m_y);
	}

	/*! Returns the magnitude squared of the vector. */
	double magnitudeSquared() const {
		return m_x*m_x + m_y*m_y;
	}

	/*! Returns the scalar product of this and another vector. */
	double scalarProduct(const Vector2D & other) const {
		return m_x*other.m_x + m_y*other.m_y;
	}

	/*! Normalizes the vector: <math>v = \frac{v}{\left| v \right|}</math>. */
	void normalize() {
		double mag = magnitude();
		if (mag != 0.0)
			*this /= magnitude();
	}

	/*! Normalizes the vector: <math>v = \frac{v}{\left| v \right|}</math>. */
	Vector2D normalized() const {
		Vector2D t(*this);
		t.normalize();
		return t;
	}

	/*! Assignment operator. */
	const Vector2D & operator=(const IBK::point3D<double> & pt) {
		m_x = pt.m_x;
		m_y = pt.m_y;
		return *this;
	}

	/*! Scales all components of the vector with the scalar \a scalar. */
	const Vector2D & operator*=(double scalar) {
		m_x *= scalar;
		m_y *= scalar;
		return *this;
	}

	/*! Devides all components of the vector by the scalar \a scalar. */
	const Vector2D & operator/=(double scalar) {
		if (scalar == 0.0)
			throw IBK::Exception("Division by zero", "[Vector2D::operator/=]");
		return operator*=(1/scalar);
	}

	/*! Adds another vector to this point/vector. */
	const Vector2D & operator+=(const Vector2D & other){
		m_x += other.m_x;
		m_y += other.m_y;
		return *this;
	}

	/*! Subtracts another vector from this point/vector. */
	const Vector2D & operator-=(const Vector2D & other){
		m_x -= other.m_x;
		m_y -= other.m_y;
		return *this;
	}

	/*! Adds another vector to this vector and returns the result. */
	Vector2D operator+(const Vector2D & other) const {
		Vector2D tmp = *this;
		tmp += other;
		return tmp;
	}

	/*! Subtracts another vector from this vector and returns the result. */
	Vector2D operator-(const Vector2D & other) const {
		Vector2D tmp = *this;
		tmp -= other;
		return tmp;
	}

};

/*! Scales all components of the vector with the scalar \a scalar. */
inline IBKMK::Vector2D operator*(double scalar, const IBKMK::Vector2D &other) {
	IBKMK::Vector2D tmp = other;
	tmp *= scalar;
	return tmp;
}

/*! Scales all components of the vector with the scalar \a scalar. */
inline IBKMK::Vector2D operator*(const IBKMK::Vector2D &other, double scalar) {
	IBKMK::Vector2D tmp = other;
	tmp *= scalar;
	return tmp;
}

} // namespace IBKMK


#endif // IBKMK_Vector2DH
