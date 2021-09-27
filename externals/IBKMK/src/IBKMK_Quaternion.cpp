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


#include "IBKMK_Quaternion.h"
#include "IBKMK_Vector3D.h"

namespace IBKMK {

Quaternion::Quaternion(double w, double x, double y, double z) {
	m_w = w;
	m_x = x;
	m_y = y;
	m_z = z;
}


Quaternion::Quaternion(double angle, const IBKMK::Vector3D &v) {
	m_w = cos(angle/2.0);
	double sin2 = sin(angle/2.0);
	m_x = sin2 * v.m_x;
	m_y = sin2 * v.m_y;
	m_z = sin2 * v.m_z;
}


Quaternion Quaternion::operator + (const Quaternion& q) const {
	return Quaternion(m_w+q.m_w, m_x+q.m_x, m_y+q.m_y, m_z+q.m_z);
}


Quaternion Quaternion::operator - (const Quaternion& q) const {
	return Quaternion(m_w-q.m_w, m_x-q.m_x, m_y-q.m_y, m_z-q.m_z);
}


Quaternion Quaternion::operator * (const Quaternion& q) const {
	return Quaternion(
				m_w*q.m_w - m_x*q.m_x - m_y*q.m_y - m_z*q.m_z,
				m_w*q.m_x + m_x*q.m_w + m_y*q.m_z - m_z*q.m_y,
				m_w*q.m_y + m_y*q.m_w + m_z*q.m_x - m_x*q.m_z,
				m_w*q.m_z + m_z*q.m_w + m_x*q.m_y - m_y*q.m_x);
}


Quaternion Quaternion::operator / (Quaternion& q) const {
	return ((*this) * (q.inverse()));
}


Quaternion& Quaternion::operator += (const Quaternion& q) {
	m_w += q.m_w;
	m_x += q.m_x;
	m_y += q.m_y;
	m_z += q.m_z;

	return (*this);
}


Quaternion& Quaternion::operator -= (const Quaternion& q) {
	m_w -= q.m_w;
	m_x -= q.m_x;
	m_y -= q.m_y;
	m_z -= q.m_z;

	return (*this);
}


Quaternion& Quaternion::operator *= (const Quaternion& q) {
	double w_val = m_w*q.m_w - m_x*q.m_x - m_y*q.m_y - m_z*q.m_z;
	double x_val = m_w*q.m_x + m_x*q.m_w + m_y*q.m_z - m_z*q.m_y;
	double y_val = m_w*q.m_y + m_y*q.m_w + m_z*q.m_x - m_x*q.m_z;
	double z_val = m_w*q.m_z + m_z*q.m_w + m_x*q.m_y - m_y*q.m_x;

	m_w = w_val;
	m_x = x_val;
	m_y = y_val;
	m_z = z_val;

	return (*this);
}


Quaternion& Quaternion::operator /= (Quaternion& q) {
	(*this) = (*this)*q.inverse();
	return (*this);
}


bool Quaternion::operator == (const Quaternion& q) const {
	return (m_w==q.m_w && m_x==q.m_x && m_y==q.m_y && m_z==q.m_z) ? true : false;
}


double Quaternion::norm() const {
	return (m_w*m_w + m_x*m_x + m_y*m_y + m_z*m_z);
}


double Quaternion::magnitude() const {
	return std::sqrt(norm());
}


void Quaternion::axisAndAngle(double &angle, IBKMK::Vector3D &v) const {
	angle = 0;
	if ((m_w >= 1.0) || (m_w <= -1.0)) {
		// invalidity; this check is necessary to avoid problems with acos if s is 1 + eps
		v.set(0,0,0);
		return;
	}

	angle = 2.0 * std::acos(m_w);
	double sin2 = m_x*m_x + m_y*m_y + m_z*m_z; //sin^2(*angle / 2.0)

	if (sin2 == 0.) {
		// identity rotation; angle is zero, any axis is equally good
		v.set(0,0,0);
	}
	else {
		double inv = 1.0 / std::sqrt(sin2); // note: *angle / 2.0 is on [0,pi], so sin(*angle / 2.0) >= 0, and therefore the sign of sqrt can be safely taken positive
		v.set(m_x * inv, m_y * inv, m_z * inv);
	}
}


void Quaternion::rotationMatrix(double * R) const {
	// first row
	R[0] = 1 - 2. * m_y * m_y - 2. * m_z * m_z;
	R[1] = 2. * m_x * m_y - 2. * m_w * m_z;
	R[2] = 2. * m_x * m_z + 2. * m_w * m_y;
	// second row
	R[3] = 2. * m_x * m_y + 2. * m_w * m_z;
	R[4] = 1 - 2. * m_x* m_x - 2. * m_z * m_z;
	R[5] = 2. * m_y* m_z - 2. * m_w * m_x;
	// third row
	R[6] = 2. * m_x * m_z - 2. * m_w * m_y;
	R[7] = 2. * m_y * m_z + 2. * m_w * m_x;
	R[8] = 1 - 2. * m_x * m_x - 2. * m_y * m_y;
}


Quaternion Quaternion::scaled(double  s) const {
	return Quaternion(m_w*s, m_x*s, m_y*s, m_z*s);
}


Quaternion Quaternion::inverse() const {
	return conjugated().scaled(1/norm());
}


Quaternion Quaternion::conjugated() const {
	return Quaternion(m_w, -m_x, -m_y, -m_z);
}


Quaternion Quaternion::normalized() const {
	return scaled(1.0/magnitude());
}


void Quaternion::rotateVector(IBKMK::Vector3D &v) const {
	Quaternion  q(0, v.m_x, v.m_y, v.m_z);
	q = (*this) * q * inverse();

	v.m_x = q.m_x;
	v.m_y = q.m_y;
	v.m_z = q.m_z;
}

}	// namespace IBKMK
