#ifndef SVCoSimCO2VentilationDialogH
#define SVCoSimCO2VentilationDialogH

#include <QDialog>

namespace Ui {
	class SVCoSimCO2VentilationDialog;
}

namespace VICUS {
	class Room;
	class Schedule;
}

namespace NANDRAD {
	class FMIVariableDefinition;
	class Project;
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
	struct CO2ComfortVentilationProject ;

	/*! This function checks the VICUS project data for valid and sufficient parametrization regarding CO2 balances. */
	bool checkProjectData() const;
	/*! This generates NANDRAD FMU. */
	void generateNandradFMU(NANDRAD::Project project, const QString &modelName, const QString &tmpDir, const QString &fmuTargetFile) const;
	/*! This generates MasterSim file. */
	void generateMasterSimFile(const CO2ComfortVentilationProject &project, const QString &targetDir, const QString &modelName) const;
	/*! This generates a binaries directories for FMUs. */
	void generateBinaries(const QString & sourceDir, const QString &targetDir, const QString &sourceName, const QString &targetName,
						  const QString & version = QString()) const;
	/*! This generates a mdoel description file for given inpt and output parameters. */
	void generateModelDescription(const QString &targetDir, const QString &modelName,
								  const QString &modelId, const QString &modelDescription, const QString &version,
								  const std::vector<NANDRAD::FMIVariableDefinition> &inputVars,
								  const std::vector<NANDRAD::FMIVariableDefinition> &outputVars) const;

	Ui::SVCoSimCO2VentilationDialog *m_ui;

	/*! Path to CO2-Balance FMU directory (wherein FMU files are collected) and where modelDescription.xml is being generated. */
	QString							m_co2FMUBaseDir;
	/*! Path to generated CO2-Balance FMU. */
	QString							m_co2FMUFilePath;
	/*! Path to NANDRAD FMU directory (wherein FMU files are collected) and where modelDescription.xml is being generated. */
	QString							m_nandradFMUBaseDir;
	/*! Path to generated NANDRAD FMU. */
	QString							m_nandradFMUFilePath;
	/*! Path to generated MSIM-Project file. */
	QString							m_msimProjectFilePath;
};

#endif // SVCoSimCO2VentilationDialogH
