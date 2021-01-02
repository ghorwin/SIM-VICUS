#ifndef SVSimulationOutputOptionsH
#define SVSimulationOutputOptionsH

#include <QWidget>

namespace Ui {
	class SVSimulationOutputOptions;
}

namespace NANDRAD {
	class Outputs;
}

/*! Widget with settings related to location. */
class SVSimulationOutputOptions : public QWidget {
	Q_OBJECT

public:
	SVSimulationOutputOptions(QWidget *parent, NANDRAD::Outputs & outputs);
	~SVSimulationOutputOptions();

	/*! Updates user interface with properties from the project data structure.
		This function is called whenever the dialog is first shown.
	*/
	void updateUi();

private:
	Ui::SVSimulationOutputOptions		*m_ui;
	NANDRAD::Outputs					*m_outputs;
};

#endif // SVSimulationOutputOptionsH
