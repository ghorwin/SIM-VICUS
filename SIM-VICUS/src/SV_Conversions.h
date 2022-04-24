#ifndef SV_ConversionsH
#define SV_ConversionsH

#include <QtExt_Conversions.h>

#include <QQuaternion>

#include <IBKMK_Vector3D.h>
#include <IBKMK_Quaternion.h>

/*! IBKMK::Vector3D to QVector3D conversion macro. */
inline QVector3D IBKVector2QVector(const IBKMK::Vector3D & v) {
	return QVector3D((float)v.m_x, (float)v.m_y, (float)v.m_z);
}

/*! QVector3D to IBKMK::Vector3D to conversion macro. */
inline IBKMK::Vector3D QVector2IBKVector(const QVector3D & v) {
	return IBKMK::Vector3D((double)v.x(), (double)v.y(), (double)v.z());
}


inline IBKMK::Quaternion QQuaternion2IBKQuaternion(const QQuaternion & q) {
	return IBKMK::Quaternion((double)q.scalar(), (double)q.x(), (double)q.y(), (double)q.z());
}

inline QString IBKVector2String(const IBKMK::Vector3D & v) {
	return QString("[%1,%2,%3]").arg(v.m_x).arg(v.m_y).arg(v.m_z);
}

#endif // SV_ConversionsH
