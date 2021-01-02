#ifndef SVSimulationOutputOptionsH
#define SVSimulationOutputOptionsH

#include <QWidget>

namespace Ui {
	class SVSimulationOutputOptions;
}

namespace VICUS {
	class Outputs;
}

/*! Widget with settings related to location. */
class SVSimulationOutputOptions : public QWidget {
	Q_OBJECT

public:
	SVSimulationOutputOptions(QWidget *parent, VICUS::Outputs & outputs);
	~SVSimulationOutputOptions();

	/*! Updates user interface with properties from the project data structure.
		This function is called whenever the dialog is first shown.
	*/
	void updateUi();

private slots:
	void on_checkBoxDefaultZoneOutputs_toggled(bool checked);

private:
	Ui::SVSimulationOutputOptions		*m_ui;
	VICUS::Outputs						*m_outputs;
};

#endif // SVSimulationOutputOptionsH
