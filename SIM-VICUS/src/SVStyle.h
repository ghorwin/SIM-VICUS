#ifndef SVStyleH
#define SVStyleH

#include "SVSettings.h"

#include <QString>

class QPlainTextEdit;
class QWidget;

/*! Central implementation and configuration of visual appearance of the software.
	This class provides several member functions that can be used to tune the appearance of the software.

	Instantiate exactly one instance of this class right *after* creating the application object but *before*
	creating the first widgets.
*/
class SVStyle {
public:
	SVStyle();

	/*! Returns the instance of the singleton. */
	static SVStyle & instance();

	void formatPlainTextEdit(QPlainTextEdit * textEdit) const;

	static void formatWidgetWithLayout(QWidget * w);

	/*! Replaces all color text placeholders with colors based on the current style sheet. */
	static void formatWelcomePage(QString & htmlCode);

	/*! Sets the application wide style sheet. */
	void setStyle(SVSettings::ThemeType dark);

	QString				m_styleSheet;

private:
	static SVStyle		*m_self;

	unsigned int		m_fontMonoSpacePointSize;
	QString				m_fontMonoSpace;
};

#endif // SVStyleH
