#ifndef SVSimulationStartNandradH
#define SVSimulationStartNandradH

#include <QDialog>

#include <NANDRAD_Project.h>
#include <VICUS_Outputs.h>

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
*/
class SVSimulationStartNandrad : public QDialog {
	Q_OBJECT

public:
	explicit SVSimulationStartNandrad(QWidget *parent = nullptr);
	~SVSimulationStartNandrad();

	int edit();

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
	/*! Generates a NANDRAD project from the currently given VICUS project data. */
	bool generateNandradProject(NANDRAD::Project & p);
	/*! Stores current input into project data structure. */
	void storeInput();
	void updateTimeFrameEdits();

	/*! Starts the simulation, either in test-init mode or regular mode. */
	void startSimulation(bool testInit);

	Ui::SVSimulationStartNandrad	*m_ui;

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

	/*  Cache for data edited in this dialog.
		Transferred to project, when simulation is started or dialog
		is closed with "close".
	*/
	NANDRAD::SolverParameter		m_solverParams;
	NANDRAD::SimulationParameter	m_simParams;
	NANDRAD::Location				m_location;
	VICUS::Outputs					m_outputs;

	SVSimulationRunRequestDialog	*m_simulationRunRequestDialog = nullptr;
};

#endif // SVSimulationStartNandradH
