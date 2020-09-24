#include "SVLogWidget.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>

#include <QtExt_Directories.h>

#include <IBK_messages.h>

#include "SVConstants.h"

SVLogWidget::SVLogWidget(QWidget *parent) :
	QWidget(parent)
{
	QVBoxLayout * lay = new QVBoxLayout;

	// *** setup textedit ***
	m_textEdit = new QPlainTextEdit(this);


	/// \todo Move any font adjustment to some central "styling" class
	// customize log window
#ifdef Q_OS_MACX
	QFont f;
	f.setFamily("Monaco");
	f.setPointSize(12);
	m_textEdit->setFont(f);
#elif defined(Q_OS_UNIX)
	QFont f;
	f.setFamily("monospace");
	f.setPointSize(9);
	m_textEdit->setFont(f);
#endif // Q_OS_UNIX
#ifdef Q_OS_WIN
	QFont f;
	f.setFamily("Courier New");
	f.setPointSize(9);
	m_textEdit->setFont(f);
#endif // Q_OS_WIN
	m_textEdit->setContextMenuPolicy(Qt::NoContextMenu);
	m_textEdit->setReadOnly(true);
	m_textEdit->setUndoRedoEnabled(false);
	m_textEdit->setWordWrapMode(QTextOption::NoWrap);

	lay->addWidget(m_textEdit);
	lay->setMargin(0);
	setLayout(lay);

	setMinimumWidth(500);
	setMinimumHeight(80);
}


void SVLogWidget::showLogFile( const QString & logFilePath ) {
	m_textEdit->clear();
	QFile f(logFilePath);
	if (!f.open(QFile::ReadOnly)) {
		m_textEdit->appendPlainText(tr("Cannot open logfile '%1'.").arg(logFilePath));
		return;
	}
	QTextStream strm(&f);
	QString line = strm.readLine();
	while (!line.isNull()) {
		QString html = QString("<span style=\"white-space:pre; color:%2\">%1</span>").arg(line);
		// color lines according to message type
		if (line.indexOf("[Warning") != -1)
			html = html.arg("#aa8800");
		else if (line.indexOf("[Error") != -1)
			html = html.arg("#bb0000");
		else
			html = html.arg("#000000");
		m_textEdit->appendHtml(html);
		line = strm.readLine();
	}
}


void SVLogWidget::onMsgReceived(int type, QString msgString) {
	// avoid empty lines between messages
	if (msgString.endsWith('\n'))
		msgString.chop(1);
	if (msgString.endsWith("\r\n"))
		msgString.chop(2);

	QString html = QString("<span style=\"white-space:pre; color:%2\">%1</span>").arg(msgString);
	// color lines according to message type
	switch (type) {
		case IBK::MSG_PROGRESS :
		case IBK::MSG_CONTINUED :
			html = html.arg("#000000");
			break;
		case IBK::MSG_WARNING :
			html = html.arg("#aa8800");
			break;
		case IBK::MSG_ERROR :
			html = html.arg("#bb0000");
			break;
		case IBK::MSG_DEBUG :
			html = html.arg("#808000");
			break;
	}

	// get position of scroll bar
	int pos = m_textEdit->verticalScrollBar()->value();
	int maxpos = m_textEdit->verticalScrollBar()->maximum();
	m_textEdit->appendHtml(html);
	// if we were at the end, go again to the end
	if (pos == maxpos)
		m_textEdit->verticalScrollBar()->setValue( m_textEdit->verticalScrollBar()->maximum() );
}


void SVLogWidget::clear() {
	m_textEdit->clear();
}
