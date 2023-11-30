#ifndef SVSIMULATIONSTARTOPTIONSH
#define SVSIMULATIONSTARTOPTIONSH

#include <QWidget>

namespace Ui {
class SVSimulationStartOptions;
}

class ModificationInfo;
class SVSimulationRunRequestDialog;
class SVSimulationOutputOptions;

class SVSimulationStartOptions : public QWidget
{
	Q_OBJECT

public:
	explicit SVSimulationStartOptions(QWidget *parent = nullptr);
	~SVSimulationStartOptions();

	/*! Starts the simulation, either in test-init mode or regular mode.
		Returns false if some error occurred during creation of the NANDRAD project.
	*/
	bool startSimulation(bool testInit, bool forceForegroundProcess = false, bool waitForFinishedProcess = false, bool calculateViewFactors=true);

	void showScreenLog();

	void updateUi();

	/*! Pointer to simulation output options */
	SVSimulationOutputOptions		*m_simulationOutputOptions = nullptr;

public slots:

	void onModified(int modificationType, ModificationInfo */*data*/);

private slots:
	void on_checkBoxCloseConsoleWindow_clicked();

	void on_checkBoxStepStats_clicked();

	void on_lineEditStartDate_editingFinished();

	void on_lineEditEndDate_editingFinished();

	void on_lineEditDuration_editingFinishedSuccessfully();

	void on_comboBoxTermEmulator_currentIndexChanged(int index);

	void on_lineEditStartDate_returnPressed();

	void on_lineEditEndDate_returnPressed();

	void on_lineEditNumThreads_editingFinishedSuccessfully();

	void on_comboBoxVerboseLevel_currentIndexChanged(int);

	void on_pushButtonRun_clicked();

	void on_pushButtonTestInit_clicked();

	void on_pushButtonExportFMU_clicked();

	void on_pushButtonShowScreenLog_clicked();

private:

	/*! Generates a NANDRAD project. */
	bool generateNANDRAD(QString & resultPath);

	/*! Composes correct command line (stored in m_cmdArgs). */
	void updateCmdLine();

	void updateTimeFrameEdits();

	Ui::SVSimulationStartOptions	*m_ui;

	QStringList						m_commandLineArgs;

	SVSimulationRunRequestDialog	*m_simulationRunRequestDialog = nullptr;
};

#endif // SVSIMULATIONSTARTOPTIONSH
