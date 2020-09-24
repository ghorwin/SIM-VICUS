#ifndef SVLogWidgetH
#define SVLogWidgetH

#include <QDialog>

class QPlainTextEdit;

/*! A widget with empedded text browser to show the content of the application log file. */
class SVLogWidget : public QWidget {
	Q_OBJECT
public:
	explicit SVLogWidget(QWidget *parent = nullptr);

	/*! Shows application log file (for use in dialog). */
	void showLogFile( const QString & path);

public slots:
	/*! Clears the text browser, connected to the project handler's signal when
		a new project has been read.
	*/
	void clear();
	/*! Connected to message handler, appends the new message to the output (for use in dock widget). */
	void onMsgReceived(int type, QString msgText);

private:
	QPlainTextEdit * m_textEdit;
};

#endif // SVLogWidgetH
