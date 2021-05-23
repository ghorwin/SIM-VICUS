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
