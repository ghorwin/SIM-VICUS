#ifndef SVSimulationStartNandradH
#define SVSimulationStartNandradH

#include <QDialog>

#include <NANDRAD_Project.h>

namespace Ui {
	class SVSimulationStartNandrad;
}

class SVSimulationPerformanceOptions;
class SVSimulationLocationOptions;
class SVSimulationOutputOptions;

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

private:
	/*! Composes correct command line (stored in m_cmdArgs). */
	void updateCmdLine();
	/*! Generates a NANDRAD project from the currently given VICUS project data. */
	bool generateNandradProject(NANDRAD::Project & p) const;
	/*! Stores current input into project data structure. */
	void storeInput();

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

	/*! Cache for data edited in this dialog.
		Transferred to project, when simulation is started or dialog
		is closed with "close".
	*/
	NANDRAD::SolverParameter		m_solverParams;
	NANDRAD::Location				m_location;
	NANDRAD::Outputs				m_outputs;
};

#endif // SVSimulationStartNandradH
