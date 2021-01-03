#ifndef SVSimulationModelOptionsH
#define SVSimulationModelOptionsH

#include <QWidget>


namespace QtExt {
	class ValidatingLineEdit;
}

namespace NANDRAD {
	class SimulationParameter;
}

namespace Ui {
	class SVSimulationModelOptions;
}

/*! Edit widget with general simulation/model options.  */
class SVSimulationModelOptions : public QWidget {
	Q_OBJECT
public:
	/*! Constructor, takes solver settings object reference (object is stored in main start dialog). */
	explicit SVSimulationModelOptions(QWidget *parent, NANDRAD::SimulationParameter & solverParams);
	~SVSimulationModelOptions();

	/*! Updates user interface with properties from the project data structure.
		This function is called, whenever the dialog is first shown.
	*/
	void updateUi();

private:

	/*! UI pointer. */
	Ui::SVSimulationModelOptions	*m_ui;

	/*! Data storage location, synchronized with user interface. */
	NANDRAD::SimulationParameter	*m_simParams = nullptr;

};

#endif // SVSimulationModelOptionsH
