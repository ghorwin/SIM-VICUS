#ifndef SVSimulationStartNandradH
#define SVSimulationStartNandradH

#include <QDialog>

#include <VICUS_Project.h>

namespace Ui {
	class SVSimulationStartNandrad;
}

class SVSimulationPerformanceOptions;
class SVSimulationLocationOptions;
class SVSimulationOutputOptions;
class SVSimulationModelOptions;
class SVSimulationRunRequestDialog;

/*! The start dialog for a NANDRAD simulation.
	Contains pages for all global simulation properties.

	The dialog holds a private copy of the global project. All changes in the dialog are made exclusively in
	this project. Only when the dialog is closes with the "save and store data" button, the calling function in
	SVMainWindow will create the respective undo-action.

	Compared to a view-style implementation of the edit controls in the main windows, we do not have to implement
	undo-actions for each individual property change in these settings.

	WARNING: within the entire dialog, do not access the global project() but always take the locally stored copy for
	direct read/write access to project data.
*/
class SVSimulationStartNandrad : public QDialog {
	Q_OBJECT

public:
	explicit SVSimulationStartNandrad(QWidget *parent = nullptr);
	~SVSimulationStartNandrad();

	int edit();

	/*! Returns a copy of the locally modified version of the project. */
	const VICUS::Project & localProject() const { return m_localProject; }

private slots:
	void on_pushButtonClose_clicked();

	void on_pushButtonRun_clicked();

	void on_checkBoxCloseConsoleWindow_toggled(bool checked);

	void on_checkBoxStepStats_toggled(bool checked);

	void on_pushButtonShowScreenLog_clicked();

	void on_lineEditDuration_editingFinishedSuccessfully();
	void on_lineEditStartDate_editingFinished();
	void on_lineEditEndDate_editingFinished();

	void on_comboBoxTermEmulator_currentIndexChanged(int index);

	void on_pushButtonTestInit_clicked();

private:
	/*! Composes correct command line (stored in m_cmdArgs). */
	void updateCmdLine();
	void updateTimeFrameEdits();

	/*! Starts the simulation, either in test-init mode or regular mode.
		Returns false if some error occurred during creation of the NANDRAD project.
	*/
	bool startSimulation(bool testInit);

	Ui::SVSimulationStartNandrad	*m_ui;

	/*! Local copy of our project data, modified in dialog and synced with global
		project when dialog is confirmed.
		This is also the cache for data edited in this dialog in the various edit pages.
	*/
	VICUS::Project					m_localProject;

	QString							m_solverExecutable;
	QString							m_nandradProjectFilePath;
	QStringList						m_cmdArgs;

	/*! Page with solver options. */
	SVSimulationPerformanceOptions	*m_simulationPerformanceOptions = nullptr;
	/*! Page with location options. */
	SVSimulationLocationOptions		*m_simulationLocationOptions = nullptr;
	/*! Page with output options. */
	SVSimulationOutputOptions		*m_simulationOutputOptions = nullptr;
	/*! Page with all other model options. */
	SVSimulationModelOptions		*m_simulationModelOptions = nullptr;

	SVSimulationRunRequestDialog	*m_simulationRunRequestDialog = nullptr;
};

#endif // SVSimulationStartNandradH
