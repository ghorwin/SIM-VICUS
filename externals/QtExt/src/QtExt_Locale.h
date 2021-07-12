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

#ifndef QtExt_LocaleH
#define QtExt_LocaleH

#include <QLocale>

namespace QtExt {

/*! Simple wrapper class around QLocale that sets the QLocale::RejectGroupSeparator number option to
	prevent a value of '1.234' to be parsed as 1234 when using german locales.
	\code
	// instead of QLocale().toDouble() simply use

	bool ok;
	// convert only using the current locale
	double val = QtExt::Locale().toDouble(text, &ok);

	// convert using current locale and fall-back to C-locale
	double val2 = QtExt::Locale().toDoubleWithFallback(text, &ok);
	\endcode
*/
class Locale : public QLocale {
public:
	Locale();

	/*! If conversion fails, 0 is returned. */
	double toDoubleWithFallback(const QString & text, bool * ok);
};

} // namespace QtExt

#endif // QtExt_LocaleH
