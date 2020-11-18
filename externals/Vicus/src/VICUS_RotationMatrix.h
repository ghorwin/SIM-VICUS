#ifndef VICUS_ROTATIONMATRIX_H
#define VICUS_ROTATIONMATRIX_H

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

#endif // VICUS_ROTATIONMATRIX_H
