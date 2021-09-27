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

#ifndef IBKMK_QuaternionH
#define IBKMK_QuaternionH

#include <iostream>
#include <cmath>

namespace IBKMK {

class Vector3D;

/*! This class implements a quaternion and the commonly used
	algebraic operations. The class works with double precicion.
	Quaternions are used to perform a three dimensional rotations arround given axis and
	angle with the help of hyper complex numbers and therefore, they represent an alternative
	implementation to rotation matrix algebra.
	Quaternions form four-dimensional associative normed division algebra with corresponding
	calculation rules (including addition, multiplicatiojn, division) as well as complex
	vector operation (normalization, conjugation, inverse).
	We use the representation: q = w + x * i + y * j + z * k with i, j, k denoting the complex
	dimensions.
*/
class Quaternion {

public:

	/*! Default constructor, creates an empty quaternion (all coefficients 0). */
	Quaternion() {}

	/*! Constructor, creates quaternion with given real and complex coefficients. */
	Quaternion(double w, double x, double y, double z);

	/*! Constructor, creates quaternion from angle and rotation axis. */
	Quaternion(double angle, const IBKMK::Vector3D &v);

	/*! Operator + for adding two quaternions. */
	Quaternion operator + (const Quaternion& q) const;

	/*! Operator + for subtracting two quaternions. */
	Quaternion operator - (const Quaternion& q) const;

	/*! Operator * for exterior product of two quaternions. */
	Quaternion operator * (const Quaternion& q) const;

	/*! Operator / , inverts esterior producr of two quadrions. */
	Quaternion operator / (Quaternion& q) const;

	/*! Operator += for adding another quaternion. */
	Quaternion& operator += (const Quaternion& q);

	/*! Operator -= for subtracting another quaternion. */
	Quaternion& operator -= (const Quaternion& q);

	/*! Operator *= for multiplying another quaternion (exterior product). */
	Quaternion& operator *= (const Quaternion& q);

	/*! Operator /= for dividing another quaternion (inverse of exterior product). */
	Quaternion& operator /= (Quaternion& q);

	/*! Operator != comparing two quaternions. */
	bool operator != (const Quaternion& q) const { return !(*this == q); }

	/*! Operator == comparing two quaternions. */
	bool operator == (const Quaternion& q) const ;

	//other methods: norm, inverse, conjugate, toEuler

	/*! Returns real coefficient. */
	double w() const { return m_w; }

	/*! Returns coefficient of complex dimension i. */
	double x() const { return m_x; }

	/*! Returns coefficient of complex dimension j. */
	double y() const { return m_y; }

	/*! Returns coefficient of complex dimension k. */
	double z() const { return m_z; }

	/*! Calculations the norm of quaternion. */
	double norm() const;

	/*! Calculations the magnitude (lenght) of quaternion. */
	double magnitude() const;

	/*! Returns rotation angle and the unit rotation axis corresponding to the quaternion.
		Assumes a unit quaternion (use normalize() to remove any noise due to floating point errors).
		If s >= 0, the angle will be on the interval [0,pi] .
		If s < 0, the angle will be on the interval (pi,2pi].
		To get a representation where the angle is on [0, pi], you can manually flip the sign of the returned unitAxis,
		and use the angle of 2pi-angle.
	*/
	void axisAndAngle(double &angle, IBKMK::Vector3D &v) const;

	/*! Caluclates the corresponding 3x3 rotation matrix in row major order.
		The quaternion is assumed to be a unit quaternion and R a vector of size 9.*/
	void rotationMatrix(double * R) const;

	/*! Scales quaternion with scalar s. */
	Quaternion scaled(double s) const;

	/*! Returns inverse of current quaternion. */
	Quaternion inverse() const;

	/*! Returns conjugated quaternion (== invers for a normalized quaternion). */
	Quaternion conjugated() const;

	/*! Return normalized quaternion (with magnitude = 1). */
	Quaternion normalized() const;

	/*! Rotated a given vector around quaternion axis by quaternion rotation angle. 	*/
	void rotateVector(IBKMK::Vector3D &v) const;

private:
	double m_w = 0;		///< Real coefficient (= null for non-initialized Quaternion)
	double m_x = 0;		///< Coefficient for complex dimension i (= null for non-initialized Quaternion)
	double m_y = 0;		///< Coefficient for complex dimension j (= null for non-initialized Quaternion)
	double m_z = 0;		///< Coefficient for complex dimension k (= null for non-initialized Quaternion)

	friend std::ostream& operator << (std::ostream& output, const Quaternion& q);
};

/*! Operator << for printing quaterion. */
inline std::ostream& operator << (std::ostream& output, const Quaternion& q) {
	output << "[" << q.m_w << ", " << "(" << q.m_x << ", " << q.m_y << ", " << q.m_z << ")]";
	return output;
}

}  // namespace IBKMK


/*! \file IBKMK_Quaternion.h
	\brief Contains the declaration of the class Quaternion.
*/

#endif // IBKMK_QuaternionH

