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
	Ui::SVSimulationLCAResultsDialog *m_ui;
};

#endif // SVSimulationLCAResultsDialogH
