#ifndef SVSimulationExportFMIH
#define SVSimulationExportFMIH

#include <QDialog>

#include <NANDRAD_Project.h>
#include <VICUS_Outputs.h>

namespace Ui {
	class SVSimulationExportFMIDialog;
}


/*! The start dialog for a NANDRAD simulation.
	Contains pages for all global simulation properties.
*/
class SVSimulationExportFMIDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVSimulationExportFMIDialog(QWidget *parent = nullptr);
	~SVSimulationExportFMIDialog();

	int edit();

private slots:
	void on_pushButtonClose_clicked();

private:
	/*! Stores current input into project data structure. */
	void storeInput();

	Ui::SVSimulationExportFMIDialog	*m_ui;

};

#endif // SVSimulationExportFMIH
