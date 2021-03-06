#ifndef SVStyleH
#define SVStyleH

#include "SVSettings.h"

#include <QString>

class QPlainTextEdit;
class QWidget;
class QTableView;
class QTreeView;

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

	static void formatDatabaseTableView(QTableView * v);
	static void formatDatabaseTreeView(QTreeView * v);

	/*! Replaces all color text placeholders with colors based on the current style sheet. */
	static void formatWelcomePage(QString & htmlCode);

	/*! Sets the application wide style sheet. */
	void setStyle(SVSettings::ThemeType dark);

	/*! Returns a randomized color.
		Subsequent calls to this function generate sufficiently different colors.
	*/
	static QColor randomColor();

	QString				m_styleSheet;

	QColor				m_alternativeBackgroundDark; // TODO
	QColor				m_alternativeBackgroundBright; // TODO
	QColor				m_alternativeBackgroundText; // TODO
	QColor				m_readOnlyEditFieldBackground;		// TODO : subtitute with QtExt::Style::ReadOnlyEditFieldBackground
	QColor				m_alternativeReadOnlyEditFieldBackground; // TODO
	QColor				m_errorEditFieldBackground; // TODO


	QColor				m_logProgressText;
	QColor				m_logErrorText;
	QColor				m_logWarningText;
	QColor				m_logDebugText;

private:
	static SVStyle		*m_self;


	/*! The hue value of the last randomly generated color. */
	static double		m_randomColorHueValue;

	unsigned int		m_fontMonoSpacePointSize;
	QString				m_fontMonoSpace;
};

#endif // SVStyleH
