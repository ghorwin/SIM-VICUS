/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#ifndef QtExt_ConversionsH
#define QtExt_ConversionsH

#include <QVector3D>
#include <QVector4D>
#include <QColor>
#include <QString>

#include <IBK_MultiLanguageString.h>
#include <IBK_Path.h>
#include <IBK_Parameter.h>

namespace QtExt {

inline QVector3D QVector3DFromQColor(const QColor & c) {
	return QVector3D((float)c.redF(), (float)c.greenF(), (float)c.blueF());
}

inline QVector4D QVector4DFromQColor(const QColor & c, float alpha) {
	return QVector4D((float)c.redF(), (float)c.greenF(), (float)c.blueF(), alpha);
}


inline QString MultiLangString2QString(const IBK::MultiLanguageString & mls) {
	return QString::fromStdString(mls.string(IBK::MultiLanguageString::m_language, "en"));
}

/*! Utility function for conversion of a QString to an IBK::Path. */
inline IBK::Path QString2Path(const QString & str) {
	return IBK::Path(str.toStdString());
}

/*! Utility function for conversion of a QString to an IBK::Path. */
inline QString Path2String(const IBK::Path & p) {
	return QString::fromUtf8(p.c_str());
}

/*! Combines QLocale().toDouble() and str.toDouble(). */
double QString2Double(const QString & str, bool * ok = nullptr);

/*! Converts a text "121 m" into a parameter. */
bool QString2Parameter(const QString & str, const std::string & keywordName, IBK::Parameter & para);

/*! Converts a parameter to "12,2 h" using the currently installed locale (as opposed to the para.toString() member function.
	In case of missing/invalid unit, the result will be "<value> -".
*/
QString parameter2String(const IBK::Parameter & para);

} // QtExt

#endif // QtExt_ConversionsH
