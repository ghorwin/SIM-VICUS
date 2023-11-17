/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVMainWindowH
#define SVMainWindowH

#include <QMainWindow>
#include <QUndoStack>
#include <QProcess>
#include <QTimer>

#include <map>

#include "SVProjectHandler.h"

namespace Ui {
	class SVMainWindow;
}

class QProgressDialog;
class QSplitter;

class SVThreadBase;
class SVWelcomeScreen;
class SVNotesDialog;
class SVButtonBar;
class SVPreferencesDialog;
class SVPostProcHandler;
class SVLogWidget;
class SVGeometryView;
class SVNavigationTreeWidget;
class SVNetworkImportDialog;
class SVNetworkExportDialog;
class SVImportIDFDialog;
class SVImportDXFDialog;
class SVNetworkEditDialog;
class SVViewStateHandler;
class SVSimulationStartNandrad;
class SVSimulationStartNetworkSim;
class SVSimulationShadingOptions;
class SVCoSimCO2VentilationDialog;
class SVLcaLccSettingsWidget;

class SVDatabaseEditDialog;
class SVDBZoneTemplateEditDialog;
class SVDBDuplicatesDialog;
class SVPluginLoader;
class SVSimulationSettingsView;


/*! Main window class. */
class SVMainWindow : public QMainWindow {
	Q_OBJECT
public:


	enum MainViewMode {
		/*! None of the main views is shown, the welcome screen should then be present */
		MV_None,
		/*! 3d scene geometry view */
		MV_GeometryView,
		/*! Simulation settings and simulation start view */
		MV_SimulationView
	};

	/*! Returns a pointer to the SVMainWindow instance.
		Only access this function during the lifetime of the
		SVMainWindow instance.
	*/
	static SVMainWindow & instance();

	/*! Adds an undo command to the global undo stack.
		Ownership of the command object will be transferred to the stack.
	*/
	static void addUndoCommand(QUndoCommand * command);



	/*! Default SVMainWindow constructor. */
	explicit SVMainWindow(QWidget *parent = nullptr);

	/*! Default destructor. */
	~SVMainWindow() override;

	/*! Public access function to save project file (called from simulation view).
		\return Returns true if project was saved and project handler
				has now an unmodified project with valid project filename.
	*/
	bool saveProject();

	/*! Exports the current project to the selected exportFilePath path.
		Opens an (internationalized) message box on the first error encountered.
		\param exportFilePath Path to resultant project package
		\param withTopLevelDir If true, the files are stored within a top-level directory.
		\return Returns true on success, false on error.

		The following conventions on the directories within the zip-file are used:
		Suppose the project lies at: ../data/test.vicus
		Suppose exportFilePath = ../data/exports/testPackage.vicus

		When withTopLevelDir==false, Zip content:

		  db/
		  climate/
		  test.vicus

		When withTopLevelDir==true, Zip content:

		  test_package/
			db/
			climate/
			test.vicus

	*/
	bool exportProjectPackage(const QString & exportFilePath, bool withTopLevelDir);

	/*! Returns the material edit dialog. */
	SVDatabaseEditDialog * dbMaterialEditDialog();
	/*! Returns the EPD edit dialog. */
	SVDatabaseEditDialog * dbEpdEditDialog();
	/*! Returns the construction edit dialog. */
	SVDatabaseEditDialog * dbConstructionEditDialog();
	/*! Returns the component edit dialog. */
	SVDatabaseEditDialog * dbComponentEditDialog();
	/*! Returns the subsurface component edit dialog. */
	SVDatabaseEditDialog * dbSubSurfaceComponentEditDialog();
	/*! Returns the boundary condition edit dialog. */
	SVDatabaseEditDialog * dbBoundaryConditionEditDialog();
	/*! Returns the window edit dialog. */
	SVDatabaseEditDialog * dbWindowEditDialog();
	/*! Returns the window glazing system edit dialog. */
	SVDatabaseEditDialog * dbWindowGlazingSystemEditDialog();
	/*! Returns the pipe edit dialog. */
	SVDatabaseEditDialog *dbPipeEditDialog();
	/*! Returns the supply system condition edit dialog. */
	SVDatabaseEditDialog * dbSupplySystemEditDialog();
	/*! Returns the network component edit dialog. */
	SVDatabaseEditDialog *dbNetworkComponentEditDialog();
	/*! Returns the schedule edit dialog. */
	SVDatabaseEditDialog *dbScheduleEditDialog();
	/*! Returns the internal loads person edit dialog. */
	SVDatabaseEditDialog *dbInternalLoadsPersonEditDialog();
	/*! Returns the internal loads electric equipment edit dialog. */
	SVDatabaseEditDialog *dbInternalLoadsElectricEquipmentEditDialog();
	/*! Returns the internal loads person edit dialog. */
	SVDatabaseEditDialog *dbInternalLoadsLightsEditDialog();
	/*! Returns the internal loads other edit dialog. */
	SVDatabaseEditDialog *dbInternalLoadsOtherEditDialog();
	/*! Returns the internal zone control thermostat edit dialog. */
	SVDatabaseEditDialog *dbZoneControlThermostatEditDialog();
	/*! Returns the internal zone control natural ventilation edit dialog. */
	SVDatabaseEditDialog *dbZoneControlVentilationNaturalEditDialog();
	/*! Returns the internal zone control shading edit dialog. */
	SVDatabaseEditDialog *dbZoneControlShadingEditDialog();
	/*! Returns the internal infiltration edit dialog. */
	SVDatabaseEditDialog *dbZoneIdealHeatingCoolingEditDialog();
	/*! Returns the internal infiltration edit dialog. */
	SVDatabaseEditDialog *dbInfiltrationEditDialog();
	/*! Returns the internal ventilation edit dialog. */
	SVDatabaseEditDialog *dbVentilationNaturalEditDialog();
	/*! Returns the zone template edit dialog. */
	SVDBZoneTemplateEditDialog * dbZoneTemplateEditDialog();
	/*! Returns the pipe edit dialog. */
	SVDatabaseEditDialog *dbFluidEditDialog();
	/*! Returns the network controller edit dialog. */
	SVDatabaseEditDialog *dbNetworkControllerEditDialog();
	/*! Returns the sub network edit dialog. */
	SVDatabaseEditDialog *dbSubNetworkEditDialog();
	/*! Returns the surface heating system edit dialog. */
	SVDatabaseEditDialog *dbSurfaceHeatingSystemEditDialog();

	/*! Returns the pointer to the Start Simulation Nandrad Widget */
	SVSimulationStartNandrad * simulationStartNandrad() const;

	/*! Returns pointer to the applications preferences dialog. */
	SVPreferencesDialog * preferencesDialog();

public slots:

	void on_actionDBComponents_triggered();
	void on_actionDBSubSurfaceComponents_triggered();

protected:
	/*! Checks if project file has been changed by external application. */
	void changeEvent(QEvent *event) override;
	/*! Does the confirm-saving-before-close stuff. */
	void closeEvent(QCloseEvent * event) override;
	/*! Called when the window is moved. Repositions measurement widget. */
	void moveEvent(QMoveEvent *event) override;

signals:

	void screenHasChanged(const QScreen *screen);


private slots:
	/*! Does the entire UI initialization.
		Triggered with slight delay from the event loop. Make sure no other events kick in before setup has
		completed!
	*/
	void setup();

	/*! Triggered when a recent file menu entry was clicked. */
	void onActionOpenRecentFile();

	/*! Triggered when a language menu entry was clicked. */
	void onActionSwitchLanguage();

	/*! Updates the state of all actions based on the current condition of the project.
		This slot is connected to the signal updateActions() from SVProjectHandler.
	*/
	void onUpdateActions();

	/*! Updates the menu entries in the 'Recent Projects' submenu.
		This is a slot because we need to update the menu with the actions
		a bit delayed. When the user clicks on a recent project menu entry, the
		loadProject() function is indirectly called which in turn calls
		updateRecentProjects(). Since the menu actions in the recent projects
		menu are deleted, this would mean that the action currently process is
		being deleted - causing a crash. Therefore we don't call updateRecentProjects()
		directly, but via a QTimer::singleShot() and thus ensure that the
		action handler function is completed before the action is touched.
	*/
	void onUpdateRecentProjects();

	/*! Opens a project with filename.
		Called from onActionOpenRecentFile() and from welcome screen.
	*/
	void onOpenProjectByFilename(const QString & filename);

	/*! Copies an example project (and its directory content) to a new directory (interactive) and
		opens the copied example project.
		Called only from welcome screen.
	*/
	void onOpenExampleByFilename(const QString & filename);

	/*! Copies an template project (and its directory content) to a new directory (interactive) and
		opens the copied template project.
		Called only from welcome screen.
	*/
	void onOpenTemplateByFilename(const QString & filename);

	/*! Triggered whenever a worker thread finishes. */
	void onWorkerThreadFinished();

	/*! Triggered whenever a project was read successfully. */
	void onFixProjectAfterRead();

	/*! Connected to SVPreferencesPageMisc */
	void onAutosaveSettingsChanged();

	void onDockWidgetToggled(bool);

	/*! Triggered, when an import plugin menu action was triggered. */
	void onImportPluginTriggered();
	/*! Triggered, when the menu action for configuring a plugin was triggered. */
	void onConfigurePluginTriggered();

	/*! Updates the device pixel ratio. */
	void onScreenChanged(QScreen *screen);


	// all menu action slots below

	void on_actionFileNew_triggered();
	void on_actionFileOpen_triggered();
	void on_actionFileSave_triggered();
	void on_actionFileSaveAs_triggered();
	void on_actionFileReload_triggered();
	void on_actionFileImportEneryPlusIDF_triggered();
	void on_actionFileOpenProjectDir_triggered();
	void on_actionFileClose_triggered();
	void on_actionFileExportProjectPackage_triggered();
	void on_actionFileQuit_triggered();

	void on_actionEditTextEditProject_triggered();
	void on_actionEditPreferences_triggered();
	void on_actionEditCleanProject_triggered();
	void on_actionEditApplicationLog_triggered();

	void on_actionDBMaterials_triggered();
	void on_actionDBWindows_triggered();
	void on_actionDBWindowGlazingSystems_triggered();
	void on_actionDBConstructions_triggered();

	void on_actionDBBoundaryConditions_triggered();
	void on_actionDBSchedules_triggered();
	void on_actionDBInternalLoadsPerson_triggered();
	void on_actionDBInternalLoadsElectricEquipment_triggered();
	void on_actionDBInternalLoadsLights_triggered();
	void on_actionDBInternalLoadsOther_triggered();

	void on_actionDBZoneTemplates_triggered();
	void on_actionDBZoneControlThermostat_triggered();
	void on_actionDBZoneControlVentilationNatural_triggered();
	void on_actionDBZoneControlShading_triggered();

	void on_actionDBZoneIdealHeatingCooling_triggered();

	void on_actionDBInfiltration_triggered();
	void on_actionDBVentilationNatural_triggered();

	void on_actionDBSurfaceHeatingSystems_triggered();

	void on_actionDBHydraulicComponents_triggered();
	void on_actionDBNetworkPipes_triggered();
	void on_actionDBFluids_triggered();
	void on_actionDBControllers_triggered();
	void on_actionDBSubNetworks_triggered();
	void on_actionDBSupplySystems_triggered();
	void on_actionDBRemoveDuplicates_triggered();


	void on_actionBuildingFloorManager_triggered();
	void on_actionBuildingSurfaceHeatings_triggered();

	void on_actionToolsCCMeditor_triggered();

	void on_actionViewShowSurfaceNormals_toggled(bool visible);
	void on_actionViewShowGrid_toggled(bool visible);
	void on_actionViewFindSelectedGeometry_triggered();
	void on_actionViewResetView_triggered();
	void on_actionViewFromNorth_triggered();
	void on_actionViewFromEast_triggered();
	void on_actionViewFromSouth_triggered();
	void on_actionViewFromWest_triggered();
	void on_actionViewFromAbove_triggered();
	void on_actionViewBirdsEyeViewNorthWest_triggered();
	void on_actionViewBirdsEyeViewNorthEast_triggered();
	void on_actionViewBirdsEyeViewSouthWest_triggered();
	void on_actionViewBirdsEyeViewSouthEast_triggered();

	void on_actionSimulationCO2Balance_triggered();

	void on_actionHelpAboutQt_triggered();
	void on_actionHelpAbout_triggered();
	void on_actionHelpBugReport_triggered();
	void on_actionHelpVisitDiscussionForum_triggered();
	void on_actionHelpCheckForUpdates_triggered();
	void on_actionHelpOnlineManual_triggered();
	void on_actionHelpKeyboardAndMouseControls_triggered();
	void on_actionHelpLinuxDesktopIntegration_triggered();

	void on_actionFileImportNetworkGISData_triggered();
	void on_actionEditProjectNotes_triggered();
	void on_actionPluginsManager_triggered();

	void on_actionExportNetworkAsGeoJSON_triggered();


	void on_actionGeometryView_triggered();

	void on_actionSimulationSettings_triggered();

	void on_actionOpenPostProcessing_triggered();

	void onShortCutStartSimulation();

	void on_actionEPDElements_triggered();

	void on_actionExternal_Post_Processor_triggered();

	void on_actionDWD_Weather_Data_Converter_triggered();

private:

	void updateMainView();

	/*! Sets up all dock widgets with definition lists. */
	void setupDockWidgets();

	/*! Loads plugins and incorporates these in the main menu.
		In case of Database plugins, also populates the database with additional data sets.
	*/
	void setupPlugins();

	/*! Populates the main menu entries for given plugins. */
	void setupPluginMenuEntries(QObject * plugin);

	/*! Updates the window title. */
	void updateWindowTitle();

	/*! Imports the project package to the selected import directory.
		Opens an (internationalized) message box on the first error encountered.
		\param packageFilePath Full path to package file.
		\param targetDirectory Target dir to extract package content into (no subdirectory is created)
		\param projectFilePath If package contains a project file, this file name (full path) will be stored
		\param isPackage True if package is a vicus package and should contain a vicus project file.
		\return Returns true on success, false on error.
	*/
	bool importProjectPackage(const QString & packageFilePath, const QString & targetDirectory,
							  QString & projectFilePath, bool isPackage);

	/*! Creates a thumbnail-image of the current project sketch.
		\return Returns full path to created thumbnail.
	*/
	QString saveThumbNail();

	/*! Adds another language setting action, when the corresponding language files exist. */
	void addLanguageAction(const QString & langId, const QString & actionCaption);

	/*! Appends the thread base to the thread pool and starts processing the threads.
		This function is usually called from the static function SVThreadBase::runWorkerThread().
	*/
	void runWorkerThreadImpl(SVThreadBase * base);

	/*! This will start the processing of the thread pool.
		If a thread is currently running, the call is ignored. */
	void processThread();

	/*! Tests of file is a project package, and if yes, asks user to select an extraction
		directory, imports the project and adjusts filename to point to the extracted
		project file.
		In case of error, an error message pops up (this is an interactive function!) and
		the function returns false. If all goes well, the function returns true.
		\param filename Full path to package file (if file has invalid extension, function silently returns).
		\param renameProjectFileAfterwards If set to true, the user is requested to specify target filename instead of
			just the extraction directory. After project is extracted it will then be renamed to match the selected file.
	*/
	bool processProjectPackage(QString & filename, bool renameProjectFileAfterwards);

	/*! This function does the actual copying of the project dependencies to the 'materials' and 'climate' subdirectories.
		\param targetDirPath Directory to export the project to (must exist and be empty).
		\param project The project to export.
	*/
	bool exportProjectCopy(QString targetDirPath, const VICUS::Project & project);


	/*! Global pointer to main window instance.
		Initialized in the constructor of SVMainWindow and
		reset to nullptr in the destructor. So be sure that the main window
		exists before accessing SVMainWindow::instance()
	*/
	static SVMainWindow			*m_self;

	/*! Map to store dock widget visibility.
		Initialized, after stored view configuration was read from file.
		Modified and applied in onNavigationBarViewChanged();
	*/
	std::map<QDockWidget*, bool>	m_dockWidgetVisibility;

	/*! Stores the current main view mode (Geometry, Simulation Start, ...) */
	MainViewMode					m_mainViewMode										= MV_GeometryView;

	/*! Main user interface pointer. */
	Ui::SVMainWindow			*m_ui													= nullptr;
	/*! The global undo stack in the program. */
	QUndoStack					*m_undoStack											= nullptr;
	/*! Menu for the recent projects entries. */
	QMenu						*m_recentProjectsMenu									= nullptr;
	/*! List with action objects for each recent project in the main menu. */
	QList<QAction*>				m_recentProjectActions;
	/*! List of language actions shown in the languages menu. */
	QList<QAction*>				m_languageActions;
	/*! The project handler that manages the actual project. */
	SVProjectHandler			m_projectHandler;
	/*! The thread pool managed by the main window. */
	QList<SVThreadBase*>		m_threadPool;
	/*! The plugin-loader. */
	SVPluginLoader				*m_pluginLoader;

	QAction *					m_undoAction;
	QAction *					m_redoAction;

	/*! The welcome screen. */
	SVWelcomeScreen				*m_welcomeScreen										= nullptr;

	/*! The navigation bar. */
	SVButtonBar					*m_buttonBar											= nullptr;

	/*! The Notes Dialog. */
	SVNotesDialog				*m_notesDialog											= nullptr;

	/*! Splitter that contains navigation tree widget and geometry view. */
	QSplitter					*m_geometryViewSplitter									= nullptr;

	/*! View with simulation settings, climate data, ouptuts and so on */
	SVSimulationSettingsView	*m_simulationSettingsView								= nullptr;

	/*! Navigation tree widget (left of 3D scene view). */
	SVNavigationTreeWidget		*m_navigationTreeWidget									= nullptr;

	/*! User preferences. */
	SVPreferencesDialog			*m_preferencesDialog									= nullptr;

	/*! IDF import dialog */
	SVImportIDFDialog			*m_importIDFDialog										= nullptr;

	/*! Network import dialog */
	SVNetworkImportDialog		*m_networkImportDialog									= nullptr;

	/*! Network export dialog */
	SVNetworkExportDialog		*m_networkExportDialog									= nullptr;


	/*! Contains the 3D scene view (and tool buttons and stuff). */
	SVGeometryView				*m_geometryView											= nullptr;

	QDockWidget					*m_logDockWidget										= nullptr;
	SVLogWidget					*m_logWidget											= nullptr;

	/*! Handles spawning/activating of PostProc executable. */
	SVPostProcHandler			*m_postProcHandler										= nullptr;

	/*! Central handler for the user interface state. */
	SVViewStateHandler					*m_viewStateHandler								= nullptr;

	SVDatabaseEditDialog				*m_dbMaterialEditDialog							= nullptr;
	SVDatabaseEditDialog				*m_dbEpdEditDialog								= nullptr;
	SVDatabaseEditDialog				*m_dbConstructionEditDialog						= nullptr;
	SVDatabaseEditDialog				*m_dbWindowEditDialog							= nullptr;
	SVDatabaseEditDialog				*m_dbWindowGlazingSystemEditDialog				= nullptr;
	SVDatabaseEditDialog				*m_dbComponentEditDialog						= nullptr;
	SVDatabaseEditDialog				*m_dbSubSurfaceComponentEditDialog				= nullptr;
	SVDatabaseEditDialog				*m_dbBoundaryConditionEditDialog				= nullptr;
	SVDatabaseEditDialog				*m_dbPipeEditDialog								= nullptr;
	SVDatabaseEditDialog				*m_dbFluidEditDialog							= nullptr;
	SVDatabaseEditDialog				*m_dbNetworkControllerEditDialog				= nullptr;
	SVDatabaseEditDialog				*m_dbSubNetworkEditDialog						= nullptr;
	SVDatabaseEditDialog				*m_dbSupplySystemEditDialog						= nullptr;
	SVDatabaseEditDialog				*m_dbNetworkComponentEditDialog					= nullptr;
	SVDatabaseEditDialog				*m_dbScheduleEditDialog							= nullptr;
	SVDatabaseEditDialog				*m_dbInternalLoadsPersonEditDialog				= nullptr;
	SVDatabaseEditDialog				*m_dbInternalLoadsElectricEquipmentEditDialog	= nullptr;
	SVDatabaseEditDialog				*m_dbInternalLoadsLightsEditDialog				= nullptr;
	SVDatabaseEditDialog				*m_dbInternalLoadsOtherEditDialog				= nullptr;
	SVDatabaseEditDialog				*m_dbZoneControlThermostatEditDialog			= nullptr;
	SVDatabaseEditDialog				*m_dbZoneControlVentilationNaturalEditDialog	= nullptr;
	SVDatabaseEditDialog				*m_dbZoneControlShadingEditDialog				= nullptr;
	SVDatabaseEditDialog				*m_dbZoneIdealHeatingCoolingEditDialog			= nullptr;
	SVDatabaseEditDialog				*m_dbInfiltrationEditDialog						= nullptr;
	SVDatabaseEditDialog				*m_dbVentilationNaturalEditDialog				= nullptr;
	SVDatabaseEditDialog				*m_dbVSurfaceHeatingSystemEditDialog			= nullptr;

	// special edit dialogs
	SVDBZoneTemplateEditDialog			*m_dbZoneTemplateEditDialog						= nullptr;

	SVDBDuplicatesDialog				*m_dbDuplicatesDialog							= nullptr;

	SVCoSimCO2VentilationDialog			*m_coSimCO2VentilationDialog					= nullptr;

	/*! Timer for auto-save periods. */
	QTimer								*m_autoSaveTimer 								= nullptr;

	SVLcaLccSettingsWidget				*m_lcaLccSettingsDialog							= nullptr;

	friend class SVThreadBase;
};

#endif // SVMainWindowH
