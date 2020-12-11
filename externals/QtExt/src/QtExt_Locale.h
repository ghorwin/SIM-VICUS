#ifndef QtExt_LocaleH
#define QtExt_LocaleH

#include <QLocale>

namespace QtExt {

/*! Simple wrapper class around QLocale that sets the QLocale::RejectGroupSeparator number option to
	prevent a value of '1.234' to be parsed as 1234 when using german locales.
	\code
	// instead of QLocale().toDouble() simply use

	bool ok;
	double val = QtExt::Locale().toDouble(text, &ok);
	\endcode
*/
class Locale : public QLocale {
public:
	Locale();
};

} // namespace QtExt

#endif // QtExt_LocaleH
