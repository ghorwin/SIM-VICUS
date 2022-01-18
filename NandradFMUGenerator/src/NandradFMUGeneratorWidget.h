#ifndef NandradFMUGeneratorWidgetH
#define NandradFMUGeneratorWidgetH

#include <QWidget>

#include <NANDRAD_Project.h>
#include "FMUVariableTableModel.h"

namespace Ui {
	class NandradFMUGeneratorWidget;
}

class QSortFilterProxyModel;
class QItemSelection;

/*! The dialog for configuring and exporting NANDRAD FMUs.
*/
class NandradFMUGeneratorWidget : public QWidget {
	Q_OBJECT

public:
	explicit NandradFMUGeneratorWidget(QWidget *parent = nullptr);
	~NandradFMUGeneratorWidget();

	/*! Handles the initial file selection. */
	void init();

	/*! Performs a renaming of an input variable.
		This function is called by the table model in setData().
		If all error checks are passed, the data storage vectors (used by the models) are modified accordingly and the function
		returns true. Otherwise the function returns false and no changes are made to the data.
		\note newVarName is passed by value since it may be modified in the function.
	*/
	bool renameInputVariable(unsigned int index, QString newVarName, bool autoAdjustName = false);

	/*! Performs a renaming of an output variable.
		This function is called by the table model in setData().
		If all error checks are passed, the data storage vectors (used by the models) are modified accordingly and the function
		returns true. Otherwise the function returns false and no changes are made to the data.
	*/
	bool renameOutputVariable(unsigned int index, const QString &newVarName, bool autoAdjustName = false);

	/*! This is the work-horse function that does the entire generation stuff.
		Expects the project file to be saved already.
		\param silent If true, the generation process does not pop-up any message box or dialog in case of error, but
			just dumps errors to console. This is meant for command-line exporting functionality.
	*/
	bool generate();

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
	/*! Here the model name is stored when exported is run in automatic-mode. */
	QString								m_autoExportModelName;

private slots:
	/*! Reads the NANDRAD project and if successful, configures the user interface and calls updateVariableLists(). */
	void setup();

	void on_pushButtonGenerate_clicked();

	void on_lineEditModelName_editingFinished();
	void on_lineEditTargetDirectory_editingFinished();
	void on_lineEditTargetDirectory_returnPressed();

	void on_pushButtonSaveNandradProject_clicked();
	void on_pushButtonSelectNandradProject_clicked();

	/*! Refreshes the project data when smth inside the project file changes */
	void on_pushButtonRefresh_clicked();

	/*! Called when solver process was started successfully. */
	void onProcessStarted();
	/*! Called when solver process could not be started. */
	void onProcessErrorOccurred();


	// ** Input Variable Table **

	void on_tableViewInputVars_doubleClicked(const QModelIndex &index);
	void on_toolButtonAddInputVariable_clicked();
	void on_toolButtonRemoveInputVariable_clicked();
	void onInputVarsSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);


	// ** Output Variable Table **

	void on_tableViewOutputVars_doubleClicked(const QModelIndex &index);
	void on_toolButtonAddOutputVariable_clicked();
	void on_toolButtonRemoveOutputVariable_clicked();
	void onOutputVarsSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

	void on_lineEditInputVarNameFilter_textEdited(const QString &arg1);

	void on_lineEditInputVarDescFilter_textEdited(const QString &arg1);

	void on_lineEditOutputVarNameFilter_textEdited(const QString &arg1);

	void on_lineEditOutputVarDescFilter_textEdited(const QString &arg1);

	void on_checkBoxUseDisplayNames_clicked(bool checked);

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

	void dumpUsedValueRefs() const;

	/*! Add a new variable as input/output FMI variable and assign unique value reference. */
	void addVariable(bool inputVar);

	/*! Removes an already configured FMI variable. */
	void removeVariable(bool inputVar);

	/*! Updates FMI configuration in project. */
	void storeFMIVariables(NANDRAD::Project & prj);

	/*! Helper function for removing value reference of current input variable (if and only
		if not used by any other input variable). */
	void removeUsedInputValueRef(unsigned int index, unsigned int fmiVarRef);

	/*! Generates a new variable name based on the suggestedName that is unique among all available input/output variables. */
	QString generateUniqueVariableName(const QString & suggestedName) const;

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



	Ui::NandradFMUGeneratorWidget					*m_ui;

	/*! The project, contains the currently defined FMI input/output variables. */
	NANDRAD::Project								m_project;

	/*! This set contains a list of all value references currently used by variables
		in the NANDRAD Project (i.e. configured variables).
	*/
	std::set<unsigned int>							m_usedValueRefs;

	/*! Holds all _available_ input variable definitions. */
	std::vector<NANDRAD::FMIVariableDefinition>		m_availableInputVariables;
	/*! Holds all _available_ output variable definitions. */
	std::vector<NANDRAD::FMIVariableDefinition>		m_availableOutputVariables;

	/*! Table model instance for input vars. */
	FMUVariableTableModel							*m_inputVariablesTableModel = nullptr;
	/*! Table model instance for output vars. */
	FMUVariableTableModel							*m_outputVariablesTableModel = nullptr;

	QSortFilterProxyModel							*m_inputVariablesProxyModel = nullptr;
	QSortFilterProxyModel							*m_outputVariablesProxyModel = nullptr;

	/*! List of known display names for different model objects. */
	std::vector<FMUVariableTableModel::DisplayNameSubstitution>		m_displayNameTable;

	std::map<std::string, std::pair<std::string, std::string> > m_variableInfoMap;
};

#endif // NandradFMUGeneratorWidgetH
