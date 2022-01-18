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

private slots:
	void on_pushButtonLaunchMasterSim_clicked();

	void on_pushButtonGenerate_clicked();

private:
	/*! This function checks the VICUS project data for valid and sufficient parametrization regarding CO2 balances. */
	bool checkProjectData() const;

	Ui::SVCoSimCO2VentilationDialog *m_ui;

	/*! Path to CO2-Balance FMU directory (wherein FMU files are collected) and where modelDescription.xml is being generated. */
	QString							m_co2FMUBaseDir;
	/*! Path to generated CO2-Balance FMU. */
	QString							m_co2FMUFilePath;
	/*! Path to generated NANDRAD FMU. */
	QString							m_nandradFMUFilePath;
	/*! Path to generated MSIM-Project file. */
	QString							m_msimProjectFilePath;
};

#endif // SVCoSimCO2VentilationDialogH
