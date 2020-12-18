#include "SVMainWindow.h"
#include "ui_SVMainWindow.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QProcess> // for starting the external editor
#include <QFileInfo>
#include <QUndoStack>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QDockWidget>
#include <QSettings>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QToolButton>
#include <QCheckBox>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QFileInfo>
#include <QInputDialog>
#include <QSplitter>
#include <QTextEdit>

#include <numeric>

#include <IBK_FileUtils.h>
#include <IBK_messages.h>

#include <VICUS_Project.h>
#include <VICUS_Constants.h>

#include <JlCompress.h> // zlib support

#include <QtExt_AutoUpdater.h>
#include <QtExt_Directories.h>

#include "SVMessageHandler.h"
#include "SVConstants.h"
#include "SVSettings.h"
#include "SVWelcomeScreen.h"
#include "SVLogWidget.h"
#include "SVThreadBase.h"
#include "SVPreferencesDialog.h"
#include "SVPostProcBindings.h"
#include "SVAboutDialog.h"
#include "SVPostProcHandler.h"
#include "SVNavigationTreeWidget.h"
//#include "SVFMIExportDialog.h"
#include "SVNetworkImportDialog.h"
#include "SVNetworkEditDialog.h"
#include "SVPreferencesPageStyle.h"
#include "SVViewStateHandler.h"
#include "SVImportIDFDialog.h"

#include "SVDBMaterialsEditWidget.h"
#include "SVDBWindowEditWidget.h"
#include "SVDBConstructionEditDialog.h"

#include "SVSimulationStartNandrad.h"
#include "SVSimulationStartNetworkSim.h"

#include "SVGeometryView.h"
#include "Vic3DSceneView.h"


#include "SVUndoProject.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoAddBuilding.h"


static bool copyRecursively(const QString &srcFilePath, const QString &tgtFilePath);

SVMainWindow * SVMainWindow::m_self = nullptr;

SVMainWindow & SVMainWindow::instance() {
	Q_ASSERT_X(m_self != nullptr, "[SVMainWindow::instance]",
		"You must not access SVMainWindow::instance() when the is no SVMainWindow "
		"instance (anylonger).");
	return *m_self;
}


void SVMainWindow::addUndoCommand(QUndoCommand * command) {
	SVMainWindow::instance().m_undoStack->push(command);
	// mark project as modified
	SVMainWindow::instance().updateWindowTitle();
}


SVMainWindow::SVMainWindow(QWidget * /*parent*/, Qt::WindowFlags /*flags*/) :
	m_ui(new Ui::SVMainWindow),
	m_undoStack(new QUndoStack(this)),
	m_postProcHandler(new SVPostProcHandler),
	m_viewStateHandler(new SVViewStateHandler)
{
	// store pointer to this object for global access
	m_self = this;

	m_ui->setupUi(this);

	// give the splashscreen a few miliseconds to show on X11 before we start our
	// potentially lengthy initialization
	QTimer::singleShot(25, this, SLOT(setup()));
}


SVMainWindow::~SVMainWindow() {
	delete m_ui;
	delete m_undoStack;
	delete m_postProcHandler;
	delete m_viewStateHandler;

	// explicitely delete all top-level DB edit widgets
	delete m_dbMaterialsEditWidget;

	m_self = nullptr;
}


bool SVMainWindow::saveProject() {
	on_actionFileSave_triggered();
	return	!SVProjectHandler::instance().isModified() &&
			!SVProjectHandler::instance().projectFile().isEmpty();
}


bool SVMainWindow::exportProjectPackage(const QString & exportFilePath, bool withTopLevelDir) {
	// we create the project package in the target file's directory
	QFileInfo	finfo(exportFilePath);
	QDir targetBaseDir = finfo.absoluteDir();

	// create target directory if it does not exist
	QString targetDirPath = targetBaseDir.absoluteFilePath(finfo.baseName())+"_folder2zip";
	// make sure that the directory does not yet exist
	if (QDir(targetDirPath).exists()) {
		int res = QMessageBox::question(this, tr("Export error"),
							  tr("Export directory '%1' exists already. Overwrite?")
							  .arg(targetDirPath));
		if (res == QDialog::Rejected)
			return false;
		// try to remove directory
		QtExt::Directories::removeDirRecursively(targetDirPath);
		if (QDir(targetDirPath).exists()) {
			QMessageBox::critical(this, tr("Export error"),
								  tr("Export directory cannot be removed (are files within directory still being used?). "
						 "Please remove directory manually and try again!"));
			return false;
		}
	}

	targetBaseDir.mkpath(targetDirPath);

	// store top-level archive path
	QString toplevelDirPath = targetDirPath;
	if (withTopLevelDir) {

		targetDirPath += "/" + QFileInfo(m_projectHandler.projectFile()).baseName() + "_package";
		targetBaseDir.mkpath(targetDirPath);
	}

	if (!exportProjectCopy(targetDirPath, m_projectHandler.project()))
		return false;

	if (withTopLevelDir) {
		// restore top-level path for call to zip routine
		targetDirPath = toplevelDirPath;
	}

	// zip project and copy zip to target path
	bool success = JlCompress::compressDir(exportFilePath, targetDirPath);
	if (!success) {
		QMessageBox::critical(this, tr("Export error"),
							  tr("Cannot create project package '%1'.")
							  .arg(exportFilePath));
		QtExt::Directories::removeDirRecursively(targetDirPath);
		return false;
	}

	// finally remove export directory
	QtExt::Directories::removeDirRecursively(targetDirPath);
	return true;
}



bool SVMainWindow::exportProjectCopy(QString targetDirPath, const VICUS::Project & project) {
	QDir targetBaseDir(targetDirPath);
	// create subdirectory for materials
	targetBaseDir.mkpath(targetDirPath + "/db");
	// create subdirectory for climate data (if any climate data reference is used)
	targetBaseDir.mkpath(targetDirPath + "/climate");

	// TODO : implement project export
	(void)project;

	return true;
}


void SVMainWindow::changeEvent(QEvent *event) {
	FUNCID(SVMainWindow::changeEvent);
	if (event->type() == QEvent::ActivationChange && this->isActiveWindow()){
		if (SVProjectHandler::instance().isValid() && !SVProjectHandler::instance().projectFile().isEmpty()) {
			// check for externally modified project file and trigger "reload" action
			QDateTime lastModified = QFileInfo(SVProjectHandler::instance().projectFile()).lastModified();
			QDateTime lastReadTime = SVProjectHandler::instance().lastReadTime();
			if (lastModified > lastReadTime) {
				IBK::IBK_Message(IBK::FormatString("Last read time '%1', last modified '%2', asking for update.\n")
					.arg(lastReadTime.toString().toStdString())
					.arg(lastModified.toString().toStdString()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
				// update last read time to avoid duplicate call
				SVProjectHandler::instance().updateLastReadTime();

				int res = QMessageBox::question(this, tr("Reload project file"),
									  tr("The project file has been modified by an external application. "
										 "When reloading this project file all unsaved changes will be lost. "
										 "Reload modified project file?"), QMessageBox::Yes | QMessageBox::No);
				if (res == QMessageBox::Yes) {
					// reload project
					m_projectHandler.reloadProject(this);
					IBK::IBK_Message(IBK::FormatString("New last read time '%1'.\n")
						.arg(lastReadTime.toString().toStdString()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);

					if (m_projectHandler.isValid())
						saveThumbNail();
				}
			}
		}
	}

	QMainWindow::changeEvent(event);
}


void SVMainWindow::closeEvent(QCloseEvent * event) {
	FUNCID(SVMainWindow::closeEvent);

	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();

	// remember current project
	if ( m_projectHandler.isValid() ) {
		// store current project file because closeProject will clear this
		QString currentProjectFile = m_projectHandler.projectFile();
		// make sure we have saved and closed our project
		if (!m_projectHandler.closeProject(this)) {
			// user must have cancelled, so prevent closing of the application
			event->ignore();
			return;
		}
		SVSettings::instance().m_lastProjectFile = currentProjectFile;
	}
	else {
		SVSettings::instance().m_lastProjectFile.clear();
	}

	if (!m_threadPool.isEmpty()) {
		// show material reader dialog

		disconnect(m_threadPool[0], SIGNAL(finished()), this, SLOT(onWorkerThreadFinished()));
		m_threadPool[0]->stop();
		IBK::IBK_Message("Waiting for worker thread to finish.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::MessageHandlerRegistry::instance().setDefaultMsgHandler();
		m_threadPool[0]->wait(1000);
	}

	// TODO : store view settings

	// store list of visible dock widgets
	QStringList dockWidgetVisibility;

	// TODO : other dock widgets view configs

	if (m_dockWidgetVisibility[m_logDockWidget])	dockWidgetVisibility.append("Log");
	SVSettings::instance().m_visibleDockWidgets = dockWidgetVisibility;

	// save user config and recent file list
	SVSettings::instance().write(saveGeometry(), saveState());

	event->accept();
}


void SVMainWindow::setup() {

	// setup log widget already, so that error messages resulting from initialization errors are already
	// send to the log widget even before the actual dock widget for the log has been created
	m_logWidget = new SVLogWidget(this);
	SVMessageHandler * msgHandler = dynamic_cast<SVMessageHandler *>(IBK::MessageHandlerRegistry::instance().messageHandler());
	connect(msgHandler, SIGNAL(msgReceived(int,QString)), m_logWidget, SLOT(onMsgReceived(int, QString)));

	// *** setup welcome widget ***

	QHBoxLayout * lay = new QHBoxLayout;
	m_welcomeScreen = new SVWelcomeScreen(this);
	lay->addWidget(m_welcomeScreen);
	lay->setMargin(0);
	lay->setSpacing(0);
	m_ui->centralWidget->setLayout(lay);
	m_welcomeScreen->updateWelcomePage();

	connect(m_welcomeScreen, SIGNAL(newProjectClicked()), this, SLOT(on_actionFileNew_triggered()));
	connect(m_welcomeScreen, SIGNAL(openProjectClicked()), this, SLOT(on_actionFileOpen_triggered()));
	connect(m_welcomeScreen, SIGNAL(openProject(QString)), this, SLOT(onOpenProjectByFilename(QString)));
	connect(m_welcomeScreen, SIGNAL(openExample(QString)), this, SLOT(onOpenExampleByFilename(QString)));
	connect(m_welcomeScreen, SIGNAL(openTemplate(QString)), this, SLOT(onOpenTemplateByFilename(QString)));
	connect(m_welcomeScreen, SIGNAL(updateRecentList()), this, SLOT(onUpdateRecentProjects()));
	connect(m_welcomeScreen, SIGNAL(softwareUpdateRequested()), this, SLOT(on_actionHelpCheckForUpdates_triggered()));

	// *** connect to ProjectHandler signals ***

	connect(&m_projectHandler, SIGNAL(updateActions()), this, SLOT(onUpdateActions()));
	connect(&m_projectHandler, SIGNAL(updateRecentProjects()), this, SLOT(onUpdateRecentProjects()));
	connect(&m_projectHandler, SIGNAL(fixProjectAfterRead()), this, SLOT(onFixProjectAfterRead()));


	// *** create menu for recent files ***

	m_recentProjectsMenu = new QMenu(this);
	m_ui->actionFileRecentProjects->setMenu(m_recentProjectsMenu);
	onUpdateRecentProjects();

	// *** Create splitter that holds navigation tree view and geometry view

	m_geometryViewSplitter = new QSplitter(this);
	// TODO : Stephan SVStyle::formatSplitter(m_geometryViewSplitter);
	lay->addWidget(m_geometryViewSplitter);

	// *** Navigation tree

	m_navigationTreeWidget = new SVNavigationTreeWidget(this);
	m_geometryViewSplitter->addWidget(m_navigationTreeWidget);
	m_geometryViewSplitter->setCollapsible(0, false);

	// *** Create geometry view ***
	m_geometryView = new SVGeometryView(this);
	m_geometryViewSplitter->addWidget(m_geometryView);
	m_geometryViewSplitter->setCollapsible(1, false);


	// *** setup tool bar (add actions for undo and redo) ***

	QAction * undoAction = m_undoStack->createUndoAction(this, tr("Undo"));
	undoAction->setIcon(QIcon(":/gfx/actions/24x24/undo.png"));
	undoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
	QAction * redoAction = m_undoStack->createRedoAction(this, tr("Redo"));
	redoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
	redoAction->setIcon(QIcon(":/gfx/actions/24x24/redo.png"));

	// this is a bit messy, but there seems to be no other way, unless we create the whole menu ourselves
	QList<QAction*> acts = m_ui->menu_Edit->actions();
	m_ui->menu_Edit->addAction(undoAction);
	m_ui->menu_Edit->addAction(redoAction);
	// now move all the actions to bottom
	for (int i=0; i<acts.count(); ++i)
		m_ui->menu_Edit->addAction(acts[i]);

	m_ui->toolBar->addAction(undoAction);
	m_ui->toolBar->addAction(redoAction);
	m_ui->menu_View->addAction(m_ui->toolBar->toggleViewAction());

	// *** Create definition lists dock widgets
	setupDockWidgets();


	// *** restore state of UI ***
	QByteArray geometry, state;
	SVSettings::instance().readMainWindowSettings(geometry,state);
	if (!state.isEmpty())
		restoreState(state);
	if (!geometry.isEmpty())
		restoreGeometry(geometry);

	// *** update actions/UI State depending on project ***
	onUpdateActions();
	// Note: this will initialize the m_dockWidgetVisibility map with all false values, because
	// we do not have a project yet and all dock widgets are invisible

	// *** retrieve visibility of dock widgets from settings ***
	// TODO : other dock widgets
	m_dockWidgetVisibility[m_logDockWidget] = SVSettings::instance().m_visibleDockWidgets.contains("Log");

	// initialize view mode
	on_actionViewToggleGeometryMode_triggered();

	// *** Populate language menu ***
	addLanguageAction("en", "English");
	addLanguageAction("de", "Deutsch");
//	addLanguageAction("fr", QString::fromUtf8("Français"));
//	addLanguageAction("cz", QString::fromUtf8("Czech"));
//	addLanguageAction("es", QString::fromUtf8("Español"));
//	addLanguageAction("it", QString::fromUtf8("Italiano"));

	// *** read last loaded project/project specified on command line ***

	if (!SVSettings::instance().m_initialProjectFile.isEmpty()) {
		QString filename = SVSettings::instance().m_initialProjectFile;
		if (processProjectPackage(filename, false)) {
			// try to load the project - silently
			m_projectHandler.loadProject(this, filename, false);
			if (m_projectHandler.isValid())
				saveThumbNail();
		}
	}


	// add user settings related window resize at program start
#if defined(Q_OS_WIN)
	showMaximized();
#elif defined(Q_OS_LINUX)
	show();
#else
	show();
#endif

}


void SVMainWindow::on_actionFileNew_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// close project if we have one
	if (!m_projectHandler.closeProject(this)) // emits updateActions() if project was closed
		return;

	// create new project
	m_projectHandler.newProject(); // emits updateActions()
	// TODO : switch to geometry view
}


void SVMainWindow::on_actionFileOpen_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// close project if we have one
	if (!m_projectHandler.closeProject(this)) // emits updateActions() if project was closed
		return;

	// request file name
	QString filename = QFileDialog::getOpenFileName(
							this,
							tr("Select SIM-VICUS project"),
							SVSettings::instance().m_propertyMap[SVSettings::PT_LastFileOpenDirectory].toString(),
							tr("SIM-VICUS projects and project packages (*%1 *%2);;All files (*.*)")
			.arg(SVSettings::instance().m_projectFileSuffix).arg(SVSettings::instance().m_projectPackageSuffix)
						);

	if (filename.isEmpty()) return;

	QFile f1(filename);
	if (!f1.exists()) {
		QMessageBox::critical(
					this,
					tr("File not found"),
					tr("The file '%1' does not exist or cannot be accessed.").arg(filename)
			);
		return;
	}

	// if we have a project package, first extract its content into a suitable subdirectory
	if (!processProjectPackage(filename, false))
		return;

	m_projectHandler.loadProject(this, filename, false); // emits updateActions() if project was successfully loaded
	if (m_projectHandler.isValid()) {
		saveThumbNail();
		SVSettings::instance().m_propertyMap[SVSettings::PT_LastFileOpenDirectory] = QFileInfo(filename).absoluteDir().absolutePath();
	}
}


void SVMainWindow::on_actionFileSave_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// check if we have a file name
	if (m_projectHandler.projectFile().isEmpty()) {
		m_projectHandler.saveWithNewFilename(this);
	}
	else {
		// Note: should not be possible of project hasn't been modified
		m_projectHandler.saveProject(this, m_projectHandler.projectFile()); // emits updateActions() if project was successfully saved
	}
	saveThumbNail();
}


void SVMainWindow::on_actionFileSaveAs_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	m_projectHandler.saveWithNewFilename(this); // emits updateActions() if project was successfully saved
	saveThumbNail();
}


void SVMainWindow::on_actionFileReload_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// if project has not yet been saved, it cannot be reloaded
	if (m_projectHandler.projectFile().isEmpty()) {
		QMessageBox::information(this, tr("Reload project"), tr("The project has not yet been saved."));
		return;
	}
	// if modified, ask user to confirm loosing changes
	if (m_projectHandler.isModified()) {
		if (QMessageBox::question(this, tr("Reload project"), tr("The project has been modified. Discard those changes?"),
								  QMessageBox::Discard | QMessageBox::Abort) == QMessageBox::Abort)
		{
			return;
		}
	}
	// reload project
	m_projectHandler.reloadProject(this);

	if (m_projectHandler.isValid())
		saveThumbNail();
}


void SVMainWindow::on_actionFileImportEneryPlusIDF_triggered() {
	// request IDF file and afterwards open import dialog
	QString filename = QFileDialog::getOpenFileName(
							this,
							tr("Select IDF file"),
							SVSettings::instance().m_propertyMap[SVSettings::PT_LastFileOpenDirectory].toString(),
							tr("EnergyPlus IDF files (*.idf);;All files (*.*)")
						);

	if (filename.isEmpty()) return;

	QFile f1(filename);
	if (!f1.exists()) {
		QMessageBox::critical(
					this,
					tr("File not found"),
					tr("The file '%1' does not exist or cannot be accessed.").arg(filename)
			);
		return;
	}

	// now spawn import dialog
	if (m_importIDFDialog == nullptr) {
		m_importIDFDialog = new SVImportIDFDialog(this);
	}

	SVImportIDFDialog::ImportResults res = m_importIDFDialog->import(filename);

	switch (res) {
		case SVImportIDFDialog::ReplaceProject : {
			setFocus();
			// close project if we have one
			if (!m_projectHandler.closeProject(this)) // emits updateActions() if project was closed
				return;

			// create new project
			m_projectHandler.newProject(&m_importIDFDialog->m_importedProject); // emits updateActions()
		} break;

		case SVImportIDFDialog::MergeProjects : {
			// import and merge databases

			// take building from project and add as new building to existing data structure via undo-action
			if (!m_importIDFDialog->m_importedProject.m_buildings.empty()) {
				SVUndoAddBuilding * undo = new SVUndoAddBuilding(tr("Added imported building"), m_importIDFDialog->m_importedProject.m_buildings[0]);
				undo->push();
			}
		} break;

		case SVImportIDFDialog::ImportCancelled :
			return;
	}
}


void SVMainWindow::on_actionFileOpenProjectDir_triggered() {
	if (m_projectHandler.projectFile().isEmpty()) {
		QMessageBox::critical(this, QString(), tr("Please save the project file first!"));
		return;
	}


	QString projectDir = QFileInfo(m_projectHandler.projectFile() ).dir().path();
	QDesktopServices::openUrl( QUrl::fromLocalFile( projectDir ) );
}


void SVMainWindow::onStyleChanged() {
	m_welcomeScreen->updateWelcomePage();
	m_welcomeScreen->update();
}


void SVMainWindow::on_actionFileExport_triggered() {
	// project must have been saved once already
	if (!saveProject())
		return;

	// request export directory
	QFileInfo finfo(m_projectHandler.projectFile());
	QString fnameSuggestion = finfo.absoluteDir().absolutePath() + "/" + finfo.baseName() + ".vicpac";
	QString filename = QFileDialog::getSaveFileName(
							this,
							tr("Specify SIM-VICUS project package"),
							fnameSuggestion,
							tr("SIM-VICUS project packages (*.vicpac);;All files (*.*)"));

	if (filename.isEmpty())
		return;

	// ensure that we have the proper extension
	if (!filename.endsWith(".vicpac")) {
		filename.append(".vicpac");
	}

	QString dirName = QFileInfo(filename).baseName();
	if (dirName.isEmpty()) {
		QMessageBox::critical(this, tr("Invalid file name"), tr("Please enter a valid file name!"));
		return;
	}

	if (exportProjectPackage(filename, false)) {
		QMessageBox::information(this, tr("Export project package"), tr("Export of project package '%1' complete.")
								 .arg(QFileInfo(filename).fileName()));
	}
}


void SVMainWindow::on_actionFileExportFMU_triggered() {
	// project must have been saved once, already
	if (!saveProject())
		return;

	// TODO : Implement FMU export dialog

	//	if (m_fmiExportDialog == nullptr)
//		m_fmiExportDialog = new SVFMIExportDialog(this);

}


void SVMainWindow::on_actionFileClose_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	m_projectHandler.closeProject(this);
}


void SVMainWindow::on_actionFileQuit_triggered() {
	close();
}


void SVMainWindow::on_actionEditTextEditProject_triggered() {
	SVSettings::instance().openFileInTextEditor(this, m_projectHandler.projectFile());
}


void SVMainWindow::on_actionEditPreferences_triggered() {
	// spawn preferences dialog
	if (m_preferencesDialog == nullptr) {
		m_preferencesDialog = new SVPreferencesDialog(this);
		connect(m_preferencesDialog->pageStyle(), &SVPreferencesPageStyle::styleChanged,
				this, &SVMainWindow::onStyleChanged);
		connect(m_preferencesDialog->pageStyle(), &SVPreferencesPageStyle::styleChanged,
				m_geometryView->sceneView(), &Vic3D::SceneView::onStyleChanged);
	}

	m_preferencesDialog->edit(0); // changes are stored automatically.
}


void SVMainWindow::on_actionEditCleanProject_triggered() {

	// create a copy of the whole project
	VICUS::Project cleanProject = SVProjectHandler::instance().project();

	// clean it
	cleanProject.clean();

	// create undo action and push it
	SVUndoProject * undo = new SVUndoProject( tr("Removed unused definitions"), cleanProject );
	undo->push();

}


void SVMainWindow::on_actionNetworkImport_triggered() {
	// opens import network dialog
	if (m_networkImportDialog == nullptr)
		m_networkImportDialog = new SVNetworkImportDialog(this);

	m_networkImportDialog->edit();
}


void SVMainWindow::on_actionViewExternalPostProcessing_triggered() {
	// configure PostProc session, save parallel to project and open session in post

	if (SVSettings::instance().m_postProcExecutable.isEmpty() ||
			!QFileInfo::exists(SVSettings::instance().m_postProcExecutable))
	{
		QMessageBox::information(this, tr("Setup external tool"), tr("Please select first the path to the external "
																  "post processing in the preferences dialog!"));
		// spawn preferences dialog
		if (m_preferencesDialog == nullptr)
			m_preferencesDialog = new SVPreferencesDialog(this);

		m_preferencesDialog->edit(0);
		return;
	}
	// if we are using the new post-processing, generate a session file:
	if (QFileInfo(SVSettings::instance().m_postProcExecutable).baseName() == "PostProcApp") {

		QString sessionFile;
		if (m_projectHandler.isValid()) {
			IBK::Path sessionFilePath = SVPostProcBindings::defaultSessionFilePath(m_projectHandler.projectFile());
			if(sessionFilePath.isValid()) {
				if (!sessionFilePath.exists())
					SVPostProcBindings::generateDefaultSessionFile(m_projectHandler.projectFile());
				sessionFile = QString::fromStdString(sessionFilePath.str());
			}
			// some session files exist in project directory - look for the right one
			else {
				sessionFile = QFileDialog::getOpenFileName(nullptr, tr("Postproc session files"),
															QFileInfo(m_projectHandler.projectFile()).absolutePath(),
															QString("*.p2"));
			}
		}

		// check, if already an instance of PostProc is running
		int res = m_postProcHandler->reopenIfActive();
		if (res != 0) {
			// try to spawn new postprocessing
			if (!m_postProcHandler->spawnPostProc(sessionFile.toStdString())) {
				QMessageBox::critical(this, tr("Error running PostProc"),
									  tr("Could not start executable '%1'.").arg(SVSettings::instance().m_postProcExecutable));
				return;
			}
		}
#if !defined(Q_OS_WIN)
		else {
			QMessageBox::information(this, tr("Error running PostProc"),
								  tr("Process already running."));
		}
#endif
	}
	else {
		// check, if already an instance of PostProc is running
		int res = m_postProcHandler->reopenIfActive();
		if (res != 0) {
			if (!m_postProcHandler->spawnPostProc(std::string())) {
				QMessageBox::critical(this, tr("Error running PostProc"),
									  tr("Could not start executable '%1'.").arg(SVSettings::instance().m_postProcExecutable));
				return;
			}
		}
	}
}


void SVMainWindow::on_actionViewCCMeditor_triggered() {

	QString ccmPath = SVSettings::instance().m_CCMEditorExecutable;
	if (ccmPath.isEmpty() || !QFileInfo::exists(ccmPath)) {
		QMessageBox::information(this, tr("Setup external tool"), tr("Please select first the path to the external "
																  "climate editor in the preferences dialog!"));
		// spawn preferences dialog
		if (m_preferencesDialog == nullptr)
			m_preferencesDialog = new SVPreferencesDialog(this);

		m_preferencesDialog->edit(0);
		return;
	}
	QProcess p;
	bool res = p.startDetached("\"" + ccmPath + "\"");
	if (!res) {
		QMessageBox::critical(this, tr("Error starting external application"), tr("Climate editor '%1' could not be started.")
							  .arg(ccmPath));
	}
}


void SVMainWindow::on_actionHelpAboutQt_triggered() {
	QMessageBox::aboutQt(this, tr("About Qt..."));
}


void SVMainWindow::on_actionHelpAbout_triggered() {
	SVAboutDialog dlg(this);
	dlg.exec();
}


void SVMainWindow::on_actionHelpBugReport_triggered() {
	QDesktopServices::openUrl( QUrl(BUG_REPORT_URL));
}

void SVMainWindow::on_actionHelpCheckForUpdates_triggered() {

}


void SVMainWindow::on_actionHelpVisitDiscussionForum_triggered() {
	QDesktopServices::openUrl( QUrl(FORUM_URL));
}


void SVMainWindow::onActionOpenRecentFile() {
	QAction *action = qobject_cast<QAction *>(sender());
	if (action) {
		// before closing the project, check if the project file exists
		QString fname = action->data().toString();
		onOpenProjectByFilename(fname);
	}
}


void SVMainWindow::onActionSwitchLanguage() {
	QAction * a = (QAction *)sender();
	QString langId = a->data().toString();
	SVSettings::instance().m_langId = langId;
	QMessageBox::information(this, tr("Languange changed"), tr("Please restart the software to activate the new language!"));
}



void SVMainWindow::onUpdateActions() {
	// purpose of this function is to update the view layout based on the existance of a project or none

	// do we have a project?
	bool have_project = m_projectHandler.isValid();
	// enable/disable all actions that require a project

	// *** Project-dependent actions ***

	m_ui->actionFileSave->setEnabled(have_project);
	m_ui->actionFileSaveAs->setEnabled(have_project);
	m_ui->actionFileExport->setEnabled(have_project);
	m_ui->actionFileReload->setEnabled(have_project);
	m_ui->actionFileClose->setEnabled(have_project);
	m_ui->actionFileExportNANDRAD->setEnabled(have_project);
	m_ui->actionFileExportEnergyPlus->setEnabled(have_project);
	m_ui->actionFileOpenProjectDir->setEnabled(have_project);

	m_ui->actionEditTextEditProject->setEnabled(have_project);
	m_ui->actionEditCleanProject->setEnabled(have_project);

	m_ui->actionNetworkImport->setEnabled(have_project);
	m_ui->actionNetworkEdit->setEnabled(have_project);

	m_ui->actionViewToggleGeometryMode->setEnabled(have_project);

	m_ui->actionSimulationNANDRAD->setEnabled(have_project);
	m_ui->actionSimulationHydraulicNetwork->setEnabled(have_project);

	// also disable menues that are not possible without project
	m_ui->menuExport->setEnabled(have_project);

	// no project, no undo actions -> clearing undostack also disables undo actions
	if (!have_project)
		m_undoStack->clear();

	// *** Geometry view ***

	// turn off geometry view first when project is gone
	if (!have_project) {
		m_geometryViewSplitter->setVisible(false);
	}

	// *** Dock widgets ***

	// Dock-Widgets are only visible when project is there
	if (!have_project) {
		// store dock widget visibility, but only if we are in construction view mode
		storeDockWidgetVisibility();

		m_logDockWidget->toggleViewAction()->setEnabled(false);

		m_logDockWidget->setVisible(false);
	}
	// Note: in case of a project, the dock widgets visibility is set in onNavigationBarViewChanged() below

	// *** View configuration ***

	// show welcome page only when we have no project
	m_welcomeScreen->setVisible(!have_project);
	// navigation bar is only visible when we have a project
	m_ui->toolBar->setVisible(have_project);
	m_ui->toolBar->setEnabled(have_project);

	// Note: in case of a project, the current view widget is set visible onNavigationBarViewChanged() below

	// when we have a project
	if (have_project) {
		// select the current view, this also enables (in case of ConstructionView) the visibility of the dock widgets
		m_logDockWidget->setVisible(m_dockWidgetVisibility[m_logDockWidget]);
		m_logDockWidget->toggleViewAction()->setEnabled(true);

		m_geometryViewSplitter->setVisible(true);
		m_ui->toolBar->setVisible(true);
		m_ui->toolBar->toggleViewAction()->setEnabled(true);

		// adjust size of navigation view to be about 250 px wide or to a user-saved size
		// TODO : whenever user resizes the splitter, the new width should be saved in the settings
		//        and re-applied next time the geometry view is shown
		QList<int> sizes;
		int availableWidth = width();
		if (m_ui->toolBar->isVisibleTo(this))
			availableWidth -= m_ui->toolBar->width();
		sizes << 250 << availableWidth - 250;
		m_geometryViewSplitter->setSizes(sizes);
	}
	else {
		m_ui->toolBar->setVisible(false);
		m_ui->toolBar->toggleViewAction()->setEnabled(false);
		m_logDockWidget->setVisible(false);
		m_logDockWidget->toggleViewAction()->setEnabled(false);
	}

	// also update window caption and status bar
	if (have_project) {
		updateWindowTitle();
	}
	else {
		setWindowTitle(QString("SIM-VICUS %1").arg(VICUS::VERSION));
		m_welcomeScreen->updateWelcomePage();
	}
}


void SVMainWindow::onUpdateRecentProjects() {
	// create actions for recent files if number of max. recent projects in settings
	// differs from current number
	if (m_recentProjectActions.count() != (int)SVSettings::instance().m_maxRecentProjects) {
		qDeleteAll(m_recentProjectActions);
		m_recentProjectActions.clear();
		for (unsigned int i = 0; i < SVSettings::instance().m_maxRecentProjects; ++i) {
			QAction * a = new QAction(this);
			m_recentProjectActions.push_back(a);
			connect(m_recentProjectActions[i], SIGNAL(triggered()), this, SLOT(onActionOpenRecentFile()));
			m_recentProjectsMenu->addAction(m_recentProjectActions[i]);
		}
	}

	// disable recent file actions
	if (SVSettings::instance().m_recentProjects.isEmpty()) {
		m_ui->actionFileRecentProjects->setEnabled(false);
		return;
	}
	else {
		m_ui->actionFileRecentProjects->setEnabled(true);
		for ( int i = 0, count = SVSettings::instance().m_recentProjects.count(); i < count; ++i) {
			/// \bug Fix underscore/whitespace display in menu action
			m_recentProjectActions[i]->setText(SVSettings::instance().m_recentProjects[i]);
			m_recentProjectActions[i]->setData(SVSettings::instance().m_recentProjects[i]);
			m_recentProjectActions[i]->setVisible(true);
		}

		for (unsigned int i = SVSettings::instance().m_recentProjects.count();
			i < SVSettings::instance().m_maxRecentProjects; ++i)
		{
			m_recentProjectActions[i]->setVisible(false);
		}
	}
}


void SVMainWindow::onOpenProjectByFilename(const QString & filename) {
	QFile f1(filename);
	if (!f1.exists()) {
		QMessageBox::critical(this, tr("File not found"), tr("The file '%1' cannot be found or does not exist.").arg(filename));
		return;
	}
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// we first need to close the current project
	if (!m_projectHandler.closeProject(this)) return;
	// then create a new project and try to load the file
	m_projectHandler.loadProject(this, filename, false);
	if (m_projectHandler.isValid())
		saveThumbNail();
	// if failed, no view state change needed
}


void SVMainWindow::onOpenExampleByFilename(const QString & filename) {
	// This function is only called from the welcome page, so there must not be a project opened!
	QFile f1(filename);
	if (!f1.exists()) {
		QMessageBox::critical(this, tr("File not found"), tr("The file '%1' cannot be found or does not exist.").arg(filename));
		return;
	}
	// determine parent directory and ask user to select an example base directory to copy the input data into
	QFileInfo finfo(filename);
	QDir srcDir = finfo.absoluteDir();
	QSettings settings( SVSettings::instance().m_organization, SVSettings::instance().m_appName );
	QString lastExampleTargetDir = settings.value("LastExampleSaveDirectory", QDir::homePath()).toString();
	QString targetDir = QFileDialog::getExistingDirectory(this, tr("Select directory to copy example project into"), lastExampleTargetDir);
	if (targetDir.isEmpty())
		return;
	settings.setValue("LastExampleSaveDirectory", targetDir);

	// append base directory name to target Dir
	targetDir = QDir(targetDir).absoluteFilePath( srcDir.dirName() );

	// now recursively copy 'srcDir' into targetDir
	if (!copyRecursively(srcDir.absolutePath(), targetDir)) {
		QMessageBox::critical(this, tr("Write error"), tr("Cannot copy example, maybe missing permissions?"));
		return;
	}

	// and finally open the copied project file and hope for the best
	QString newFName = QDir(targetDir).absoluteFilePath(finfo.fileName());
	onOpenProjectByFilename(newFName);
}


void SVMainWindow::onOpenTemplateByFilename(const QString & filename) {

	// filename points to a SIM-VICUS project package

	// we re-use the open project package functionality - this will automatically ask user for a target directory to
	// save the project into - we add a flag so that the user can enter also a target file name and after extracting
	// the project package, the project file will be renamed accordingly

	// we have a project package, first extract its content into a suitable subdirectory
	QString fname = filename;
	if (!processProjectPackage(fname, true))
		return;

	// fname now holds the full file path to the extracted and renamed project file in 'fname'

	m_projectHandler.loadProject(this, fname, false); // emits updateActions() if project was successfully loaded
	if (m_projectHandler.isValid()) {
		saveThumbNail();
		SVSettings::instance().m_propertyMap[SVSettings::PT_LastFileOpenDirectory] = QFileInfo(filename).absoluteDir().absolutePath();
	}

}


void SVMainWindow::onWorkerThreadFinished() {

	/// \todo Figure out why thread sends finished signal twice when terminated
	//SVThreadBase * b = (SVThreadBase *)sender();
	if ( m_threadPool.isEmpty() ) {
		return;
	}

	// let the thread object store its data
	m_threadPool[0]->store();

	// we have to remove the object from the thread list again, but later since
	// the thread object is calling this function from its event loop
	m_threadPool[0]->deleteLater(); // when thread is deleted, it will also remove all connections
	// remove first in list
	m_threadPool.removeFirst();

	// and start next task
	processThread();
}



void SVMainWindow::onFixProjectAfterRead() {
	// here we do all entry checks that will tell users about suggested changes in project

	/// \todo implement interactive fixes here with dialogs and user-confirmations

}




// *** Private Functions ***

void SVMainWindow::setupDockWidgets() {
#ifdef VERTICAL_DOCK_WIDGET_BAR
	QDockWidget::DockWidgetFeature titleBarOption = QDockWidget::DockWidgetVerticalTitleBar;
#else // VERTICAL_DOCK_WIDGET_BAR
	QDockWidget::DockWidgetFeature titleBarOption = QDockWidget::NoDockWidgetFeatures;
#endif // VERTICAL_DOCK_WIDGET_BAR

	// *** Log widget ***
	m_logDockWidget = new QDockWidget(this);
	m_logDockWidget->setObjectName("LogDockWidget");
	m_logDockWidget->setContentsMargins(0,0,0,0);
	m_logDockWidget->setWindowTitle(tr("Application Log"));
	m_logDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures | titleBarOption);
	m_logDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
	m_ui->menu_View->addAction(m_logDockWidget->toggleViewAction());
	m_logDockWidget->setWidget(m_logWidget);
	addDockWidget(Qt::BottomDockWidgetArea,m_logDockWidget);

//	tabifyDockWidget(m_outputListDockWidget, m_outputGridListDockWidget);
}


void SVMainWindow::updateWindowTitle() {
	// no project file given?
	QString shortFileName, longFileName;
	if (m_projectHandler.projectFile().isEmpty()) {
		shortFileName = tr("unnamed%1").arg(SVSettings::instance().m_projectFileSuffix);
		longFileName = shortFileName;
	}
	else {
		shortFileName = QFileInfo(m_projectHandler.projectFile()).fileName();
		longFileName = m_projectHandler.projectFile();
	}
	if (m_projectHandler.isModified())
		shortFileName += "*";
	setWindowTitle(QString("SIM-VICUS %1 - %2").arg(VICUS::VERSION).arg(shortFileName));
}


bool SVMainWindow::importProjectPackage(const QString & packageFilePath, const QString & targetDirectory,
										 QString & projectFilePath, bool isPackage)
{
	QStringList extractedFiles = JlCompress::extractDir(packageFilePath, targetDirectory);
	if (extractedFiles.isEmpty()) {
		QMessageBox::critical(this, tr("Import error"),
							  tr("Cannot extract project package '%1'.")
							  .arg(packageFilePath));
		return false;
	}
	else {
		Q_FOREACH(QString fname, extractedFiles) {
			// now try to determine project file in project package
			if (isPackage && fname.endsWith(".vicpac")) {
				projectFilePath = fname;
				break;
			}
			else if (!isPackage && fname.endsWith(".vicpac")) {
				projectFilePath = fname;
				break;
			}
		}
		return true;
	}
}


QString SVMainWindow::saveThumbNail() {
	// check if thumbnail dir exists and if not, create it
	QString thumbNailPath = QtExt::Directories::userDataDir()  + "/thumbs";
	if (!QDir(thumbNailPath).exists())
		QDir().mkpath(thumbNailPath);
	// compose temporary file path
	QString thumbPath = QtExt::Directories::userDataDir()  + "/thumbs/" + QFileInfo(m_projectHandler.projectFile() + ".png").fileName();
	QFileInfo finfo(thumbPath);
	if (finfo.exists()) {
		// only update thumbnail if project file is newer than thumbnail file
		QFileInfo prjFInfo(m_projectHandler.projectFile());
		if (finfo.lastModified() >= prjFInfo.lastModified())
			return thumbPath;
	}
	m_geometryView->saveScreenShot(thumbPath);
	return thumbPath;
}


void SVMainWindow::addLanguageAction(const QString &langId, const QString &actionCaption) {
	FUNCID(SVMainWindow::addLanguageAction);
	QString languageFilename = QString("%1/%2_%3.qm").arg(QtExt::Directories::translationsDir()).arg("SIM-VICUS").arg(langId);
	if (langId == "en" || QFile(languageFilename).exists()) {
		QAction * a = new QAction(actionCaption, this);
		a->setData(langId);
		a->setIcon( QIcon( QString(":/gfx/languages/%1.png").arg(langId)) );
		a->setIconVisibleInMenu(true);
		connect(a, SIGNAL(triggered()),
				this, SLOT(onActionSwitchLanguage()));
		m_ui->menuLanguage->insertAction(nullptr, a);
	}
	else {
		IBK::IBK_Message( IBK::FormatString("Language file '%1' missing.").arg(languageFilename.toUtf8().data()),
						  IBK::MSG_WARNING, FUNC_ID);
	}
}


void SVMainWindow::runWorkerThreadImpl(SVThreadBase * base) {
	m_threadPool.append(base);
	processThread();
}


void SVMainWindow::processThread() {

	//qDebug() << "SVMainWindow::processThread()";
	// no threads? return
	if (m_threadPool.isEmpty()) return;

	// is a thread currently running?
	if (m_threadPool[0]->isRunning()) return;

	// ok, we can run the thread, but first connect the signal to it
	connect(m_threadPool[0], SIGNAL(finished()), this, SLOT(onWorkerThreadFinished()));

	// let the thread object fetch its data
	m_threadPool[0]->fetch();

	// for now we expect the thread to
	m_threadPool[0]->start(QThread::LowPriority); // always start on low priority

	/// \todo What happens if the thread fails to start?
}


void SVMainWindow::storeDockWidgetVisibility() {
	// TODO : other dock widgets
	m_dockWidgetVisibility[m_logDockWidget] = m_logDockWidget->isVisible();
}




bool SVMainWindow::processProjectPackage(QString & filename, bool renameProjectFileAfterwards) {
	if (filename.endsWith(".vicpac")) {
		// determine a suitable starting directory - how about the last directory something was saved in?
		// so take the top-most recent project file and use this path

		QString recentPath;
		if (!SVSettings::instance().m_recentProjects.isEmpty()) {
			recentPath = QFileInfo(SVSettings::instance().m_recentProjects[0]).dir().absolutePath();
		}
		QString targetDir;
		QString targetFilePath;
		if (renameProjectFileAfterwards) {
			recentPath += "/" + QFileInfo(filename).baseName();
			targetFilePath = QFileDialog::getSaveFileName(
					this,
					tr("Specify SIM-VICUS project"),
					recentPath,
					tr("SIM-VICUS project files (%1);;All files (*.*)").arg(SVSettings::instance().m_projectFileSuffix));
			if (targetFilePath.isEmpty())
				return false;
			if (!targetFilePath.endsWith(SVSettings::instance().m_projectFileSuffix))
				targetFilePath += SVSettings::instance().m_projectFileSuffix;
			targetDir = QFileInfo(targetFilePath).dir().absolutePath();
		}
		else {
			targetDir = QFileDialog::getExistingDirectory(
								this,
								tr("Select target directory to extract project package into"),
								recentPath
							);
			if (targetDir.isEmpty())
				return false;
		}
		QString projectFile;
		if (!importProjectPackage(filename, targetDir, projectFile, true))
			return false;
		if (projectFile.isEmpty()) {
			QMessageBox::critical(this, tr("Import error"),
								  tr("Project package does not contain a SIM-VICUS project file (%1-file).").arg(SVSettings::instance().m_projectFileSuffix));
			return false;
		}
		// if renaming is selected, perform the renaming
		if (renameProjectFileAfterwards) {
			// try to delete any existing project files (confirmation was already requested during getSaveFileName()
			if (QFileInfo::exists(targetFilePath)) {
				if (!QFile::remove(targetFilePath)) {
					QMessageBox::critical(this, tr("Write error"), tr("Cannot remove existing project file (maybe missing permissions?)."));
					return false;
				}
			}
			// now rename extracted project file to target project file
			QFile::rename(projectFile, targetFilePath);
			if (!QFileInfo::exists(targetFilePath)) {
				QMessageBox::critical(this, tr("Write error"), tr("Cannot rename project file to target file (invalid file name?)."));
				return false;
			}
			projectFile = targetFilePath;
		}
		// Mind: the importProjectPackage function has stored the extracted vicus-file as most recent file on top of the recent file list.
		//       Since we haven't saved it ourselves, we remove it again from the list
		Q_ASSERT(!SVSettings::instance().m_recentProjects.isEmpty());
		SVSettings::instance().m_recentProjects.removeFirst();
		// and continue loading the project
		filename = projectFile;
	}
	return true;
}


//https://qt.gitorious.org/qt-creator/qt-creator/source/1a37da73abb60ad06b7e33983ca51b266be5910e:src/app/main.cpp#L13-189
// taken from utils/fileutils.cpp. We can not use utils here since that depends app_version.h.
static bool copyRecursively(const QString &srcFilePath,
							const QString &tgtFilePath)
{
	QFileInfo srcFileInfo(srcFilePath);
	if (srcFileInfo.isDir()) {
		QDir targetDir(tgtFilePath);
		targetDir.cdUp();
		// only create subdir if it does not yet exist
		if (!QFileInfo::exists(tgtFilePath))
			if (!targetDir.mkdir(QFileInfo(tgtFilePath).fileName()))
				return false;
		QDir sourceDir(srcFilePath);
		QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
		foreach (const QString &fileName, fileNames) {
			const QString newSrcFilePath
					= srcFilePath + QLatin1Char('/') + fileName;
			const QString newTgtFilePath
					= tgtFilePath + QLatin1Char('/') + fileName;
			if (!copyRecursively(newSrcFilePath, newTgtFilePath))
				return false;
		}
	} else {
		// remove potentially existing file
		if (QFileInfo::exists(tgtFilePath)) {
			if (!QFile::remove(tgtFilePath))
				return false;
		}
		if (!QFile::copy(srcFilePath, tgtFilePath))
			return false;
	}
	return true;
}


void SVMainWindow::on_actionNetworkEdit_triggered() {
	// opens edit network dialog
	if (m_networkEditDialog == nullptr)
		m_networkEditDialog = new SVNetworkEditDialog(this);

	m_networkEditDialog->edit();
}


void SVMainWindow::on_actionViewToggleGeometryMode_triggered() {
	// switch view state to geometry edit mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_viewMode = SVViewState::VM_GeometryEditMode;
	vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	SVViewStateHandler::instance().setViewState(vs);
	m_ui->actionViewToggleGeometryMode->setChecked(true);
	m_ui->actionViewToggleParametrizationMode->setChecked(false);
}


void SVMainWindow::on_actionViewToggleParametrizationMode_triggered() {
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_viewMode = SVViewState::VM_PropertyEditMode;
	vs.m_propertyWidgetMode = SVViewState::PM_SiteProperties;
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	SVViewStateHandler::instance().setViewState(vs);

	m_ui->actionViewToggleParametrizationMode->setChecked(true);
	m_ui->actionViewToggleGeometryMode->setChecked(false);
}


void SVMainWindow::on_actionDBMaterials_triggered() {
	if (m_dbMaterialsEditWidget == nullptr) {
		m_dbMaterialsEditWidget = new SVDBMaterialsEditWidget(nullptr); // global widget, not inside main window
	}
	m_dbMaterialsEditWidget->edit();
}


void SVMainWindow::on_actionDBWindows_triggered() {
	if (m_dbWindowEditWidget == nullptr) {
		m_dbWindowEditWidget = new SVDBWindowEditWidget(nullptr); // global widget, not inside main window
	}
	m_dbWindowEditWidget->edit();
}


void SVMainWindow::on_actionDBConstructions_triggered() {
	if (m_dbConstructionEditDialog == nullptr)
		m_dbConstructionEditDialog = new SVDBConstructionEditDialog(nullptr);
	m_dbConstructionEditDialog->edit();
}


void SVMainWindow::on_actionFileExportNANDRAD_triggered() {
	// ask user for target file name
	// open export dialog, call via edit(fname) (with ok and cancel)
}


void SVMainWindow::on_actionSimulationNANDRAD_triggered() {
	if (m_simulationStartNandrad == nullptr)
		m_simulationStartNandrad = new SVSimulationStartNandrad;
	// open simulation start dialog, with settings for climate location, simulation and
	// solver settings and simulation start button
	int res = m_simulationStartNandrad->edit();
}


void SVMainWindow::on_actionSimulationHydraulicNetwork_triggered() {
	if (m_simulationStartNetworkSim == nullptr)
		m_simulationStartNetworkSim = new SVSimulationStartNetworkSim(this);
	// we require a saved project with at least one network definition
	if (SVProjectHandler::instance().projectFile().isEmpty()) {
		QMessageBox::critical(this, QString(), tr("The project must be saved, first!"));
		return;
	}
	if (project().m_geometricNetworks.empty()) {
		QMessageBox::critical(this, QString(), tr("You need to define at least one network!"));
		return;
	}

	m_simulationStartNetworkSim->edit();
}


void SVMainWindow::on_actionOnline_manual_triggered() {
	QDesktopServices::openUrl( QUrl(MANUAL_URL));
}


void SVMainWindow::on_actionKeyboard_and_mouse_controls_triggered() {
	// show keyboard/mouse control cheat sheet
	QDialog dlg(this);
	QVBoxLayout * lay = new QVBoxLayout(&dlg);
	QTextEdit * w = new QTextEdit(&dlg);
	lay->addWidget(w);
	dlg.setLayout(lay);
	QFile manual_en(":/doc/KeyboardMouseControls.html");
	manual_en.open(QFile::ReadOnly);
	QString manual = manual_en.readAll();
	w->setHtml(manual);
	dlg.resize(1400,800);
	dlg.exec();
}

