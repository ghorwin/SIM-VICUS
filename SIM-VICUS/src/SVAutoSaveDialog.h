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

/*! Class that contains all auto-save features.
	Such as a timer that emits a signal and brings the Project Handler to perform an auto-save.
	Also a dialog is shown, when auto-saves have been detected and can be restored.
*/
class SVAutoSaveDialog : public QDialog {
Q_OBJECT
public:

	/*! Struct to encapsulate all meta-data from auto-save. */
	struct AutoSaveData {

		AutoSaveData() {}

		AutoSaveData(const QString &fileName, const QString &hash, const QString &timeStamp, const QString & basePath) :
			m_fileName(fileName),
			m_hash(hash),
			m_timeStamp(timeStamp),
			m_basePath(basePath)
		{}

		QString			m_fileName;		///>	filename of auto-save file
		QString			m_hash;			///>	hash of auto-save file
		QString			m_timeStamp;	///>    timestamp of auto-save file
		QString			m_basePath;		///>	basePath of auto save file

	};

	enum AutosaveColumns {
		AC_FileName,
		AC_Hash,
		AC_BasePath,
		AC_TimeStamp,
	};

	explicit SVAutoSaveDialog(QDialog *parent = nullptr);
	~SVAutoSaveDialog() override;

	/*! Checks whether autosaves were found.
		\param autoSaves contains all found autosaves
		\returns true, when autosaves exist
	*/
	bool checkForAutoSaves();

	/*! Removes all auto-saves when project will be closed. */
	void removeProjectSepcificAutoSaves(const QString &projectName);

	/*! Starts auto-save dialog on start-up, when auto-saves have been found.
		Shows all information about auto-saved files (Name, time, folder)
	*/
	void handleAutoSaves();

	/*! Updates table with all found auto-saves from m_autoSaveData. */
	void updateAutoSavesInTable();

	/*! Writes updated auto-save data to "autosave-metadata.info". */
	void writeAutoSaveData();

private slots:
	void onTimerFinished();

	void on_pushButtonRecoverFile_pressed();

	void on_pushButtonRemoveAutoSave_clicked();

signals:
	/*! Is always emitted, when an auto-save has to be done. */
	void autoSave();

private:
	/*! Pointer to Ui. */
	Ui::SVAutoSaveDialog		*m_ui;

	/*! Timer for auto-save periods. */
	QTimer						*m_timer = nullptr;

	/*! Cashed auto-save data. */
	std::vector<AutoSaveData>	m_autoSaveData;
};

#endif // SVAutoSaveDialogH
