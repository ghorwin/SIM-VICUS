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

	/*! Handles the initial file selection. */
	void init();

	/*! Sets the target FMU filename. */
	void setModelName(const QString & fname);

	/*! This is the work-horse function that does the entire generation stuff.
		Expects the project file to be saved already.
		\param silent If true, the generation process does not pop-up any message box or dialog in case of error, but
			just dumps errors to console. This is meant for command-line exporting functionality.
	*/
	int generate();

	/*! Full file path to the currently used nandrad file. */
	IBK::Path							m_nandradFilePath;
	/*! Path to dll/lib files. */
	QString								m_installDir;
	/*! Path to NANDRAD solver. */
	QString								m_nandradSolverExecutable;

	/*! Directory where fmu-file shall be placed into.
		By default the same directory as the .nandrad file, but can be changed by user.
	*/
	IBK::Path							m_fmuExportDirectory;

	/*! If true, all user-interactive dialogs are disabled and exporter runs as command-line-tool
		with just command line error messages.
	*/
	bool								m_silent = false;

public slots:
	/*! Reads the NANDRAD project and if successful, configures the user interface and calls updateVariableLists(). */
	int setup();

private slots:

	void on_tableWidgetInputVars_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
	void on_tableWidgetOutputVars_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

	void on_toolButtonAddInputVariable_clicked();
	void on_toolButtonRemoveInputVariable_clicked();
	void on_tableWidgetInputVars_itemDoubleClicked(QTableWidgetItem *item);

	void on_toolButtonAddOutputVariable_clicked();
	void on_toolButtonRemoveOutputVariable_clicked();
	void on_tableWidgetOutputVars_itemDoubleClicked(QTableWidgetItem *item);

	void on_pushButtonGenerate_clicked();

	void on_lineEditModelName_editingFinished();
	void on_lineEditTargetDirectory_editingFinished();
	void on_lineEditTargetDirectory_returnPressed();

	void on_pushButtonSaveNandradProject_clicked();
	void on_pushButtonSelectNandradProject_clicked();

private:
	/*! Toggles the GUI state depending on whether a valid NANDRAD Project was read or not. */
	void setGUIState(bool active);

	/*! Checks if the model name and/or the path is conforming to the requirements.
		- no umlaute/special characters in model name
		- no spaces
		- no leading numbers
		- target path must exist

		Shows a critical message box if any error occurred.
	*/
	bool checkModelName();

	/*! Runs the currently loaded NANDRAD project in test-init mode and parses the
		input and output references files, then updates m_availableInputVariables and
		m_availableOutputVariables.
	*/
	void updateVariableLists();

	/*! Reads a variable definition file and generates a map with model variables versus object/vector IDs. */
	bool parseVariableList(const QString & varsFile, std::vector<NANDRAD::FMIVariableDefinition> & modelVariables);

	/*! Fills in the FMU data tables with already configured FMU variables.
		All FMI variables stored in m_localProject.m_fmiDescription are shown in the table,
		however, those without matching model variables (in m_modelInputVariables and m_modelOutputVariables)
		are shown in gray and italic (as invalid!).
	*/
	void updateFMUVariableTables();

	/*! This fills in a table with FMI variables. */
	void populateTable(QTableWidget * table, const std::vector<NANDRAD::FMIVariableDefinition> & availableVars,
					   const std::vector<NANDRAD::FMIVariableDefinition> & invalidVars);

	/*! Adds a new row to a table widget.
		\param tableWidget The target table widget.
		\param var The variable to add.
		\param valid True, if such a model variable exists in the current model (invalid variables are marked in red).
	*/
	void appendVariableEntry(QTableWidget * tableWidget, const NANDRAD::FMIVariableDefinition & var, bool valid);


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

	/*! The project, contains the currently defined FMI input/output variables. */
	NANDRAD::Project					m_project;

	/*! This set contains a list of all value references currently used by variables
		in the NANDRAD Project (i.e. configured variables).
	*/
	std::set<unsigned int>							m_usedValueRefs;

	/*! Holds all _available_ input variable definitions. */
	std::vector<NANDRAD::FMIVariableDefinition>		m_availableInputVariables;
	/*! Holds all _available_ output variable definitions. */
	std::vector<NANDRAD::FMIVariableDefinition>		m_availableOutputVariables;

};

#endif // NandradFMUGeneratorWidgetH
