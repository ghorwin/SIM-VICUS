/*	IBK Math Kernel Library
	Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

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

#ifndef IBKMK_Vector3DH
#define IBKMK_Vector3DH

#include <IBK_point.h>
#include <IBK_Exception.h>

namespace IBKMK {

/*! A vector class in 3D, extends the IBK::point3D<> template class with computation
	functionality.
*/
class Vector3D : public IBK::point3D<double> {
public:

	/*! Default constructor, initializes default point/vector. */
	Vector3D() {}
	/*! Convenience constructor. */
	Vector3D(double a, double b, double c) : IBK::point3D<double>(a,b,c)
	{
	}
	/*! Initializing constructor for base class. */
	Vector3D(const IBK::point3D<double> & pt) : IBK::point3D<double>(pt)
	{
	}

	/*! Returns the magnitude of the vector. */
	double magnitude() const {
		return std::sqrt(m_x*m_x + m_y*m_y + m_z*m_z);
	}

	/*! Returns the cross product of this vector with another.
		Computes result = this x other.
	*/
	Vector3D crossProduct(const Vector3D & other) const {
		Vector3D result;
		crossProduct(other, result);
		return result;
	}

	/*! Returns the cross product of this vector with another.
		Computes result = this x other.
		This function is implemented performance oriented, but not as
		readible as the other crossProduct() version.
	*/
	void crossProduct(const Vector3D & other, Vector3D & result) const {
		result.m_x = m_y*other.m_z - m_z*other.m_y;
		result.m_y = m_z*other.m_x - m_x*other.m_z;
		result.m_z = m_x*other.m_y - m_y*other.m_x;
	}

	/*! Returns the scalar product of this and another vector. */
	double scalarProduct(const Vector3D & other) const {
		return m_x*other.m_x + m_y*other.m_y + m_z*other.m_z;
	}

	/*! Normalizes the vector: <math>v = \frac{v}{\left| v \right|}</math>. */
	void normalize() {
		*this /= magnitude();
	}

	/*! Assignment operator. */
	const Vector3D & operator=(const IBK::point3D<double> & pt) {
		m_x = pt.m_x;
		m_y = pt.m_y;
		m_z = pt.m_z;
		return *this;
	}

	/*! Scales all components of the vector with the scalar \a scalar. */
	const Vector3D & operator*=(double scalar) {
		m_x *= scalar;
		m_y *= scalar;
		m_z *= scalar;
		return *this;
	}

	/*! Devides all components of the vector by the scalar \a scalar. */
	const Vector3D & operator/=(double scalar) {
		if (scalar == 0)
			throw IBK::Exception("Division by zero", "[Vector3D::operator/=]");
		return operator*=(1/scalar);
	}

	/*! Adds another vector to this point/vector. */
	const Vector3D & operator+=(const Vector3D & other){
		m_x += other.m_x;
		m_y += other.m_y;
		m_z += other.m_z;
		return *this;
	}

	/*! Subtracts another vector from this point/vector. */
	const Vector3D & operator-=(const Vector3D & other){
		m_x -= other.m_x;
		m_y -= other.m_y;
		m_z -= other.m_z;
		return *this;
	}

	/*! Adds another vector to this vector and returns the result. */
	Vector3D operator+(const Vector3D & other) const {
		Vector3D tmp = *this;
		tmp += other;
		return tmp;
	}

	/*! Subtracts another vector from this vector and returns the result. */
	Vector3D operator-(const Vector3D & other) const {
		Vector3D tmp = *this;
		tmp -= other;
		return tmp;
	}

};

/*! Scales all components of the vector with the scalar \a scalar. */
inline IBKMK::Vector3D operator*(double scalar, const IBKMK::Vector3D &other) {
	IBKMK::Vector3D tmp = other;
	tmp *= scalar;
	return tmp;
}

} // namespace IBKMK


#endif // IBKMK_Vector3DH
