/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

*/

#ifndef QtExt_ConversionsH
#define QtExt_ConversionsH

#include <QVector3D>
#include <QVector4D>
#include <QColor>
#include <QString>

#include <IBKMK_Vector3D.h>
#include <IBK_MultiLanguageString.h>
#include <IBK_Path.h>
#include <IBK_Parameter.h>

namespace QtExt {

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

/*! Utility function for conversion of a QString to an IBK::Path. */
inline IBK::Path QString2Path(const QString & str) {
	return IBK::Path(str.toUtf8().data());
}

/*! Utility function for conversion of a QString to an IBK::Path. */
inline QString Path2String(const IBK::Path & p) {
	return QString::fromUtf8(p.c_str());
}

/*! Combines QLocale().toDouble() and str.toDouble(). */
double QString2Double(const QString & str, bool * ok = nullptr);

/*! Converts a text "121 m" into a parameter. */
bool QString2Parameter(const QString & str, const std::string & keywordName, IBK::Parameter & para);


} // QtExt

#endif // QtExt_ConversionsH
