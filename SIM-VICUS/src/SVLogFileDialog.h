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

#ifndef SVLogFileDialogH
#define SVLogFileDialogH

#include <QDialog>

namespace Ui {
	class SVLogFileDialog;
}

/*! A dialog that shows the a text/log file.
	Usage:
	\code
	SVLogFileDialog dlg;
	dlg.setLogFile("path/to/logfile.txt");
	dlg.exec();
	\endcode
*/
class SVLogFileDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVLogFileDialog(QWidget *parent = nullptr);
	~SVLogFileDialog();

	/*! Call this function to tell the dialog to load the contents of the
		log file into it.
	*/
	void setLogFile(const QString & logfilepath, QString projectfilepath, bool editFileButtonVisible);

private slots:
	/*! Connected to custom button in button box. */
	void onOpenFileClicked();

	/*! Connected to custom button in button box. */
	void onEditLogClicked();

	/*! Connected to custom button in button box. */
	void onReloadprojectClicked();

private:
	Ui::SVLogFileDialog		*m_ui;
	QPushButton				*m_pushButtonOpenLogInTextEditor;
	QPushButton				*m_pushButtonOpenFileInTextEditor;
	QPushButton				*m_pushButtonReloadProject;
	QString					m_projectFilePath;
	QString					m_logFilePath;
};

#endif // SVLogFileDialogH
