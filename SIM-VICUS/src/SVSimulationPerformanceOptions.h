#ifndef SVSimulationPerformanceOptionsH
#define SVSimulationPerformanceOptionsH

#include <QWidget>


namespace QtExt {
	class ValidatingLineEdit;
}

namespace NANDRAD {
	class SolverParameter;
}

namespace Ui {
	class SVSimulationPerformanceOptions;
}

/*! Edit widget for performance-related solver options.  */
class SVSimulationPerformanceOptions : public QWidget {
	Q_OBJECT
public:
	/*! Constructor, takes solver settings object reference (object is stored in main start dialog). */
	explicit SVSimulationPerformanceOptions(QWidget *parent, NANDRAD::SolverParameter & solverParams);
	~SVSimulationPerformanceOptions();

	/*! Updates user interface with properties from the project data structure.
		This function is called, whenever the dialog is first shown.
	*/
	void updateUi();

private slots:
	/*! Triggered whenever one of the combo boxes on this widget has changed index. */
	void currentIndexChanged(int index);

	void on_lineEditMaxOrder_editingFinishedSuccessfully();
	void on_lineEditNonLin_editingFinishedSuccessfully();
	void on_lineEditMaxKry_editingFinishedSuccessfully();
	void on_lineEditIterative_editingFinishedSuccessfully();
	void on_lineEditPreILU_editingFinishedSuccessfully();
	void on_lineEditInitialDT_editingFinishedSuccessfully();
	void on_lineEditMinDT_editingFinishedSuccessfully();

	void on_checkBoxDisableKinsolLineSearch_stateChanged(int arg1);

private:
	void parameterEditingFinished(int paraEnum, const QtExt::ValidatingLineEdit *edit);

	/*! UI pointer. */
	Ui::SVSimulationPerformanceOptions	*m_ui;

	/*! Data storage location, synchronized with user interface. */
	NANDRAD::SolverParameter			*m_solverParams = nullptr;

};

#endif // SVSimulationPerformanceOptionsH
