/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

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
