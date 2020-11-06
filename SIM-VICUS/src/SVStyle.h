#ifndef SVSTYLE_H
#define SVSTYLE_H

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

private:
	static SVStyle		*m_self;
	QString				m_styleSheet;

	unsigned int		m_fontMonoSpacePointSize;
	QString				m_fontMonoSpace;
};

#endif // SVSTYLE_H
