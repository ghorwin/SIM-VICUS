#ifndef SVSimulationLCAResultsDialogH
#define SVSimulationLCAResultsDialogH
#include <QDialog>

namespace Ui {
class SVSimulationLCAResultsDialog;
}

class SVSimulationLCAResultsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVSimulationLCAResultsDialog(QWidget *parent = nullptr);
	~SVSimulationLCAResultsDialog();

	void setLcaResults();


private:

	/*! Sets up Dialog with LCA Data. Needs to be called once before Dialog is beeing used. */
	void setup();

	/*! Pointer to UI. */
	Ui::SVSimulationLCAResultsDialog *m_ui;
};

#endif // SVSimulationLCAResultsDialogH
