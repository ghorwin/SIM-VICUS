#include "QtExt_Locale.h"

namespace QtExt {

Locale::Locale() {
	setNumberOptions(QLocale::RejectGroupSeparator | QLocale::OmitGroupSeparator);
}

double Locale::toDoubleWithFallback(const QString & text, bool * ok) {
	Q_ASSERT(ok != nullptr);

	double val = toDouble(text, ok);
	if (ok != nullptr && *ok)
		return val;

	// fall-back on C-locale
	val = text.toDouble(ok);
	if (ok != nullptr && *ok)
		return val;
	else
		return 0;
}

} // namespace QtExt
