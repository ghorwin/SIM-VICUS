#ifndef SVCoSimCO2VentilationDialogH
#define SVCoSimCO2VentilationDialogH

#include <QDialog>

namespace Ui {
	class SVCoSimCO2VentilationDialog;
}

/*! Dialog to generate CO2-Balance FMU and Co-Sim project files. */
class SVCoSimCO2VentilationDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVCoSimCO2VentilationDialog(QWidget *parent = nullptr);
	~SVCoSimCO2VentilationDialog() override;

	int exec() override;

private:
	/*! This function checks the VICUS project data for valid and sufficient parametrization regarding CO2 balances. */
	bool checkProjectData() const;

	Ui::SVCoSimCO2VentilationDialog *m_ui;
};

#endif // SVCoSimCO2VentilationDialogH
