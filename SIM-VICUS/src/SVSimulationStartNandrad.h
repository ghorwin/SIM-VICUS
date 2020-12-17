#ifndef SVSimulationStartNandradH
#define SVSimulationStartNandradH

#include <QDialog>

#include <NANDRAD_Project.h>

namespace Ui {
	class SVSimulationStartNandrad;
}

class SVSimulationPerformanceOptions;
class SVSimulationLocationOptions;

/*! The start dialog for a NANDRAD simulation.
	Contains pages for all global simulation properties.
*/
class SVSimulationStartNandrad : public QDialog {
	Q_OBJECT

public:
	explicit SVSimulationStartNandrad(QWidget *parent = nullptr);
	~SVSimulationStartNandrad();

	int edit();

private:
	Ui::SVSimulationStartNandrad	*m_ui;

	/*! Page with solver options. */
	SVSimulationPerformanceOptions	*m_simulationPerformanceOptions = nullptr;
	/*! Page with location options. */
	SVSimulationLocationOptions		*m_simulationLocationOptions = nullptr;

	/*! Cache for data edited in this dialog.
		Transferred to project, when simulation is started or dialog
		is closed with "close".
	*/
	NANDRAD::SolverParameter		m_solverParams;
	NANDRAD::Location				m_location;
};

#endif // SVSimulationStartNandradH
