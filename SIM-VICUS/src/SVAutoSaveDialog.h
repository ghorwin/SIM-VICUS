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


#ifndef SVAutoSaveDialogH
#define SVAutoSaveDialogH

#include <QDialog>
#include <QObject>
#include <QTimer>

namespace Ui {
	class SVAutoSaveDialog;
}


/*! Class that contains all auto-save features. Such as a timer that emits a signal and
	brings the MainWindow to perform an auto-save.
*/
class SVAutoSaveDialog : public QDialog {
Q_OBJECT
public:
	explicit SVAutoSaveDialog(QDialog *parent = nullptr);
	~SVAutoSaveDialog() override;

	/*! Checks whether autosaves were found.
		\param autoSaves contains all found autosaves
		\returns true, when autosaves exist
	*/
	void extracted(QStringList &files);
	bool checkForAutoSaves(std::vector<QString> &autoSaves);

	/*! Removes all auto-saves when project will be closed. */
	void removeAutoSaves();

	/*! .*/
	void handleAutoSaves();

private slots:
	void onTimerFinished();

	void on_pushButtonRecoverFile_pressed();

signals:
	void autoSave();

private:
	Ui::SVAutoSaveDialog		*m_ui;

	/*! Timer for auto-save periods. */
	QTimer						*m_timer = nullptr;
};

#endif // SVAutoSaveDialogH
