#include "SVLogWidget.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <QDebug>
#include <QApplication>

#include <QtExt_Directories.h>

#include <IBK_messages.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVLogWidget::SVLogWidget(QWidget *parent) :
	QWidget(parent)
{
	QVBoxLayout * lay = new QVBoxLayout;

	// *** setup textedit ***
	m_textEdit = new QPlainTextEdit(this);

	SVStyle::instance().formatPlainTextEdit(m_textEdit);

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
			html = html.arg(SVStyle::instance().m_logWarningText.name());
		else if (line.indexOf("[Error") != -1)
			html = html.arg(SVStyle::instance().m_logErrorText.name());
		else
			html = html.arg(SVStyle::instance().m_logProgressText.name());
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
			html = html.arg(SVStyle::instance().m_logProgressText.name());
			break;
		case IBK::MSG_WARNING :
			html = html.arg(SVStyle::instance().m_logWarningText.name());
			break;
		case IBK::MSG_ERROR :
			html = html.arg(SVStyle::instance().m_logErrorText.name());
			break;
		case IBK::MSG_DEBUG :
			html = html.arg(SVStyle::instance().m_logDebugText.name());
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
