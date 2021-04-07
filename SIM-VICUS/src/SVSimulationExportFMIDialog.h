#ifndef SVSimulationExportFMIDialogH
#define SVSimulationExportFMIDialogH

#include <QDialog>

#include <VICUS_Project.h>

namespace Ui {
	class SVSimulationExportFMIDialog;
}

class QTableWidgetItem;
class QTableWidget;

/*! The start dialog for a NANDRAD simulation.
	Contains pages for all global simulation properties.
*/
class SVSimulationExportFMIDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVSimulationExportFMIDialog(QWidget *parent = nullptr);
	~SVSimulationExportFMIDialog();

	int edit();

	/*! Returns a copy of the locally modified version of the project. */
	const VICUS::Project & localProject() const { return m_localProject; }

private slots:
	void on_pushButtonClose_clicked();
	void on_pushButtonUpdateVariableList_clicked();


	void on_tableWidgetInputVars_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
	void on_tableWidgetOutputVars_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);


	void on_toolButtonAddInputVariable_clicked();
	void on_toolButtonRemoveOutputVariable_clicked();

	void on_toolButtonRemoveInputVariable_clicked();

private:
	struct IDInfo {
		std::vector<unsigned int>	m_objectIDs;
		std::vector<unsigned int>	m_vectorIndexes;
	};

	void updateVariableLists(bool silent);

	/*! Fills in the FMU data tables with already configured FMU variables.
		All FMI variables stored in m_localProject.m_fmiDescription are shown in the table,
		however, those without matching model variables (in m_modelInputVariables and m_modelOutputVariables)
		are shown in gray and italic (as invalid!).
	*/
	void updateFMUVariableTables();

	/*! Reads a variable definition file and generates a map with model variables versus object/vector IDs. */
	bool parseVariableList(const QString & varsFile, std::map<QString, IDInfo> & modelVariables, bool silent);

	/*! Adds a new row to a table widget.
		\param var The variable to add.
		\param tableWidget The target table widget.
		\param exists True, if such a model variable exists in the current model.
	*/
	void appendVariableEntry(unsigned int index, QTableWidget * tableWidget, bool exists);

	Ui::SVSimulationExportFMIDialog		*m_ui;

	VICUS::Project						m_localProject;

	std::map<QString, IDInfo>			m_modelInputVariables;
	std::map<QString, IDInfo>			m_modelOutputVariables;
};

#endif // SVSimulationExportFMIDialogH
