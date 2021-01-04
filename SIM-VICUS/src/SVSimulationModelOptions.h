#ifndef SVSimulationModelOptionsH
#define SVSimulationModelOptionsH

#include <QWidget>


namespace QtExt {
	class ValidatingLineEdit;
}

namespace NANDRAD {
	class SimulationParameter;
	class Location;
}

namespace Ui {
	class SVSimulationModelOptions;
}

/*! Edit widget with general simulation/model options.  */
class SVSimulationModelOptions : public QWidget {
	Q_OBJECT
public:
	/*! Constructor, takes solver settings object reference (object is stored in main start dialog). */
	explicit SVSimulationModelOptions(QWidget *parent,
									  NANDRAD::SimulationParameter & solverParams, NANDRAD::Location & location);
	~SVSimulationModelOptions();

	/*! Updates user interface with properties from the project data structure.
		This function is called, whenever the dialog is first shown.
	*/
	void updateUi();

private slots:
	void on_comboBoxSolarDistributionModeltype_currentIndexChanged(int index);

private:

	/*! UI pointer. */
	Ui::SVSimulationModelOptions	*m_ui;

	// Data storage locations, synchronized with user interface.
	NANDRAD::SimulationParameter	*m_simParams = nullptr;
	NANDRAD::Location				*m_location = nullptr;

};

#endif // SVSimulationModelOptionsH
