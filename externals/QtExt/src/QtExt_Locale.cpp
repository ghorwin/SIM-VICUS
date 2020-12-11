#include "QtExt_Locale.h"

namespace QtExt {

Locale::Locale() {
	setNumberOptions(QLocale::RejectGroupSeparator | QLocale::OmitGroupSeparator);
}

} // namespace QtExt
