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


	/*! Returns a pointer to the SVAutoSaveDialog instance.
		Only access this function during the lifetime of the
		SVAutoSaveDialog instance.
	*/
	static SVAutoSaveDialog & instance();

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
		AC_BasePath,
	};

	explicit SVAutoSaveDialog(QDialog *parent = nullptr);
	~SVAutoSaveDialog() override;

	/*! Checks whether autosaves were found.
		\param autoSaves contains all found autosaves
		\returns true, when autosaves exist
	*/
	bool checkForAutoSaves();


	/*! Starts auto-save dialog on start-up, when auto-saves have been found.
		Shows all information about auto-saved files (Name, time, folder)
	*/
	void handleAutoSaves();

	/*! Updates table with all found auto-saves from m_autoSaveData. */
	void updateAutoSavesInTable();

	/*! Writes updated auto-save data to "autosave-metadata.info". */
	void writeAutoSaveData();

	/*! Updates Ui with current data. */
	void updateUi();

	/*! Removes all auto-saved project files.
		\param basePath base path of project
		\param projectName name of project file
	*/
	void removeProjectFiles(const QString &basePath, const QString &projectName);

	/*! Restarts the timer without doing any autosaving. */
	void restartTimerWithoutAutosaving();

	/*! Recover currently selected file with selected timestamp.
		\returns true if wrong data was specified in saving dialog and false when recovering file is broken
	*/
	bool recoverFile();

	/*! Removes auto-save file. */
	void removeAutosave();

private slots:
	void onTimerFinished();

	void on_pushButtonRecoverFile_pressed();

	void on_pushButtonRemoveAutoSave_clicked();

	void on_pushButtonDiscard_clicked();

	/*! Removes all auto-saves when project will be closed.
		\param projectName name of project for auto-saves to be removed.
	*/
	void onRemoveProjectSepcificAutoSaves(const QString &projectName);

	void on_radioButtonLatestAutoSave_toggled(bool checked);

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

	/*! Current index in m_autoSaveData. */
	unsigned int				m_currenIdx;

	/*! To currently selected line connected lines. */
	std::vector<unsigned int>	m_correspondingLines;

	/*! Shows only latest autosave. */
	bool						m_showOnlyLatestAutosave = true;

	/*! Pointer to AutoSaveDialog. */
	static	SVAutoSaveDialog	*m_self;
};

#endif // SVAutoSaveDialogH
