/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>
	  
	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef VICUS_RotationMatrixH
#define VICUS_RotationMatrixH

#include <QQuaternion>

#include "VICUS_CodeGenMacros.h"

namespace VICUS {

/*! This is a pretty dumb class and only used for the serialization code generator.
	It stores 4 floats and matches internally the QQuaternion class.
	When we copy back and forth from a QQuaternion, we just treat ourselves as a QQuaternion class
	(low level memory access is can be beautiful :-)
*/
class RotationMatrix {
public:
	RotationMatrix() {}
	RotationMatrix(const QQuaternion & q) {
		setQuaternion(q);
	}

	/*! Conversion from QQuaternion */
	void setQuaternion(const QQuaternion & q) {
		*(QQuaternion*)(&m_wp) = q;
	}

	/*! Conversion to QQuaternion */
	QQuaternion toQuaternion() const {
		return QQuaternion(*(const QQuaternion*)(&m_wp));
	}

	VICUS_READWRITE
	VICUS_COMP(RotationMatrix)

	float m_wp;	// XML:E:required
	float m_x;	// XML:E:required
	float m_y;	// XML:E:required
	float m_z;	// XML:E:required
};


inline bool RotationMatrix::operator!=(const RotationMatrix & other) const {
	if (m_wp != other.m_wp) return true;
	if (m_x != other.m_x) return true;
	if (m_y != other.m_y) return true;
	if (m_z != other.m_z) return true;
	return false;
}

} // namespace VICUS

#endif // VICUS_RotationMatrixH
