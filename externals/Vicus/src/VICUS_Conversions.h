#ifndef VICUS_CONVERSIONS_H
#define VICUS_CONVERSIONS_H

#include <QVector3D>
#include <IBKMK_Vector3D.h>

namespace VICUS {

/*! IBKMK::Vector3D to QVector3D conversion macro. */
inline QVector3D IBKVector2QVector(const IBKMK::Vector3D & v) {
	return QVector3D((float)v.m_x, (float)v.m_y, (float)v.m_z);
}

/*! QVector3D to IBKMK::Vector3D to conversion macro. */
inline IBKMK::Vector3D QVector2IBKVector(const QVector3D & v) {
	return IBKMK::Vector3D((double)v.x(), (double)v.y(), (double)v.z());
}

} // VICUS

#endif // VICUS_CONVERSIONS_H
