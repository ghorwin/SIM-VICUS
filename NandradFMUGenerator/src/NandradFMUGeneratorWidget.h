#ifndef NandradFMUGeneratorWidgetH
#define NandradFMUGeneratorWidgetH

#include <QWidget>

#include <NANDRAD_Project.h>

namespace Ui {
	class NandradFMUGeneratorWidget;
}

class QTableWidgetItem;
class QTableWidget;

/*! The dialog for configuring and exporting NANDRAD FMUs.
*/
class NandradFMUGeneratorWidget : public QWidget {
	Q_OBJECT

public:
	explicit NandradFMUGeneratorWidget(QWidget *parent = nullptr);
	~NandradFMUGeneratorWidget();

	void setup();

	IBK::Path							m_nandradFilePath;
	/*! Path to dll/lib files. */
	QString								m_installDir;
	/*! Path to NANDRAD solver. */
	QString								m_nandradSolverExecutable;

private slots:
	void on_pushButtonUpdateVariableList_clicked();

	void on_tableWidgetInputVars_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
	void on_tableWidgetOutputVars_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

	void on_toolButtonAddInputVariable_clicked();
	void on_toolButtonRemoveOutputVariable_clicked();

	void on_toolButtonRemoveInputVariable_clicked();

	void on_pushButtonGenerateAllVariables_clicked();

	void on_pushButtonGenerate_clicked();

	void on_lineEditModelName_editingFinished();
	void on_lineEditTargetPath_editingFinished();
	void on_lineEditTargetPath_returnPressed();

	void on_pushButtonSaveNandradProject_clicked();
	void on_pushButtonSelectNandradProject_clicked();

private:
	/*! Checks if the model name and/or the path is conforming to the requirements.
		- no umlaute/special characters in model name
		- no spaces
		- no leading numbers
		- target path must exist

		Shows a critical message box if any error occurred.
	*/
	bool checkModelName();

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

	/*! This function returns detailed variable information to be used when generating FMU variables.
		This might be better placed somewhere in the VICUS library?
		The variables are defined in the NandradSolver sources, some with keyword list support, some without.
		So we generally have no translated complete list of variables and associated object reference types anywhere
		and need to duplicate that information somewhere within a Qt-based library.
		This also means that when new variables are added in the NANDRAD solver, we need to add these
		variables also here in the Qt library.

		For now, we only need these variables in the FMU interface (maybe also for outputs?), so let's just implement this
		function here. We can move this static function elsewhere, later.
	*/
	static void variableInfo(const std::string & fullVarName, QString & description, std::string & unit, std::string & fmuType);

	Ui::NandradFMUGeneratorWidget		*m_ui;

	NANDRAD::Project					m_project;


	std::map<QString, IDInfo>			m_modelInputVariables;
	std::map<QString, IDInfo>			m_modelOutputVariables;
};

#endif // NandradFMUGeneratorWidgetH
