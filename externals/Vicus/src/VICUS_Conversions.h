#ifndef VICUS_CONVERSIONS_H
#define VICUS_CONVERSIONS_H

#include <QVector3D>
#include <QVector4D>
#include <IBKMK_Vector3D.h>
#include <QColor>
#include <IBK_MultiLanguageString.h>

namespace VICUS {

/*! IBKMK::Vector3D to QVector3D conversion macro. */
inline QVector3D IBKVector2QVector(const IBKMK::Vector3D & v) {
	return QVector3D((float)v.m_x, (float)v.m_y, (float)v.m_z);
}

/*! QVector3D to IBKMK::Vector3D to conversion macro. */
inline IBKMK::Vector3D QVector2IBKVector(const QVector3D & v) {
	return IBKMK::Vector3D((double)v.x(), (double)v.y(), (double)v.z());
}

inline QVector3D QVector3DFromQColor(const QColor & c) {
	return QVector3D((float)c.redF(), (float)c.greenF(), (float)c.blueF());
}

inline QVector4D QVector4DFromQColor(const QColor & c, float alpha) {
	return QVector4D((float)c.redF(), (float)c.greenF(), (float)c.blueF(), alpha);
}

inline QString IBKVector2String(const IBKMK::Vector3D & v) {
	return QString("[%1,%2,%3]").arg(v.m_x).arg(v.m_y).arg(v.m_z);
}

inline QString MultiLangString2QString(const IBK::MultiLanguageString & mls) {
	return QString::fromStdString(mls.string(IBK::MultiLanguageString::m_language, "en"));
}

} // VICUS

#endif // VICUS_CONVERSIONS_H
