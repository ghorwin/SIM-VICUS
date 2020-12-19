/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

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
