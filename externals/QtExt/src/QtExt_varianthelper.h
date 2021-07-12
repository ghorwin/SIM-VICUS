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

#ifndef QtExt_varianthelperH
#define QtExt_varianthelperH

#include <QVariant>

namespace QtExt {

/*! Class in order to make pointer to/from QVariant conversions more userfriendly.
	\code
	// Example for using:
	MyClass *p;
	QVariant v = VPtr<MyClass>::asQVariant(p);

	MyClass *p1 = VPtr<MyClass>::asPtr(v);
	\endcode
*/
template <class T> class VPtr {
public:
	static T* asPtr(QVariant v) {
		return  (T *) v.value<void *>();
	}

	static QVariant asQVariant(T* ptr) {
		return qVariantFromValue((void *) ptr);
	}
};

} // end namespace QtExt

#endif // QtExt_varianthelperH
