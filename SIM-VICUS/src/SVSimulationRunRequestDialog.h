#ifndef SVSimulationRunRequestDialogH
#define SVSimulationRunRequestDialogH

#include <QDialog>

namespace Ui {
class SVSimulationRunRequestDialog;
}

/*! This dialog is shown when a user re-runs a simulation and and already result files exist.
	Then the user can choose between starting the simulation from scratch, hereby removing
	all existing files or continuing simulation from last output time point.
*/
class SVSimulationRunRequestDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVSimulationRunRequestDialog(QWidget *parent = 0);
	~SVSimulationRunRequestDialog();

	enum SimulationStartType {
		Normal,
		Continue,
		TestInit,
		DoNotRun
	};

	/*! Use this function instead of exec(). */
	SimulationStartType askForOption();

private slots:
	void on_pushButtonStart_clicked();

	void on_pushButtonContinue_clicked();

	void on_checkBoxClearResultDir_toggled(bool checked);

private:
	Ui::SVSimulationRunRequestDialog *m_ui;

	SimulationStartType m_simulationStartType;
};

#endif // SVSimulationRunRequestDialogH
