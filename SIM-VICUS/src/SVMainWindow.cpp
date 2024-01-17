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
#include <QPluginLoader>
#include <QScreen>
#include <QApplication>
#include <QGuiApplication>
#include <QMessageBox>
#include <QShortcut>
#include <QProgressDialog>

#include <numeric>

#include <IBK_FileUtils.h>
#include <IBK_messages.h>
#include <IBK_NotificationHandler.h>

#include <VICUS_Project.h>
#include <VICUS_Constants.h>

#include <JlCompress.h> // zlib support

#include <QtExt_AutoUpdater.h>
#include <QtExt_Directories.h>
#include <QtExt_LanguageHandler.h>

#include <NANDRAD_Project.h>

#include "SVLcaLccSettingsWidget.h"
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
#include "SVNetworkImportDialog.h"
#include "SVNetworkExportDialog.h"
#include "SVPreferencesPageStyle.h"
#include "SVPreferencesPageMisc.h"
#include "SVViewStateHandler.h"
#include "SVImportIDFDialog.h"
#include "SVPropVertexListWidget.h"
#include "SVPropertyWidget.h"
#include "SVPropEditGeometry.h"
#include "SVStyle.h"
#include "SVPropFloorManagerWidget.h"
#include "SVPropAddWindowWidget.h"
#include "SVNotesDialog.h"
#include "SVSimulationShadingOptions.h"
#include "SVPluginLoader.h"
#include "SVLcaLccResultsWidget.h"
#include "SVAcousticConstraintsCheckWidget.h"

#include "SVDatabaseEditDialog.h"
#include "SVDBZoneTemplateEditDialog.h"
#include "SVDBDuplicatesDialog.h"
#include "SVLogFileDialog.h"

#include "SVDBInternalLoadsTableModel.h"
#include "SVCoSimCO2VentilationDialog.h"

#include "SVGeometryView.h"
#include "Vic3DSceneView.h"

#include "SVUndoModifySiteData.h"
#include "SVSimulationSettingsView.h"
#include "SVStructuralUnitCreationDialog.h"

#include "plugins/SVDatabasePluginInterface.h"
#include "plugins/SVImportPluginInterface.h"


static bool copyRecursively(const QString &srcFilePath, const QString &tgtFilePath);


class ProgressNotifyer : public IBK::NotificationHandler {
public:
	void notify() override {}
	void notify(double percentage, const char *txt) override;
	QProgressDialog		*m_prgDlg = nullptr;
};

void ProgressNotifyer::notify(double percentage, const char *txt) {
	m_prgDlg->setValue((int)(m_prgDlg->maximum() * percentage));
	m_prgDlg->setLabelText(txt);
	qApp->processEvents();
}


SVMainWindow * SVMainWindow::m_self = nullptr;

// *** public static functions ***

SVMainWindow & SVMainWindow::instance() {
	Q_ASSERT_X(m_self != nullptr, "[SVMainWindow::instance]",
			   "You must not access SVMainWindow::instance() when there is no SVMainWindow "
			   "instance (anylonger).");
	return *m_self;
}


void SVMainWindow::addUndoCommand(QUndoCommand * command) {
	SVMainWindow::instance().m_undoStack->push(command);
	// mark project as modified
	SVMainWindow::instance().updateWindowTitle();
}


// *** public functions ***

SVMainWindow::SVMainWindow(QWidget * /*parent*/) :
	m_ui(new Ui::SVMainWindow),
	m_undoStack(new QUndoStack(this)),
	m_pluginLoader(new SVPluginLoader),
	m_postProcHandler(new SVPostProcHandler),
	m_viewStateHandler(new SVViewStateHandler)
{
	// store pointer to this object for global access
	m_self = this;

	m_ui->setupUi(this);

	// give the splashscreen a few miliseconds to show on X11 before we start our
	// potentially lengthy initialization
	QTimer::singleShot(25, this, SLOT(setup()));

#if defined(Q_OS_LINUX) && !defined(IBK_BUILDING_DEBIAN_PACKAGE)
	m_ui->actionHelpLinuxDesktopIntegration->setVisible(true);
#else
	m_ui->actionHelpLinuxDesktopIntegration->setVisible(false);
#endif

	// manually specify keyboard shortcut again, since on Windows this is a "standard shortcut" and get's removed
	// when setting up UI
	m_ui->actionFileClose->setShortcut(QKeySequence((int)Qt::CTRL + Qt::Key_W));
	m_ui->actionViewFindSelectedGeometry->setShortcut(QKeySequence((int)Qt::CTRL + Qt::Key_F));

	// connect "F9" with showing the start simulation page
	QShortcut *shortCutStartSim = new QShortcut(QKeySequence((int)Qt::Key_F9), this);
	connect(shortCutStartSim, &QShortcut::activated, this, &SVMainWindow::onShortCutStartSimulation);

	// enforce using a native window; we need this so we can call window() and retrieve scaling information
	setAttribute(Qt::WA_NativeWindow);
	QWindow *w = window()->windowHandle();
	connect(w, &QWindow::screenChanged, this, &SVMainWindow::onScreenChanged);

	// we need to set the theme name once here
	if (SVSettings::instance().m_theme == SVSettings::TT_Dark)
		QIcon::setThemeName("dark");
	else
		QIcon::setThemeName("light");

	m_ui->actionFileNew->setIcon(QIcon::fromTheme("file_new"));
	m_ui->actionFileOpen->setIcon(QIcon::fromTheme("file_open"));
	m_ui->actionFileSave->setIcon(QIcon::fromTheme("file_save"));

	m_ui->actionGeometryView->setIcon(QIcon::fromTheme("3d_geometry_view"));
	m_ui->actionSimulationSettings->setIcon(QIcon::fromTheme("simulation_view"));
	m_ui->actionOpenPostProcessing->setIcon(QIcon::fromTheme("post_processing"));
}


SVMainWindow::~SVMainWindow() {
	delete m_ui;
	delete m_undoStack;
	delete m_postProcHandler;
	delete m_viewStateHandler;

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


SVDatabaseEditDialog * SVMainWindow::dbMaterialEditDialog() {
	if (m_dbMaterialEditDialog == nullptr) {
		m_dbMaterialEditDialog = SVDatabaseEditDialog::createMaterialEditDialog(this);
	}
	m_dbMaterialEditDialog->resizeDBDialog(0.65);
	return m_dbMaterialEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbEpdEditDialog() {
	if (m_dbEpdEditDialog == nullptr) {
		m_dbEpdEditDialog = SVDatabaseEditDialog::createEpdEditDialog(this);
	}
	m_dbEpdEditDialog->resizeDBDialog(0.6);
	return m_dbEpdEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbConstructionEditDialog() {
	if (m_dbConstructionEditDialog == nullptr) {
		m_dbConstructionEditDialog = SVDatabaseEditDialog::createConstructionEditDialog(this);
	}
	m_dbConstructionEditDialog->resizeDBDialog(0.7);
	return m_dbConstructionEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbComponentEditDialog() {
	if (m_dbComponentEditDialog == nullptr)
		m_dbComponentEditDialog = SVDatabaseEditDialog::createComponentEditDialog(this);
	m_dbComponentEditDialog->resizeDBDialog(0.6);
	return m_dbComponentEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbSubSurfaceComponentEditDialog() {
	if (m_dbSubSurfaceComponentEditDialog == nullptr)
		m_dbSubSurfaceComponentEditDialog = SVDatabaseEditDialog::createSubSurfaceComponentEditDialog(this);
	m_dbSubSurfaceComponentEditDialog->resizeDBDialog(0.6);
	return m_dbSubSurfaceComponentEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbAcousticBoundaryConditionEditDialog(){
	if (m_dbAcousticBoundaryConditionEditDialog == nullptr)
		m_dbAcousticBoundaryConditionEditDialog = SVDatabaseEditDialog::createAcousticBoundaryConditionEditDialog(this);
	return m_dbAcousticBoundaryConditionEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbAcousticSoundAbsorptionEditDialog(){
	if (m_dbAcousticSoundAbsorptionEditDialog == nullptr)
		m_dbAcousticSoundAbsorptionEditDialog = SVDatabaseEditDialog::createAcousticSoundAbsorptionEditDialog(this);
	return m_dbAcousticSoundAbsorptionEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbBoundaryConditionEditDialog() {
	if (m_dbBoundaryConditionEditDialog == nullptr)
		m_dbBoundaryConditionEditDialog = SVDatabaseEditDialog::createBoundaryConditionsEditDialog(this);
	m_dbBoundaryConditionEditDialog->resizeDBDialog();
	return m_dbBoundaryConditionEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbWindowEditDialog() {
	if (m_dbWindowEditDialog == nullptr)
		m_dbWindowEditDialog = SVDatabaseEditDialog::createWindowEditDialog(this);
	m_dbWindowEditDialog->resizeDBDialog();
	return m_dbWindowEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbWindowGlazingSystemEditDialog() {
	if (m_dbWindowGlazingSystemEditDialog == nullptr)
		m_dbWindowGlazingSystemEditDialog = SVDatabaseEditDialog::createWindowGlazingSystemEditDialog(this);
	m_dbWindowGlazingSystemEditDialog->resizeDBDialog();
	return m_dbWindowGlazingSystemEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbPipeEditDialog() {
	if (m_dbPipeEditDialog == nullptr)
		m_dbPipeEditDialog = SVDatabaseEditDialog::createPipeEditDialog(this);
	m_dbPipeEditDialog->resizeDBDialog();
	return m_dbPipeEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbSupplySystemEditDialog() {
	if (m_dbSupplySystemEditDialog == nullptr)
		m_dbSupplySystemEditDialog = SVDatabaseEditDialog::createSupplySystemsEditDialog(this);
	m_dbSupplySystemEditDialog->resizeDBDialog();
	return m_dbSupplySystemEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbNetworkComponentEditDialog() {
	if (m_dbNetworkComponentEditDialog == nullptr)
		m_dbNetworkComponentEditDialog = SVDatabaseEditDialog::createNetworkComponentEditDialog(this);
	m_dbNetworkComponentEditDialog->resizeDBDialog();
	return m_dbNetworkComponentEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbFluidEditDialog() {
	if (m_dbFluidEditDialog == nullptr)
		m_dbFluidEditDialog = SVDatabaseEditDialog::createFluidEditDialog(this);
	m_dbFluidEditDialog->resizeDBDialog();
	return m_dbFluidEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbNetworkControllerEditDialog() {
	if (m_dbNetworkControllerEditDialog == nullptr)
		m_dbNetworkControllerEditDialog = SVDatabaseEditDialog::createNetworkControllerEditDialog(this);
	m_dbNetworkControllerEditDialog->resizeDBDialog();
	return m_dbNetworkControllerEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbSubNetworkEditDialog() {
	if (m_dbSubNetworkEditDialog == nullptr)
		m_dbSubNetworkEditDialog = SVDatabaseEditDialog::createSubNetworkEditDialog(this);
	m_dbSubNetworkEditDialog->resizeDBDialog();
	return m_dbSubNetworkEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbScheduleEditDialog() {
	if (m_dbScheduleEditDialog == nullptr)
		m_dbScheduleEditDialog = SVDatabaseEditDialog::createScheduleEditDialog(this);
	m_dbScheduleEditDialog->resizeDBDialog();
	return m_dbScheduleEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbInternalLoadsPersonEditDialog() {
	if (m_dbInternalLoadsPersonEditDialog == nullptr)
		m_dbInternalLoadsPersonEditDialog = SVDatabaseEditDialog::createInternalLoadsEditDialog(this, VICUS::InternalLoad::IC_Person);
	m_dbInternalLoadsPersonEditDialog->resizeDBDialog();
	return m_dbInternalLoadsPersonEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbInternalLoadsElectricEquipmentEditDialog() {
	if (m_dbInternalLoadsElectricEquipmentEditDialog == nullptr)
		m_dbInternalLoadsElectricEquipmentEditDialog = SVDatabaseEditDialog::createInternalLoadsEditDialog(this, VICUS::InternalLoad::IC_ElectricEquiment);
	m_dbInternalLoadsElectricEquipmentEditDialog->resizeDBDialog();
	return m_dbInternalLoadsElectricEquipmentEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbInternalLoadsLightsEditDialog() {
	if (m_dbInternalLoadsLightsEditDialog == nullptr)
		m_dbInternalLoadsLightsEditDialog = SVDatabaseEditDialog::createInternalLoadsEditDialog(this, VICUS::InternalLoad::IC_Lighting);
	m_dbInternalLoadsLightsEditDialog->resizeDBDialog();
	return m_dbInternalLoadsLightsEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbInternalLoadsOtherEditDialog() {
	if (m_dbInternalLoadsOtherEditDialog == nullptr)
		m_dbInternalLoadsOtherEditDialog = SVDatabaseEditDialog::createInternalLoadsEditDialog(this, VICUS::InternalLoad::IC_Other);
	m_dbInternalLoadsOtherEditDialog->resizeDBDialog();
	return m_dbInternalLoadsOtherEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbZoneControlThermostatEditDialog() {
	if (m_dbZoneControlThermostatEditDialog == nullptr)
		m_dbZoneControlThermostatEditDialog = SVDatabaseEditDialog::createZoneControlThermostatEditDialog(this);
	m_dbZoneControlThermostatEditDialog->resizeDBDialog();
	return m_dbZoneControlThermostatEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbZoneControlVentilationNaturalEditDialog() {
	if (m_dbZoneControlVentilationNaturalEditDialog == nullptr)
		m_dbZoneControlVentilationNaturalEditDialog = SVDatabaseEditDialog::createZoneControlVentilationNaturalEditDialog(this);
	m_dbZoneControlVentilationNaturalEditDialog->resizeDBDialog();
	return m_dbZoneControlVentilationNaturalEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbZoneControlShadingEditDialog() {
	if (m_dbZoneControlShadingEditDialog == nullptr)
		m_dbZoneControlShadingEditDialog = SVDatabaseEditDialog::createZoneControlShadingEditDialog(this);
	m_dbZoneControlShadingEditDialog->resizeDBDialog();
	return m_dbZoneControlShadingEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbVentilationNaturalEditDialog() {
	if (m_dbVentilationNaturalEditDialog == nullptr)
		m_dbVentilationNaturalEditDialog = SVDatabaseEditDialog::createVentilationNaturalEditDialog(this);
	m_dbVentilationNaturalEditDialog->resizeDBDialog();
	return m_dbVentilationNaturalEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbInfiltrationEditDialog() {
	if (m_dbInfiltrationEditDialog == nullptr)
		m_dbInfiltrationEditDialog = SVDatabaseEditDialog::createInfiltrationEditDialog(this);
	m_dbInfiltrationEditDialog->resizeDBDialog();
	return m_dbInfiltrationEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbZoneIdealHeatingCoolingEditDialog() {
	if (m_dbZoneIdealHeatingCoolingEditDialog == nullptr)
		m_dbZoneIdealHeatingCoolingEditDialog = SVDatabaseEditDialog::createZoneIdealHeatingCoolingEditDialog(this);
	m_dbZoneIdealHeatingCoolingEditDialog->resizeDBDialog();
	return m_dbZoneIdealHeatingCoolingEditDialog;
}

SVDBZoneTemplateEditDialog * SVMainWindow::dbZoneTemplateEditDialog() {
	if (m_dbZoneTemplateEditDialog == nullptr)
		m_dbZoneTemplateEditDialog = new SVDBZoneTemplateEditDialog(this);
	return m_dbZoneTemplateEditDialog;
}

SVDatabaseEditDialog * SVMainWindow::dbSurfaceHeatingSystemEditDialog() {
	if (m_dbVSurfaceHeatingSystemEditDialog == nullptr)
		m_dbVSurfaceHeatingSystemEditDialog = SVDatabaseEditDialog::createSurfaceHeatingSystemEditDialog(this);
	m_dbVSurfaceHeatingSystemEditDialog->resizeDBDialog();
	return m_dbVSurfaceHeatingSystemEditDialog;
}


// *** public slots ***


void SVMainWindow::on_actionDBMaterials_triggered() {
	dbMaterialEditDialog()->edit();
}

void SVMainWindow::on_actionDBConstructions_triggered() {
	dbConstructionEditDialog()->edit();
}

void SVMainWindow::on_actionDBWindows_triggered() {
	dbWindowEditDialog()->edit();
}

void SVMainWindow::on_actionDBWindowGlazingSystems_triggered() {
	dbWindowGlazingSystemEditDialog()->edit();
}

void SVMainWindow::on_actionDBComponents_triggered() {
	dbComponentEditDialog()->edit();
	// update all widgets that show the components somewhere (in a combo box or else)
	if (SVViewStateHandler::instance().m_propVertexListWidget != nullptr) // guard against not yet created property widget
		SVViewStateHandler::instance().m_propVertexListWidget->updateComponentComboBoxes();
	if (SVViewStateHandler::instance().m_propAddWindowWidget != nullptr) // guard against not yet created property widget
		SVViewStateHandler::instance().m_propAddWindowWidget->updateSubSurfaceComponentList();
}

void SVMainWindow::on_actionDBSubSurfaceComponents_triggered() {
	dbSubSurfaceComponentEditDialog()->edit();
	// update all widgets that show the components somewhere (in a combo box or else)
	if (SVViewStateHandler::instance().m_propAddWindowWidget != nullptr) // guard against not yet created property widget
		SVViewStateHandler::instance().m_propAddWindowWidget->updateSubSurfaceComponentList();
}

void SVMainWindow::on_actionDBBoundaryConditions_triggered() {
	dbBoundaryConditionEditDialog()->edit();
}

void SVMainWindow::on_actionDBSchedules_triggered() {
	dbScheduleEditDialog()->edit();
}

void SVMainWindow::on_actionDBInternalLoadsPerson_triggered() {
	dbInternalLoadsPersonEditDialog()->edit();
}

void SVMainWindow::on_actionDBInternalLoadsElectricEquipment_triggered() {
	dbInternalLoadsElectricEquipmentEditDialog()->edit();
}

void SVMainWindow::on_actionDBInternalLoadsLights_triggered() {
	dbInternalLoadsLightsEditDialog()->edit();
}

void SVMainWindow::on_actionDBInternalLoadsOther_triggered() {
	dbInternalLoadsOtherEditDialog()->edit();
}

void SVMainWindow::on_actionDBZoneControlThermostat_triggered() {
	dbZoneControlThermostatEditDialog()->edit();
}

void SVMainWindow::on_actionDBZoneControlVentilationNatural_triggered() {
	dbZoneControlVentilationNaturalEditDialog()->edit();
}

void SVMainWindow::on_actionDBZoneControlShading_triggered() {
	dbZoneControlShadingEditDialog()->edit();
}

void SVMainWindow::on_actionDBVentilationNatural_triggered() {
	dbVentilationNaturalEditDialog()->edit();
}

void SVMainWindow::on_actionDBInfiltration_triggered() {
	dbInfiltrationEditDialog()->edit();
}

void SVMainWindow::on_actionDBZoneTemplates_triggered() {
	dbZoneTemplateEditDialog()->edit();
}

void SVMainWindow::on_actionDBZoneIdealHeatingCooling_triggered() {
	dbZoneIdealHeatingCoolingEditDialog()->edit();
}

void SVMainWindow::on_actionDBSurfaceHeatingSystems_triggered() {
	dbSurfaceHeatingSystemEditDialog()->edit();
}


void SVMainWindow::on_actionDBNetworkPipes_triggered() {
	dbPipeEditDialog()->edit();
}

void SVMainWindow::on_actionDBFluids_triggered() {
	dbFluidEditDialog()->edit();
}

void SVMainWindow::on_actionDBHydraulicComponents_triggered() {
	dbNetworkComponentEditDialog()->edit();
}

void SVMainWindow::on_actionDBControllers_triggered() {
	dbNetworkControllerEditDialog()->edit();
}

void SVMainWindow::on_actionDBSubNetworks_triggered() {
	dbSubNetworkEditDialog()->edit();
}

void SVMainWindow::on_actionDBSupplySystems_triggered() {
	dbSupplySystemEditDialog()->edit();
}

void SVMainWindow::on_actionDBRemoveDuplicates_triggered() {
	if (m_dbDuplicatesDialog == nullptr)
		m_dbDuplicatesDialog = new SVDBDuplicatesDialog(this);
	if (SVProjectHandler::instance().isValid()) {
		SVSettings::instance().showDoNotShowAgainMessage(this, "DuplicateRemovalInProjectWarning",
														 tr("Removing database duplicates"),
														 tr("When removing duplicate database elements that are used in the currently open project, "
															"the resulting change to the project data cannot be undone (and none of the earlier changes, either)."));
	}
	bool dbModified = m_dbDuplicatesDialog->removeDuplicates();
	if (dbModified) {
		if (SVProjectHandler::instance().isValid()) {
			// signal an "all changed" to the world so that all colors/db references are being updated
			SVProjectHandler::instance().setModified(SVProjectHandler::AllModified);
			// now clear the undo stack, since we cannot roll-back a DB change
			m_undoStack->clear();
		}
	}
}


// *** protected functions ***



// *** private slots ***


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

	// store view settings
	SVSettings::instance().m_navigationSplitterSize = (int)(m_geometryViewSplitter->sizes()[0] * SVSettings::instance().m_ratio);

	// store list of visible dock widgets
	QStringList dockWidgetVisibility;

	// TODO : other dock widgets view configs

	if (m_dockWidgetVisibility[m_logDockWidget])
		dockWidgetVisibility.append("Log");
	SVSettings::instance().m_visibleDockWidgets = dockWidgetVisibility;

	// save user config and recent file list
	SVSettings::instance().write(saveGeometry(), saveState());

	event->accept();
}


void SVMainWindow::moveEvent(QMoveEvent *event) {
	QMainWindow::moveEvent(event);
	SVViewStateHandler::instance().m_geometryView->moveTransparentSceneWidgets();
}


void SVMainWindow::setup() {

	// setup log widget already, so that error messages resulting from initialization errors are already
	// send to the log widget even before the actual dock widget for the log has been created
	m_logWidget = new SVLogWidget(this);
	SVMessageHandler * msgHandler = dynamic_cast<SVMessageHandler *>(IBK::MessageHandlerRegistry::instance().messageHandler());
	connect(msgHandler, &SVMessageHandler::msgReceived, m_logWidget, &SVLogWidget::onMsgReceived);

	// *** setup welcome widget ***

	QHBoxLayout * mainLayout = new QHBoxLayout;
	m_welcomeScreen = new SVWelcomeScreen(this);
	mainLayout->addWidget(m_welcomeScreen);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	m_ui->centralWidget->setLayout(mainLayout);
	// updates colors and entire content of welcome page
	m_welcomeScreen->onStyleChanged();
	//m_welcomeScreen->setStyleSheet("QWidget {background-image: url(:/gfx/background/simvicus-background-mail.png);}");

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
	mainLayout->addWidget(m_geometryViewSplitter);

	// *** Navigation tree

	m_navigationTreeWidget = new SVNavigationTreeWidget(this);
	m_geometryViewSplitter->addWidget(m_navigationTreeWidget);
	m_geometryViewSplitter->setCollapsible(0, false);

	// *** Geometry view ***

	m_geometryView = new SVGeometryView(this);
	m_geometryView->m_focusRootWidgets.insert(m_navigationTreeWidget); // remember as possible focus widget for events
	m_geometryViewSplitter->addWidget(m_geometryView);
	m_geometryViewSplitter->setCollapsible(1, false);

	connect(SVMainWindow::instance().preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged,
			m_geometryView, &SVGeometryView::onStyleChanged);

	// *** Simulation Settings View

	m_simulationSettingsView = new SVSimulationSettingsView(this);
	mainLayout->addWidget(m_simulationSettingsView);


	// *** Signal/slot connections ***

	Vic3D::SceneView * sv = const_cast<Vic3D::SceneView*>(m_geometryView->sceneView());
	connect(m_navigationTreeWidget, SIGNAL(removeSelected()), sv, SLOT(onDeleteSelected()));
	connect(m_navigationTreeWidget, SIGNAL(showSelected()), sv, SLOT(onShowSelected()));
	connect(m_navigationTreeWidget, SIGNAL(hideSelected()), sv, SLOT(onHideSelected()));
	connect(m_navigationTreeWidget, SIGNAL(selectAll()), sv, SLOT(onSelectAll()));
	connect(m_navigationTreeWidget, SIGNAL(deselectAll()), sv, SLOT(onDeselectAll()));

	// *** setup tool bar (add actions for undo and redo) ***

	m_undoAction = m_undoStack->createUndoAction(this, tr("Undo"));
	m_undoAction->setIcon(QIcon::fromTheme("undo"));
	m_undoAction->setShortcut(QKeySequence((int)Qt::CTRL + Qt::Key_Z));
	m_redoAction = m_undoStack->createRedoAction(this, tr("Redo"));
	m_redoAction->setShortcut(QKeySequence((int)Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
	m_redoAction->setIcon(QIcon::fromTheme("redo"));

	// this is a bit messy, but there seems to be no other way, unless we create the whole menu ourselves
	QList<QAction*> acts = m_ui->menuEdit->actions();
	m_ui->menuEdit->addAction(m_undoAction);
	m_ui->menuEdit->addAction(m_redoAction);

	QWidget* spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	spacer->setFixedSize(QSize(10, 150));
	m_ui->toolBar->addWidget(spacer);

	QList<QAction*> actions = m_ui->toolBar->actions();

	m_ui->toolBar->insertAction(actions[3], m_undoAction);
	m_ui->toolBar->insertAction(actions[3], m_redoAction);

	actions = m_ui->toolBar->actions();

	m_ui->toolBar->insertWidget(actions[3], spacer);
	m_ui->toolBar->insertWidget(actions[5], spacer);

	m_ui->toolBar->insertSeparator(actions[3]);

	m_ui->menuView->addAction(m_ui->toolBar->toggleViewAction());

	// *** Create definition lists dock widgets
	setupDockWidgets();

	// *** restore state of UI ***
	QByteArray geometry, state;
	SVSettings::instance().readMainWindowSettings(geometry,state);
	if (!state.isEmpty())
		restoreState(state);
	if (!geometry.isEmpty()) {
		restoreGeometry(geometry);
	}

	// *** update actions/UI State depending on project ***
	onUpdateActions();
	// Note: this will initialize the m_dockWidgetVisibility map with all false values, because
	// we do not have a project yet and all dock widgets are invisible

	// *** retrieve visibility of dock widgets from settings ***
	// TODO : other dock widgets
	m_dockWidgetVisibility[m_logDockWidget] = SVSettings::instance().m_visibleDockWidgets.contains("Log");


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

	// Set the width of geometryViewSplitter. We do that only once here.
	// If the stored width of the navigationSplitter is within a reasonable range, we use it.
	// If not, we reset it to the preferred width of the navigation tree.
	QScreen *screen = windowHandle()->screen();
	int storedNavSplitterWidth = (int)(SVSettings::instance().m_navigationSplitterSize / SVSettings::instance().m_ratio);
	int preferredNavSplitterWidth = m_navigationTreeWidget->sizeHint().width();
	if (storedNavSplitterWidth > preferredNavSplitterWidth && storedNavSplitterWidth < 1.5*preferredNavSplitterWidth)
		preferredNavSplitterWidth = storedNavSplitterWidth;
	QList<int> sizes;
	sizes << preferredNavSplitterWidth << screen->size().width() - preferredNavSplitterWidth;
	m_geometryViewSplitter->setSizes(sizes);

	// we want to limit the width, so buttons are always visible
//	m_navigationTreeWidget->setMinimumWidth(preferredNavSplitterWidth);
	onScreenChanged(screen);

	// *** Plugins ***
	setupPlugins();

	// *** Auto-Save Timer ***

	m_autoSaveTimer = new QTimer(this); // parent takes ownership
	// Connect time out to autosave
	connect(m_autoSaveTimer, &QTimer::timeout, &SVProjectHandler::instance(), &SVProjectHandler::onAutoSave);

	// apply settings
	onAutosaveSettingsChanged();

	m_ui->toolBar->setStyleSheet("QToolBar {spacing: 5px; padding: 5px; }");
}


void SVMainWindow::onAutosaveSettingsChanged() {
	FUNCID(SVMainWindow::onAutosaveSettingsChanged);
	if (SVSettings::instance().m_autosaveEnabled) {
		m_autoSaveTimer->start(SVSettings::instance().m_autosaveInterval*60*1000); // Mind: conversion to milliseconds
		// Note: time keeps autofiring every interval until stopped
		IBK::IBK_Message(IBK::FormatString("Autosave enabled, interval = %1 min\n").arg(SVSettings::instance().m_autosaveInterval), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	else {
		m_autoSaveTimer->stop();
		IBK::IBK_Message(IBK::FormatString("Autosave disabled\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
}


void SVMainWindow::onDockWidgetToggled(bool visible) {
	// get sender
	QAction * toggleAction = qobject_cast<QAction*>(sender());
	if (m_logDockWidget->toggleViewAction() == toggleAction) {
		m_dockWidgetVisibility[m_logDockWidget] = visible;
	}
}


void SVMainWindow::onImportPluginTriggered() {
	FUNCID(SVMainWindow::onImportPluginTriggered);

	QAction * a = qobject_cast<QAction *>(sender());
	if (a == nullptr) {
		IBK::IBK_Message("Invalid call to onImportPluginTriggered()", IBK::MSG_ERROR);
		return;
	}
	// retrieve plugin
	SVCommonPluginInterface * plugin = a->data().value<SVCommonPluginInterface *>();
	Q_ASSERT(plugin != nullptr);
	SVImportPluginInterface * importPlugin = dynamic_cast<SVImportPluginInterface *>(plugin);
	Q_ASSERT(importPlugin != nullptr);

	VICUS::Project p;
	QString projectText;
	bool success = importPlugin->import(this, projectText);

	ProgressNotifyer *notifyer = new ProgressNotifyer;
	notifyer->m_prgDlg = new QProgressDialog(QString(), QString(), 0, 100, this);
	notifyer->m_prgDlg->setWindowTitle(tr("Import project"));
	notifyer->m_prgDlg->setMinimumDuration(0);
	notifyer->notify(0, "");
	try {
		p.readImportedXML(projectText, dynamic_cast<IBK::NotificationHandler*>(notifyer));
//		std::ofstream out("C:/test/VicusImport.xml");
//		out << projectText.toStdString();
	}
	catch(IBK::Exception &ex) {
		success = false;
		// call of import plugin not successful
		QMessageBox::critical(this, "Plugin Error", tr("Error while importing a project with plugin '%1'\n%2")
							  .arg(importPlugin->title())
							  .arg(ex.what()), IBK::MSG_ERROR, *FUNC_ID);

		IBK::IBK_Message(IBK::FormatString("Error while importing a project with plugin '%1'")
						 .arg(importPlugin->title().toStdString()), IBK::MSG_ERROR);
	}

	if (success) {
		// if we have no project, yet, create a new project based on our imported data
		if (!m_projectHandler.isValid()) {
			// create new project
			m_projectHandler.newProject(&p); // emits updateActions()
//			m_projectHandler.project().writeXML(IBK::Path("g:\\temp\\VicusImport_clean.txt"));
		}
		else {

			// The merging of project and referenced data is a bit complicated.
			// First we must import the embedded database from the imported project
			// Then, we can copy the buildings to our project.
			m_projectHandler.importEmbeddedDB(p); // this might modify IDs of the imported project

			m_projectHandler.importProject(p);
			QTimer::singleShot(0, &SVViewStateHandler::instance(), &SVViewStateHandler::refreshColors);

		}
	}
	notifyer->notify(1, "Finished");
}


void SVMainWindow::onConfigurePluginTriggered() {
	QAction * a = qobject_cast<QAction *>(sender());
	if (a == nullptr) {
		IBK::IBK_Message("Invalid call to onConfigurePluginTriggered()", IBK::MSG_ERROR);
		return;
	}
	// retrieve plugin
	SVCommonPluginInterface * plugin = a->data().value<SVCommonPluginInterface *>();
	Q_ASSERT(plugin != nullptr);
	int updateNeeds = plugin->showSettingsDialog(this);
	if (updateNeeds & SVCommonPluginInterface::SceneUpdate) {
		// TODO : redraw scene
	}
	else if (updateNeeds & SVCommonPluginInterface::DatabaseUpdate) {
		// TODO : update property widgets
	}
}


void SVMainWindow::onScreenChanged(QScreen *screen) {
	// set minimum width of navigation tree, buttons shall be visible
//	m_navigationTreeWidget->setMinimumWidth(m_navigationTreeWidget->sizeHint().width());
	qDebug() << "Screen Changed: Device pixel ratio has been updated to: " << screen->devicePixelRatio();
	SVSettings::instance().m_ratio = screen->devicePixelRatio();
	// let others know (e.g. DB dialogs)
	emit screenHasChanged(screen);
}

SVStructuralUnitCreationDialog *SVMainWindow::structuralUnitDialog() {
	if (m_structuralUnitCreationDialog == nullptr)
		m_structuralUnitCreationDialog = new SVStructuralUnitCreationDialog(this);
	return m_structuralUnitCreationDialog;
}


void SVMainWindow::on_actionFileNew_triggered() {
	m_geometryView->switch2AddGeometry();
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// close project if we have one
	if (!m_projectHandler.closeProject(this)) // emits updateActions() if project was closed
		return;

	// create new project
	m_projectHandler.newProject(); // emits updateActions()
}


void SVMainWindow::on_actionFileOpen_triggered() {
	m_geometryView->switch2AddGeometry();
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
			.arg(SVSettings::instance().m_projectFileSuffix, SVSettings::instance().m_projectPackageSuffix), nullptr,
			SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
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
	// clear shift-keyboard state, since user has likely released shift key when using the open file dialog
	QKeyEvent * e = new QKeyEvent (QEvent::KeyRelease,Qt::Key_S,Qt::ShiftModifier,"s");
	qApp->postEvent((QObject*)m_geometryView->sceneView(),(QEvent *)e);
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


void SVMainWindow::on_actionEneryPlus_IDF_triggered() {
	// request IDF file and afterwards open import dialog
	QString filename = QFileDialog::getOpenFileName(
				this,
				tr("Select IDF file"),
				SVSettings::instance().m_propertyMap[SVSettings::PT_LastImportOpenDirectory].toString(),
			tr("EnergyPlus IDF files (*.idf);;All files (*.*)"), nullptr,
			SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
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

	SVSettings::instance().m_propertyMap[SVSettings::PT_LastImportOpenDirectory] = QDir(filename).absolutePath();
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
			SVProjectHandler::instance().importProject(m_importIDFDialog->m_importedProject);
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


void SVMainWindow::on_actionFileClose_triggered() {
	/* We dont know which project (network/building) will be opened afterwards.
	 * Also we need to hide transparent widgets (color legend, measurement widget).
	 */
	m_geometryView->switch2AddGeometry();
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	m_projectHandler.closeProject(this);
}


void SVMainWindow::on_actionFileExportProjectPackage_triggered() {
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
				tr("SIM-VICUS project packages (*.vicpac);;All files (*.*)"), nullptr,
				SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
																);

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


void SVMainWindow::on_actionFileQuit_triggered() {
	close();
}


void SVMainWindow::on_actionEditTextEditProject_triggered() {
	SVSettings::instance().openFileInTextEditor(this, m_projectHandler.projectFile());
}


void SVMainWindow::on_actionEditPreferences_triggered() {
	preferencesDialog()->edit(0); // changes are stored automatically.
}


void SVMainWindow::on_actionEditCleanProject_triggered() {
	QMessageBox::critical(this, QString(), tr("Not implemented, yet."));
//	// create a copy of the whole project
//	VICUS::Project cleanProject = SVProjectHandler::instance().project();

//	// TODO : Implement clean functionality

//	// create undo action and push it
//	SVUndoModifyProject * undo = new SVUndoModifyProject( tr("Removed unused definitions"), cleanProject );
//	undo->push();

}


void SVMainWindow::on_actionEditApplicationLog_triggered() {
	SVLogFileDialog dlg;
	dlg.setLogFile(QtExt::Directories::globalLogFile(), QString(), false);
	dlg.exec();
}


void SVMainWindow::on_actionSimulationCO2Balance_triggered() {
	if (m_coSimCO2VentilationDialog == nullptr)
		m_coSimCO2VentilationDialog = new SVCoSimCO2VentilationDialog(this);
	m_coSimCO2VentilationDialog->exec();
}


void SVMainWindow::on_actionViewShowSurfaceNormals_toggled(bool visible) {
	// set corresponding flag in View
	const_cast<Vic3D::SceneView*>(m_geometryView->sceneView())->setNormalVectorsVisible(visible);
}


void SVMainWindow::on_actionViewShowGrid_toggled(bool visible) {
	// set corresponding flag in View
	std::vector<VICUS::GridPlane> gridPlanes(project().m_viewSettings.m_gridPlanes);
	gridPlanes[0].m_isVisible = visible;
	SVUndoModifySiteData * undo = new SVUndoModifySiteData(tr("Main grid visibility changed"),
														   gridPlanes,
														   project().m_viewSettings.m_farDistance);
	undo->push();
}


void SVMainWindow::on_actionViewFindSelectedGeometry_triggered() {
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_FindSelection);
}


void SVMainWindow::on_actionViewResetView_triggered() {
	// set scene view to recenter its camera
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_Reset);
}


void SVMainWindow::on_actionViewFromNorth_triggered() {
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_North);
}



void SVMainWindow::on_actionViewFromEast_triggered() {
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_East);
}


void SVMainWindow::on_actionViewFromSouth_triggered() {
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_South);
}


void SVMainWindow::on_actionViewFromWest_triggered() {
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_West);
}


void SVMainWindow::on_actionViewFromAbove_triggered() {
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_Above);
}


void SVMainWindow::on_actionViewBirdsEyeViewSouthWest_triggered(){
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_BirdEyeSouthWest);
}


void SVMainWindow::on_actionViewBirdsEyeViewNorthWest_triggered(){
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_BirdEyeNorthWest);
}


void SVMainWindow::on_actionViewBirdsEyeViewSouthEast_triggered(){
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_BirdEyeSouthEast);
}


void SVMainWindow::on_actionViewBirdsEyeViewNorthEast_triggered(){
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_BirdEyeNorthEast);
}


void SVMainWindow::on_actionToolsCCMeditor_triggered() {

	QString ccmPath = SVSettings::instance().m_CCMEditorExecutable;
	if (ccmPath.isEmpty() || !QFileInfo::exists(ccmPath)) {
		QMessageBox::information(this, tr("Setup external tool"), tr("Please select first the path to the external "
																	 "climate editor in the preferences dialog!"));
		// spawn preferences dialog
		preferencesDialog()->edit(0);
		// still not ccm editor selected?
		ccmPath = SVSettings::instance().m_CCMEditorExecutable;
		if (ccmPath.isEmpty() || !QFileInfo::exists(ccmPath))
			return;
	}
	bool res = QProcess::startDetached(ccmPath, QStringList(), QString());
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


void SVMainWindow::on_actionHelpVisitDiscussionForum_triggered() {
	QDesktopServices::openUrl( QUrl(FORUM_URL));
}


void SVMainWindow::on_actionHelpCheckForUpdates_triggered() {
	// TODO :
}


void SVMainWindow::on_actionHelpOnlineManual_triggered() {
	QDesktopServices::openUrl( QUrl(MANUAL_URL));
}


void SVMainWindow::on_actionHelpKeyboardAndMouseControls_triggered() {
	// show keyboard/mouse control cheat sheet
	QDialog dlg(this);
	QVBoxLayout * lay = new QVBoxLayout(&dlg);
	QTextEdit * w = new QTextEdit(&dlg);
	w->setReadOnly(true);
	lay->addWidget(w);
	dlg.setLayout(lay);
	QString shortRefFile;
	if (QtExt::LanguageHandler::instance().langId() == "de") {
		shortRefFile = ":/doc/KeyboardMouseControls.html.de";
	}
	else {
		shortRefFile = ":/doc/KeyboardMouseControls.html";
	}
	QFile manualFile(shortRefFile);
	manualFile.open(QFile::ReadOnly);
	QString manual = manualFile.readAll();
	w->setHtml(manual);
	dlg.resize(1400,800);
	dlg.exec();
}


void SVMainWindow::on_actionHelpLinuxDesktopIntegration_triggered() {
#ifdef IBK_DEPLOYMENT
	QString iconLocation = QtExt::Directories::resourcesRootDir();
#else
	QString iconLocation = QtExt::Directories::resourcesRootDir() + "/logo/icons";
#endif

	SVSettings::linuxDesktopIntegration(this,
										iconLocation,
										"SIM-VICUS",
										"simvicus",
										"Building Energy Performance and District Simulation",
										SVSettings::instance().m_installDir + "/SIM-VICUS",
										"vicus"
										);
}


// *** other slots (not main menu slots) ***

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


void SVMainWindow::on_actionNetwork_GIS_data_triggered() {
	// opens import network dialog
	if (m_networkImportDialog == nullptr)
		m_networkImportDialog = new SVNetworkImportDialog(this);

	m_networkImportDialog->edit();
}


void SVMainWindow::on_actionEditProjectNotes_triggered() {
	// create on first use
	if (m_notesDialog == nullptr)
		m_notesDialog = new SVNotesDialog(this);

	m_notesDialog->exec();
}


void SVMainWindow::on_actionPluginsManager_triggered() {
	// show plugin view

}


void SVMainWindow::on_actionExportNetworkAsGeoJSON_triggered() {
	// opens export network dialog
	if (m_networkExportDialog == nullptr)
		m_networkExportDialog = new SVNetworkExportDialog(this);

	m_networkExportDialog->edit();
}



void SVMainWindow::onUpdateActions() {
	// purpose of this function is to update the view layout based on the existance of a project or none

	// do we have a project?
	bool have_project = m_projectHandler.isValid();
	// enable/disable all actions that require a project

	// *** Project-dependent actions ***

	m_ui->actionFileSave->setEnabled(have_project);
	m_ui->actionFileSaveAs->setEnabled(have_project);
	m_ui->actionFileReload->setEnabled(have_project);
	m_ui->actionEditProjectNotes->setEnabled(have_project);
	m_ui->actionFileClose->setEnabled(have_project);
	m_ui->actionFileExportProjectPackage->setEnabled(have_project);
	m_ui->actionExportNetworkAsGeoJSON->setEnabled(have_project);
	m_ui->actionFileOpenProjectDir->setEnabled(have_project);

	m_ui->actionEditTextEditProject->setEnabled(have_project);
	m_ui->actionEditCleanProject->setEnabled(have_project);

	m_ui->actionViewFindSelectedGeometry->setEnabled(have_project);
	m_ui->actionViewResetView->setEnabled(have_project);
	m_ui->actionViewShowSurfaceNormals->setEnabled(have_project);
	m_ui->actionViewShowGrid->setEnabled(have_project);
	m_ui->actionViewFromNorth->setEnabled(have_project);
	m_ui->actionViewFromEast->setEnabled(have_project);
	m_ui->actionViewFromSouth->setEnabled(have_project);
	m_ui->actionViewFromWest->setEnabled(have_project);
	m_ui->actionViewFromAbove->setEnabled(have_project);
	m_ui->actionViewBirdsEyeViewNorthEast->setEnabled(have_project);
	m_ui->actionViewBirdsEyeViewNorthWest->setEnabled(have_project);
	m_ui->actionViewBirdsEyeViewSouthEast->setEnabled(have_project);
	m_ui->actionViewBirdsEyeViewSouthWest->setEnabled(have_project);

	m_ui->actionSimulationCO2Balance->setEnabled(have_project);


	// *** View configuration ***

	// navigation bar is only visible when we have a project
	m_ui->toolBar->setVisible(have_project);
	m_ui->toolBar->setEnabled(have_project);
	m_ui->toolBar->toggleViewAction()->setEnabled(have_project);


	// We dont have NO PROJECT
	if (!have_project) {

		// no project, no undo actions -> clearing undostack also disables undo actions
		m_undoStack->clear();

		// turn off main view first when project is gone
		m_mainViewMode = MV_None;
		updateMainView();

		// Dock-Widgets are only visible when project is there
		m_logDockWidget->toggleViewAction()->setEnabled(false);
		m_logDockWidget->toggleViewAction()->blockSignals(true);
		m_logDockWidget->setVisible(false);
		m_logDockWidget->toggleViewAction()->blockSignals(false);

		// window caption
		setWindowTitle(QString("SIM-VICUS %1").arg(VICUS::LONG_VERSION));

		// if no recent projects set welcome page to demo files
		if (SVSettings::instance().m_recentProjects.empty())
			m_welcomeScreen->setPageType(SVWelcomeScreen::PT_DemoFiles);
		m_welcomeScreen->updateWelcomePage();
	}

	// when we have A PROJECT
	else {

		updateWindowTitle();

		if (m_mainViewMode == MV_None)
			m_mainViewMode = MV_GeometryView;
		updateMainView();

		// select the current view, this also enables (in case of ConstructionView) the visibility of the dock widgets
		m_logDockWidget->setVisible(m_dockWidgetVisibility[m_logDockWidget]);
		m_logDockWidget->toggleViewAction()->setEnabled(true);

		bool isVisible = true;
		if (project().m_viewSettings.m_gridPlanes.size() > 0)
			isVisible = project().m_viewSettings.m_gridPlanes[0].m_isVisible;
		m_ui->actionViewShowGrid->setChecked(isVisible);
	}
}


void SVMainWindow::updateMainView() {

	// uncheck all button bar actions
	m_ui->actionGeometryView->setChecked(false);
	m_ui->actionSimulationSettings->setChecked(false);

	switch (m_mainViewMode) {
		case MV_None: {
			m_geometryViewSplitter->setVisible(false);
			m_simulationSettingsView->setVisible(false);
			m_welcomeScreen->setVisible(true);
		} break;
		case MV_GeometryView: {
			m_welcomeScreen->setVisible(false);
			m_simulationSettingsView->setVisible(false);
			m_geometryViewSplitter->setVisible(true);
			m_ui->actionGeometryView->setChecked(true);
			m_geometryView->setFocus();
		} break;
		case MV_SimulationView: {
			m_welcomeScreen->setVisible(false);
			m_geometryViewSplitter->setVisible(false);
			m_simulationSettingsView->setVisible(true);
			m_ui->actionSimulationSettings->setChecked(true);
		} break;
	}

	// toggle visibility and update position of floating transparent widgets (color legend, measurement, snap options)
	SVViewStateHandler::instance().toggleTransparentWidgetsVisibility(m_mainViewMode);
	SVViewStateHandler::instance().m_geometryView->moveTransparentSceneWidgets();
}


void SVMainWindow::on_actionGeometryView_triggered() {
	m_mainViewMode = MV_GeometryView;
	updateMainView();
}


void SVMainWindow::on_actionSimulationSettings_triggered() {
	m_mainViewMode = MV_SimulationView;
	updateMainView();
}



void SVMainWindow::onUpdateRecentProjects() {
	// create actions for recent files if number of max. recent projects in settings
	// differs from current number
	if (m_recentProjectActions.count() != (int)SVSettings::instance().m_maxRecentProjects) {
		qDeleteAll(m_recentProjectActions);
		m_recentProjectActions.clear();
		for (int i = 0; i < (int)SVSettings::instance().m_maxRecentProjects; ++i) {
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
		for (int i = 0, count = SVSettings::instance().m_recentProjects.count(); i < count; ++i) {
			/// \bug Fix underscore/whitespace display in menu action
			m_recentProjectActions[i]->setText(SVSettings::instance().m_recentProjects[i]);
			m_recentProjectActions[i]->setData(SVSettings::instance().m_recentProjects[i]);
			m_recentProjectActions[i]->setVisible(true);
		}

		for (int i = SVSettings::instance().m_recentProjects.count(); i < (int)SVSettings::instance().m_maxRecentProjects; ++i) {
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
	QFileInfo finfo(filename);
	if (!finfo.exists()) {
		QMessageBox::critical(this, tr("File not found"), tr("The file '%1' cannot be found or does not exist.").arg(filename));
		return;
	}

	// soruce and target dirs
	QDir srcDir = finfo.absoluteDir();
	QDir targetDir(QtExt::Directories::userExampleFileDir());

	// create targetDir, if required
	if (!targetDir.exists())
		targetDir.mkpath(".");

	// copy target file, ask permission if existing
	QString targetFile = targetDir.absoluteFilePath(finfo.fileName());
	if (QFile::exists(targetFile)) {
		QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Overwrite example"), tr("Example project exists already user directory, overwrite it?"), QMessageBox::Yes|QMessageBox::No);
		if (reply == QMessageBox::No)
			return;
		else
			if (!QFile::remove(targetFile))
				QMessageBox::critical(this, tr("Write error"), tr("Unable to overwrite example file, maybe missing permissions?"));
	}
	QFile::copy(filename, targetFile);

	// now also copy the "DataFiles" directory needed for network examples, we don't ask permission for that
	srcDir.cd("DataFiles");
	targetDir.mkpath("DataFiles");
	targetDir.cd("DataFiles");
	if (srcDir.exists()) {
		if (!copyRecursively(srcDir.absolutePath(), targetDir.absolutePath())) {
			QMessageBox::critical(this, tr("Write error"), tr("Cannot copy example, maybe missing permissions?"));
			return;
		}
	}

	// and finally open the copied project file and hope for the best
	onOpenProjectByFilename(targetFile);
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

	/// \todo Andreas: Figure out why thread sends finished signal twice when terminated
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
	FUNCID(SVMainWindow::onFixProjectAfterRead);

	// project has been read - if we have a nandrad-export specified, run the automatic nandrad generation
	if (!SVSettings::instance().m_nandradExportFileName.isEmpty()) {

		QStringList errorStack;

		VICUS::Project localProject(project());
		localProject.updatePointers();
		SVSettings::instance().m_db.updateEmbeddedDatabase(localProject);
		try {
			NANDRAD::Project p;
			// add default placeholders
			p.m_placeholders[VICUS::DATABASE_PLACEHOLDER_NAME] = IBK::Path((QtExt::Directories::databasesDir()).toStdString());
			p.m_placeholders[VICUS::USER_DATABASE_PLACEHOLDER_NAME] = IBK::Path((QtExt::Directories::userDataDir()).toStdString());
			// "Project Directory" placeholder is needed to resolve paths to files referenced via relative paths
			p.m_placeholders["Project Directory"] = IBK::Path(SVSettings::instance().m_nandradExportFileName.toStdString()).parentPath().str();
			project().generateNandradProject(p, errorStack, SVSettings::instance().m_nandradExportFileName.toStdString());
			// save project
			IBK::Path targetNandradFile(SVSettings::instance().m_nandradExportFileName.toStdString());
			p.writeXML(targetNandradFile);

			IBK::IBK_Message( IBK::FormatString("NANDRAD project file '%1' generated.\n").arg(targetNandradFile.absolutePath()), IBK::MSG_PROGRESS, FUNC_ID);
		}
		catch (IBK::Exception & ex) {
			for (const QString & s : errorStack)
				IBK::IBK_Message(s.toStdString(), IBK::MSG_ERROR, FUNC_ID);
			// just show a generic error message
			ex.writeMsgStackToError();
			IBK::IBK_Message("An error occurred during NANDRAD project generation.", IBK::MSG_ERROR, FUNC_ID);
			exit(1);
		}
		exit(0); // bail out successfully
	}

	// here we do all entry checks that will tell users about suggested changes in project

	std::vector<std::vector<SVDatabase::DuplicateInfo> > dups;
	SVSettings::instance().m_db.determineDuplicates(dups);
	// any duplicates?
	bool haveDups = false;
	for (const std::vector<SVDatabase::DuplicateInfo> & v : dups)
		if (!v.empty()) {
			haveDups = true;
			break;
		}
	if (haveDups) {
		int res = SVSettings::instance().showDoNotShowAgainQuestion(this, "RemoveDuplicatesAfterProjectRead",
																	tr("Remove/merge duplicates in database"),
																	tr("The database contains now some duplicate definitions. Do you want "
																	   "to review them and remove unnecessary duplicates (you can also do this later via Databases|Remove duplicates...)?"), QMessageBox::Yes | QMessageBox::No);
		if (res == QMessageBox::Yes)
			on_actionDBRemoveDuplicates_triggered();
	}
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
	m_geometryView->m_focusRootWidgets.insert(m_logDockWidget); // remember as possible focus widget for events
	m_logDockWidget->setObjectName("LogDockWidget");
	m_logDockWidget->setContentsMargins(0,0,0,0);
	m_logDockWidget->setWindowTitle(tr("Application Log"));
	m_logDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures | titleBarOption);
	m_logDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
	QAction * toggleAction = m_logDockWidget->toggleViewAction();
	m_ui->menuView->addAction(toggleAction);
	connect(toggleAction, &QAction::toggled, this, &SVMainWindow::onDockWidgetToggled);
	m_logDockWidget->setWidget(m_logWidget);
	addDockWidget(Qt::BottomDockWidgetArea,m_logDockWidget);

	//	tabifyDockWidget(m_outputListDockWidget, m_outputGridListDockWidget);
}


void SVMainWindow::setupPlugins() {
	m_pluginLoader->loadPlugins();

//	const auto staticInstances = QPluginLoader::staticInstances();
	for (auto pldata = m_pluginLoader->m_plugins.begin(); pldata != m_pluginLoader->m_plugins.end(); ++pldata) {
		QObject* pl = pldata->m_loader->instance();
		setupPluginMenuEntries(pl);
	}
}


void SVMainWindow::setupPluginMenuEntries(QObject * plugin) {
	FUNCID(SVMainWindow::setupPluginMenuEntries);
	// depending on the implemented interface, do different stuff
	SVImportPluginInterface* importPlugin = dynamic_cast<SVImportPluginInterface*>(plugin);
	if (importPlugin != nullptr) {
		IBK::IBK_Message(IBK::FormatString("  Adding importer plugin '%1'\n").arg(importPlugin->title().toStdString()),
						 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// add a menu action into the import menu
		QAction * a = new QAction(importPlugin->importMenuCaption(), this);
		connect(a, &QAction::triggered,
				this, &SVMainWindow::onImportPluginTriggered);
		QVariant v;
		v.setValue<SVCommonPluginInterface*>(importPlugin);
		a->setData(v);
		m_ui->menuImport->addAction(a); // transfers ownership
		// if plugin publishes settings action, also add plugin configuration action
		if (importPlugin->hasSettingsDialog()) {
			QAction * a = new QAction(tr("Configure %1").arg(importPlugin->title()), this);
			QVariant v;
			v.setValue<SVCommonPluginInterface*>(importPlugin);
			a->setData(v);
			connect(a, &QAction::triggered,
					this, &SVMainWindow::onConfigurePluginTriggered);
			m_ui->menuPlugins->addAction(a); // transfers ownership
		}
	}

	// database plugins
	SVDatabasePluginInterface* dbPlugin = dynamic_cast<SVDatabasePluginInterface*>(plugin);
	if (dbPlugin != nullptr) {
		// create copy of database
		SVDatabase addedDB;
		// update database entries
		if (dbPlugin->retrieve(SVSettings::instance().m_db, addedDB)) {
			IBK::IBK_Message(IBK::FormatString("  Database plugin '%1' added\n").arg(dbPlugin->title().toStdString()),
							 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

			// process all DB elements in the second DB and transfer those into our db, but only if we do not have
			// conflicting IDs


		}
		else {
			IBK::IBK_Message(IBK::FormatString("  Error importing database entries from plugin '%1'").arg(dbPlugin->title().toStdString()),
							 IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
		}
		// settings?
		if (dbPlugin->hasSettingsDialog()) {
			QAction * a = new QAction(tr("Configure %1").arg(dbPlugin->title()), this);
			QVariant v;
			v.setValue<SVCommonPluginInterface*>(dbPlugin);
			a->setData(v);
			connect(a, &QAction::triggered,
					this, &SVMainWindow::onConfigurePluginTriggered);
			m_ui->menuPlugins->addAction(a); // transfers ownership
		}
	}
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
	setWindowTitle(QString("SIM-VICUS %1 - %2").arg(VICUS::LONG_VERSION).arg(shortFileName));
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
			if (isPackage && fname.endsWith(".vicus")) {
				projectFilePath = fname;
				break;
			}
			else if (!isPackage && fname.endsWith(".vicus")) {
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
	// thumb name is full filepath with replaces / \ and :
	QString thumbName = m_projectHandler.projectFile();
	thumbName.replace("/", "_");
	thumbName.replace("\\", "_");
	thumbName.replace(":", "_");
	thumbName.replace(".vicus", "");
	QString thumbPath = QtExt::Directories::userDataDir()  + "/thumbs/" + thumbName + ".png";
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
	QString languageFilename = QtExt::Directories::translationsFilePath(langId);
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
		IBK::IBK_Message( IBK::FormatString("Language file '%1' missing.").arg(languageFilename.toStdString()),
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
						tr("SIM-VICUS project files (%1);;All files (*.*)").arg(SVSettings::instance().m_projectFileSuffix), nullptr,
						SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
																		);
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
						recentPath,
						SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
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


bool SVMainWindow::exportProjectCopy(QString targetDirPath, const VICUS::Project & project) {
	// TODO : implement project export
	//
	// - create a temporary project copy
	// - copy climate data file to target directory
	// - adjust path to climate data file (like ${Project Directory}/climateFilePath.c6b )
	// - copy all externally referenced files (are there any?)
	// - save modified project
	(void)targetDirPath;
	(void)project;

	return true;
}


SVPreferencesDialog * SVMainWindow::preferencesDialog() {
	if (m_preferencesDialog == nullptr) {
		m_preferencesDialog = new SVPreferencesDialog(this);
		connect(preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged,
				m_welcomeScreen, &SVWelcomeScreen::onStyleChanged);
		connect(m_preferencesDialog->pageStyle(), &SVPreferencesPageStyle::styleChanged,
				m_geometryView->sceneView(), &Vic3D::SceneView::onStyleChanged);
		connect(m_preferencesDialog->pageMisc(), &SVPreferencesPageMisc::autosaveSettingsChanged,
				this, &SVMainWindow::onAutosaveSettingsChanged);

		connect(preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged,
				m_navigationTreeWidget, &SVNavigationTreeWidget::onStyleChanged);
	}
	return m_preferencesDialog;
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

void SVMainWindow::onShortCutStartSimulation() {
	on_actionSimulationSettings_triggered();
	m_simulationSettingsView->setCurrentPage(2); // page 2 is simulation start
}


void SVMainWindow::on_actionOpenPostProcessing_triggered() {
	// configure PostProc session, save parallel to project and open session in post

	if (SVSettings::instance().m_postProcExecutable.isEmpty() ||
			!QFileInfo::exists(SVSettings::instance().m_postProcExecutable))
	{
		QMessageBox::information(this, tr("Setup external tool"), tr("Please select first the path to the external "
																	 "post processing in the preferences dialog!"));
		// spawn preferences dialog
		preferencesDialog()->edit(0);
		// still not postproc selected?
		if (SVSettings::instance().m_postProcExecutable.isEmpty() || !QFileInfo::exists(SVSettings::instance().m_postProcExecutable))
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
														   QString("*.p2"), nullptr,
														   SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
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

void SVMainWindow::on_actionEPDElements_triggered() {
	dbEpdEditDialog()->edit();
}


void SVMainWindow::on_actionExternal_Post_Processor_triggered() {
	on_actionOpenPostProcessing_triggered();
}


void SVMainWindow::on_actionDWD_Weather_Data_Converter_triggered() {
	QString dwdPath = SVSettings::instance().m_DWDConverterExecutable;
	if (dwdPath.isEmpty() || !QFileInfo::exists(dwdPath)) {
		QMessageBox::information(this, tr("Setup external tool"), tr("Please select first the path to the DWD "
																	 "weather downloader in the preferences dialog!"));
		// spawn preferences dialog
		preferencesDialog()->edit(0);
		// still not ccm editor selected?
		dwdPath = SVSettings::instance().m_DWDConverterExecutable;
		if (dwdPath.isEmpty() || !QFileInfo::exists(dwdPath))
			return;
	}
	bool res = QProcess::startDetached(dwdPath, QStringList(), QString());
	if (!res) {
		QMessageBox::critical(this, tr("Error starting external application"), tr("DWD weather downloader '%1' could not be started.")
							  .arg(dwdPath));
	}
}


void SVMainWindow::on_actionDBAcousticBoundaryConditions_triggered() {
	dbAcousticBoundaryConditionEditDialog()->edit();
}


void SVMainWindow::on_actionDBAcousticSoundAbsorptions_triggered() {
	dbAcousticSoundAbsorptionEditDialog()->edit();
}





