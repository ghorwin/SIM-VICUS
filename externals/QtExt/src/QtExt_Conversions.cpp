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

#include "QtExt_Conversions.h"
#include "QtExt_Locale.h"

namespace QtExt {

double QString2Double(const QString & str, bool * ok) {
	bool myOk;
	double val;
	val = QtExt::Locale().toDouble(str, &myOk);
	if (!myOk) {
		val = str.toDouble(&myOk);
		if (!myOk) {
			if (ok != nullptr)
				*ok = false;
			return 0;
		}
	}
	if (ok != nullptr)
		*ok = true;
	return val;
}


bool QString2Parameter(const QString & str, const std::string & keywordName, IBK::Parameter & para) {

	// split up value and unit
	QStringList tokens = str.split(QRegExp(" |\t"));
	if (tokens.count() != 2)
		return false; // fail

	bool ok;
	double val = QString2Double(tokens[0], &ok);
	if (!ok)
		return false; // failed

	// attempt unit conversion
	try {
		IBK::Unit u(tokens[1].toStdString());
		// success, set keyword name and return
		para.set(keywordName, val, u);
	}
	catch (...) {
		return false; // failed to recognize unit
	}

	return true;

}


QString parameter2String(const IBK::Parameter & para) {
	if (para.IO_unit.id() == 0)
		return QString("%L1 -").arg(para.value);
	else {
		return QString("%L1 %2")
			.arg(para.get_value())
			.arg(QString::fromStdString(para.IO_unit.name()));
	}
}

} // QtExt

