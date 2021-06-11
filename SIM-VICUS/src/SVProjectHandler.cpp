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

#include "SVProjectHandler.h"

#include <QStringList>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QWizard>
#include <QTextEdit>
#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QTimer>

#include <IBK_Exception.h>
#include <IBK_FileUtils.h>
#include <IBK_Path.h>
#include <IBK_assert.h>

#include <QtExt_Directories.h>
#include <QtExt_configuration.h>

#include <VICUS_Project.h>

#include "SVSettings.h"
#include "SVConstants.h"
#include "SVLogFileDialog.h"
#include "SVViewStateHandler.h"

SVProjectHandler * SVProjectHandler::m_self = nullptr;

ModificationInfo::~ModificationInfo() {
}

SVProjectHandler & SVProjectHandler::instance() {
	Q_ASSERT_X(m_self != nullptr, "[SVProjectHandler::instance]",
		"You must not access SVProjectHandler::instance() when the is no SVProjectHandler "
		"instance (anylonger).");
	return *m_self;
}


SVProjectHandler::SVProjectHandler() :
	m_project(nullptr),
	m_modified(false)
{
	IBK_ASSERT(m_self == nullptr);
	m_self = this;
}


SVProjectHandler::~SVProjectHandler( ){
	// free owned project, if any
	delete m_project;
	m_self = nullptr;
}


QString SVProjectHandler::nandradProjectFilePath() const {
	QString nandradProjectFilePath = QFileInfo(SVProjectHandler::instance().projectFile()).completeBaseName() + ".nandrad";
	return QFileInfo(SVProjectHandler::instance().projectFile()).dir().filePath(nandradProjectFilePath);
}


bool SVProjectHandler::newProject(VICUS::Project * project) {
	createProject();
	if (project != nullptr) {
		*m_project = *project; // copy over project
		// update all internal pointers
		m_project->updatePointers();
	}

	// initialize viewstate
	SVViewState vs = SVViewStateHandler::instance().viewState();
	SVViewStateHandler::instance().setViewState(vs);

	setModified(AllModified);

	// signal UI that we now have a project
	emit updateActions();

	return true;
}


bool SVProjectHandler::closeProject(QWidget * parent) {

	// if no project exists, simply return true
	if (!isValid())
		return true;

	// ask user for confirmation to save, if project was modified
	if (isModified()) {

		// ask user for confirmation to save
		int result = QMessageBox::question(
				parent,
				tr("Save project before closing"),
				tr("Would you like to save the project before closing it?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
				);

		// user bails out?
		if (result == QMessageBox::Cancel)
			return false; // project not closed

		// saving requested by user
		if (result == QMessageBox::Save) {

			SaveResult res;

			// let user pick new filename
			if (m_projectFile.isEmpty())
				res = saveWithNewFilename(parent);
			else
				res = saveProject(parent, m_projectFile);

			// saving failed ?
			if (res != SaveOK)
				return false;

		}

	} // if (isModified())

	// saving succeeded, now we can close the project
	destroyProject();

	// signal application that we have no longer a project, and thus all project-related
	// actions should be disabled
	emit updateActions();

	return true;
}


void SVProjectHandler::loadProject(QWidget * parent, const QString & fileName,	bool silent) {
	FUNCID(SVProjectHandler::loadProject);

	// we must not have a project loaded
	IBK_ASSERT(!isValid());

	do {
		m_reload = false;

		// create a new project
		createProject();

		try {
			if (!read(fileName))
				throw IBK::Exception(IBK::FormatString("Error reading project '%1'").arg(fileName.toUtf8().data()), FUNC_ID);
			// project read successfully

			// run sanity checks - basically check for invalid user-given parameters that might crash the UI
#if 0
			m_project->GUISanityChecks();
#endif
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			if (!silent) {

				QMessageBox::critical(
							parent,
							tr("Error loading project"),
							tr("Error loading project file '%1', see error log file '%2' for details.")
							.arg(fileName)
							.arg(QtExt::Directories::globalLogFile())
							);

				SVLogFileDialog dlg;
				dlg.setLogFile(QtExt::Directories::globalLogFile(), fileName, true);
				dlg.exec();
			}
			// remove project again
			destroyProject();

			// Note: no need to emit updateActions() here since view state hasn't finished.
			if( !m_reload)
				return;
		}
	} while(m_reload);


	// first tell project and thus all connected views that the
	// structure of the project has changed
	try {
		bool have_modified_project = false;

		have_modified_project = importEmbeddedDB();

		/// \todo Hauke, check uniqueness of IDs in networks

		/// \todo Andreas: implement and project data checks with automatic fixes and
		///		  set have_modified_project to true
		///		  in such cases, so that the project starts up in "modified" state.

		setModified(AllModified); // notify all views that the entire data has changed

		// this will clear the modified flag again (since we just read the project) except if we had made some automatic
		// fixes above
		m_modified = have_modified_project;

		emit updateActions();
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();

		// project data was incomplete, we show an error message and default to empty project
		if (!silent) {
			QMessageBox::critical(
					parent,
					tr("Error loading project"),
					tr("Data in project was missing/invalid, see error log '%1' for details.").arg(QtExt::Directories::globalLogFile())
			);
#if 0
			SVLogFileDialog dlg;
			dlg.setLogFile(QtExt::Directories::globalLogFile(), fileName, true);
			dlg.exec();
#endif
		}
		// remove project again
		destroyProject();
		return;
	}

	// If we have read an old project file, the fileName and project().projectFile
	// will be different, because the extension was changed. In this case
	// we leave the modification state to modified and do not add the file to the
	// recent file list.

	// if the filenames are the same, we can savely assume that the project is not
	// modified and we add the file to the recent file list
	if (fileName == m_projectFile) {
		// add project file name to recent file list
		addToRecentFiles(fileName);
	} // if (fileName == m_projectFile)


	// issue a call to user-dialog fixes/adjustments
	QTimer::singleShot(0, this, SIGNAL(fixProjectAfterRead()));
}


void SVProjectHandler::reloadProject(QWidget * parent) {
	QString projectFileName = projectFile();
	m_modified = false; // so that closeProject doesn't ask questions
	closeProject(parent);
	loadProject(parent, projectFileName, false); // emits updateActions() if project was successfully loaded
}


SVProjectHandler::SaveResult SVProjectHandler::saveWithNewFilename(QWidget * parent) {

	// determine default path from current project file
	QString currentPath = QFileInfo(m_projectFile).filePath();

	// ask user for filename
	QString filename = QFileDialog::getSaveFileName(
			parent,
			tr("Specify SIM-VICUS project file"),
			currentPath,
			tr("SIM-VICUS project files (*%1);;All files (*.*)").arg(SVSettings::instance().m_projectFileSuffix),
			nullptr
#ifdef QTEXT_DONT_USE_NATIVE_FILEDIALOG
			,QFileDialog::DontUseNativeDialog
#endif // QTEXT_DONT_USE_NATIVE_FILEDIALOG
		);


	if (filename.isEmpty()) return SaveCancelled; // cancelled

	QString fnamebase = QFileInfo(filename).baseName();
	if (fnamebase.isEmpty()) {
		QMessageBox::critical(parent, tr("Invalid file name"), tr("Please enter a valid file name!"));
		return SaveCancelled;
	}

	// relay to saveProject() which updates modified flag and emits corresponding signals.
	if (saveProject(parent, filename) != SaveOK)
		return SaveFailed; // saving failed

	return SaveOK;
}


SVProjectHandler::SaveResult SVProjectHandler::saveProject(QWidget * parent, const QString & fileName, bool addToRecentFilesList) {

	// check project file ending, if there is none append it
	QString fname = fileName;
	if (!fname.endsWith(SVSettings::instance().m_projectFileSuffix))
		fname.append( SVSettings::instance().m_projectFileSuffix );

	// updated created and lastEdited tags
	if (m_project->m_projectInfo.m_created.empty())
		m_project->m_projectInfo.m_created = QDateTime::currentDateTime().toString(Qt::TextDate).toUtf8().data();
	m_project->m_projectInfo.m_lastEdited = QDateTime::currentDateTime().toString(Qt::TextDate).toUtf8().data();

	// update standard placeholders in project file
	m_project->m_placeholders[VICUS::DATABASE_PLACEHOLDER_NAME]			= QtExt::Directories::databasesDir().toStdString();
	m_project->m_placeholders[VICUS::USER_DATABASE_PLACEHOLDER_NAME]	= QtExt::Directories::userDataDir().toStdString();

	// update embedded database
	SVSettings::instance().m_db.updateEmbeddedDatabase(*m_project);

	// save project file
	if (!write(fname)) {

		QMessageBox::critical(
				parent,
				tr("Saving failed"),
				tr("Error while saving project file, see error log file '%1' for details.").arg(QtExt::Directories::globalLogFile())
				);

		return SaveFailed;
	}

	// clear modified flag
	m_modified = false;
	// signal UI to update project status
	emit updateActions();

	// add project file name to recent file list
	if (addToRecentFilesList)
		addToRecentFiles(fname);

	return SaveOK; // saving succeeded
}


void SVProjectHandler::setModified(int modificationType, ModificationInfo * data) {
	// special case:  modification type = NotModified
	//ModificationTypes modType = static_cast<ModificationTypes>(modificationType);
	//switch (modType) {

	//	default: ; // skip all others
	//}
	m_modified = true;

	emit modified(modificationType, data);
	emit updateActions();
}


const VICUS::Project & SVProjectHandler::project() const {
	FUNCID(SVProjectHandler::project);

	if (m_project == nullptr)
		throw IBK::Exception("Must not call project() on invalid ProjectHandler.", FUNC_ID);
	return *m_project;
}


void SVProjectHandler::updateLastReadTime() {
	FUNCID(SVProjectHandler::updateLastReadTime);
	if (!isValid())
		throw IBK::Exception("Must not call updateLastReadTime() on invalid project.", FUNC_ID);
	m_lastReadTime = QFileInfo(projectFile()).lastModified().addSecs(5);
}


IBK::Path SVProjectHandler::replacePathPlaceholders(const IBK::Path & stringWithPlaceholders) {
	std::map<std::string, IBK::Path> mergedPlaceholders; // = project().m_placeholders;
	/// \todo All: discuss placeholder handling in SIM-VICUS


	if (!projectFile().isEmpty())
		mergedPlaceholders["Project Directory"] =
				QFileInfo(projectFile()).absoluteDir().absolutePath().toUtf8().data();

	IBK::Path newPath( stringWithPlaceholders );

	// replace all placeholders
	if ( stringWithPlaceholders.hasPlaceholder() ) {
		newPath = stringWithPlaceholders.withReplacedPlaceholders( mergedPlaceholders );
	}

	return newPath;
}


VICUS::ViewSettings & SVProjectHandler::viewSettings() {
	if (m_project == nullptr) {
		Q_ASSERT(m_project != nullptr);
	}

	// We bypass the undo/redo action settings because view settings (camera position, grid color, grid spacing)
	// are properties of the project, yet outside the undo/redo system. It would feel unnatural to "redo" a camera
	// position change.
	return m_project->m_viewSettings;
}


// *** PRIVATE MEMBER FUNCTIONS ***

void SVProjectHandler::createProject() {
	Q_ASSERT(m_project == nullptr);

	m_project = new VICUS::Project;
	m_projectFile.clear();
	m_modified = false; // new projects are never modified
}


void SVProjectHandler::destroyProject() {
	Q_ASSERT(m_project != nullptr);

	delete m_project;
	m_project = nullptr;
	m_projectFile.clear();
}


bool SVProjectHandler::read(const QString & fname) {
	FUNCID(SVProjectHandler::read);

	// check that we have a project, should be newly created
	Q_ASSERT(isValid());

	if (!QFileInfo(fname).exists()) {
		IBK::IBK_Message(IBK::FormatString("File '%1' does not exist or permissions are missing for accessing the file.")
						 .arg(fname.toUtf8().data()), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	try {

		// filename is converted to utf8 before calling readXML
		m_project->readXML(IBK::Path(fname.toStdString()) );
		m_projectFile = fname;

		m_lastReadTime = QFileInfo(fname).lastModified();

		//update the colors
		//if project has invalid colors nothing is drawn ...
		updateSurfaceColors();

		// after reading the project file, we should update the views
		// this is done in a subsequent call to setModified() from the calling function
		return true;
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
	}
	catch (std::exception & ex) {
		// this shouldn't happen, unless we have something weird going on
		IBK::IBK_Message(IBK::FormatString("std::exception caught: %1").arg(ex.what()), IBK::MSG_ERROR, FUNC_ID);
	}

	return false;
}


bool SVProjectHandler::write(const QString & fname) const {
	FUNCID(SVProjectHandler::write);
	Q_ASSERT(isValid());

	// if project file exists, create backup file
	if (QFile(fname).exists()) {
		// if backup file exists already, delete it
		bool fileExists = QFile(fname + ".bak").exists();
		if (fileExists) {
			// try to delete the backup file
			if (QFile(fname + ".bak").remove())
				fileExists = false;
		}
		if (!fileExists && !QFile(fname).copy(fname + ".bak"))
			IBK::IBK_Message(IBK::FormatString("Cannot create backup file '%1' (path does not exists or missing permissions).")
							 .arg((fname + ".bak").toUtf8().data()), IBK::MSG_ERROR, FUNC_ID);
	}

	// create file
	QFile file(fname);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		IBK::IBK_Message(IBK::FormatString("Cannot create/write file '%1' (path does not exists or missing permissions).")
						 .arg(fname.toUtf8().data()), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}
	file.close();

	try {
		IBK::Path filename(fname.toUtf8().data());
		IBK::Path newProjectDir = filename.parentPath();
		IBK::Path oldProjectDir(m_projectFile.toUtf8().data());
		std::map<std::string, IBK::Path> pmap;
		try {
			oldProjectDir = oldProjectDir.parentPath();
			pmap["Project Directory"] = oldProjectDir;
		}
		catch (...) {
			// invalid old path -> we shouldn't have "Project Directory" placeholder in this case...
		}


		// filename is converted to utf8 before calling writeXML
		m_project->writeXML(IBK::Path(fname.toStdString()) );

		// also set the project file name
		m_projectFile = fname;
		*const_cast<QDateTime*>(&m_lastReadTime) = QFileInfo(fname).lastModified();

		// remove backup file again
		QFile(fname + ".bak").remove();

		return true;
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		// restore file from backup file, but keep backup file just in case
		QFile(fname + ".bak").copy(fname);
	}
	catch (std::exception & ex) {
		// this shouldn't happen, unless we have something weird going on
		IBK::IBK_Message(IBK::FormatString("std::exception caught: %1").arg(ex.what()), IBK::MSG_ERROR, FUNC_ID);
	}
	return false;
}



void SVProjectHandler::addToRecentFiles(const QString& fname) {

	SVSettings & si = SVSettings::instance();
	//qDebug() << si.m_recentProjects;

	// compose absolute file name
	QFileInfo finfo(fname);
	QString filePath =  finfo.absoluteFilePath();

	// check if recent project file is already in the list
	int i = si.m_recentProjects.indexOf(filePath);

	if (i != -1) {
		// already there, move it to front
		si.m_recentProjects.removeAt(i);
		si.m_recentProjects.push_front(filePath);
	}
	else {
		si.m_recentProjects.push_front(filePath);
		while (static_cast<unsigned int>(si.m_recentProjects.count()) > si.m_maxRecentProjects)
			si.m_recentProjects.pop_back();
	}

	// update recent project list
	emit updateRecentProjects();
}


void replaceID(unsigned int & id, const std::map<unsigned int, unsigned int> & idSubstitutionMap) {
	// only replace if set
	if (id != VICUS::INVALID_ID) {
		std::map<unsigned int, unsigned int>::const_iterator idIt = idSubstitutionMap.find(id);
		if (idIt != idSubstitutionMap.end())
			id = idIt->second; // replace ID
	}
}


bool SVProjectHandler::importEmbeddedDB() {
	FUNCID(SVProjectHandler::importEmbeddedDB);

	bool idsModified = false;

	// we sync the embedded database with the built-in DB
	// we process all lists
	// - for each DB element, we check if such an element exists already in the DB
	// - if not, it is imported into the user DB
	// - if yes, the ID is checked and if mismatching, the ID is changed
	// - the old-new-ID transfer is recorded in db element-specific ID-map

	// Importing an element works as follows:
	// - check if ID is already used? -> given new ID and add with new ID

	SVDatabase & db = SVSettings::instance().m_db; // readibility-improvement

	// materials
	std::map<unsigned int, unsigned int> materialIDMap; // newID = materialIDMap[oldID];
	for (VICUS::Material & e : m_project->m_embeddedDB.m_materials) {
		// check, if element exists in built-in DB
		const VICUS::Material * existingElement = db.m_materials.findEqual(e);
		if (existingElement == nullptr) {
			// element does not yet exist, import element; we try to keep the id from the embedded element
			// but if this is already taken, the database assigns a new unused id for use
			unsigned int oldId = e.m_id;
			unsigned int newId = db.m_materials.add(e, oldId); // e.m_id gets modified here!
			IBK::IBK_Message( IBK::FormatString("Material '%1' with #%2 imported -> new ID #%3.\n")
				.arg(e.m_displayName.string(),50,std::ios_base::left).arg(oldId).arg(newId),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			if (newId != oldId)
				materialIDMap[oldId] = newId;
		}
		else {
			// check if IDs match
			if (existingElement->m_id != e.m_id) {
				// we need to adjust the ID name of material
				materialIDMap[e.m_id] = existingElement->m_id;
				IBK::IBK_Message( IBK::FormatString("Material '%1' with #%2 exists already -> new ID #%3.\n")
					.arg(e.m_displayName.string(),50,std::ios_base::left).arg(e.m_id).arg(existingElement->m_id),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			}
		}
	}

	// constructions
	std::map<unsigned int, unsigned int> constructionIDMap;
	for (VICUS::Construction & e : m_project->m_embeddedDB.m_constructions) {
		// apply material ID substitutions
		for (VICUS::MaterialLayer & lay : e.m_materialLayers)
			replaceID(lay.m_matId, materialIDMap);

		// check, if element exists in built-in DB
		const VICUS::Construction * existingElement = db.m_constructions.findEqual(e);
		if (existingElement == nullptr) {
			// element does not yet exist, import element; we try to keep the id from the embedded element
			// but if this is already taken, the database assigns a new unused id for use
			unsigned int oldId = e.m_id;
			unsigned int newId = db.m_constructions.add(e, oldId);
			IBK::IBK_Message( IBK::FormatString("Construction type '%1' with #%2 imported -> new ID #%3.\n")
				.arg(e.m_displayName.string(),50,std::ios_base::left).arg(oldId).arg(newId),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			if (newId != oldId)
				constructionIDMap[oldId] = newId;
		}
		else {
			// check if IDs match
			if (existingElement->m_id != e.m_id) {
				// we need to adjust the ID name of material
				constructionIDMap[e.m_id] = existingElement->m_id;
				IBK::IBK_Message( IBK::FormatString("Construction type '%1' with #%2 exists already -> new ID #%3.\n")
					.arg(e.m_displayName.string(),50,std::ios_base::left).arg(e.m_id).arg(existingElement->m_id),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			}
		}
	}

	// window glazing systems
	std::map<unsigned int, unsigned int> glazingSystemsIDMap;
	for (VICUS::WindowGlazingSystem & e : m_project->m_embeddedDB.m_windowGlazingSystems) {
		const VICUS::WindowGlazingSystem * existingElement = db.m_windowGlazingSystems.findEqual(e);
		if (existingElement == nullptr) {
			unsigned int oldId = e.m_id;
			unsigned int newId = db.m_windowGlazingSystems.add(e, oldId);
			IBK::IBK_Message( IBK::FormatString("Window glazing system '%1' with #%2 imported -> new ID #%3.\n")
				.arg(e.m_displayName.string(),50,std::ios_base::left).arg(oldId).arg(newId),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			if (newId != oldId)
				glazingSystemsIDMap[oldId] = newId;
		}
		else {
			if (existingElement->m_id != e.m_id) {
				glazingSystemsIDMap[e.m_id] = existingElement->m_id;
				IBK::IBK_Message( IBK::FormatString("Window glazing system '%1' with #%2 exists already -> new ID #%3.\n")
					.arg(e.m_displayName.string(),50,std::ios_base::left).arg(e.m_id).arg(existingElement->m_id),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			}
		}
	}

	// windows
	std::map<unsigned int, unsigned int> windowIDMap;
	for (VICUS::Window & e : m_project->m_embeddedDB.m_windows) {
		replaceID(e.m_idGlazingSystem, glazingSystemsIDMap);
		replaceID(e.m_frame.m_id, materialIDMap);
		replaceID(e.m_divider.m_id, materialIDMap);

		const VICUS::Window * existingElement = db.m_windows.findEqual(e);
		if (existingElement == nullptr) {
			// element does not yet exist, import element; we try to keep the id from the embedded element
			// but if this is already taken, the database assigns a new unused id for use
			unsigned int oldId = e.m_id;
			unsigned int newId = db.m_windows.add(e, oldId);
			IBK::IBK_Message( IBK::FormatString("Window '%1' with #%2 imported -> new ID #%3.\n")
				.arg(e.m_displayName.string(),50,std::ios_base::left).arg(oldId).arg(newId),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			if (newId != oldId)
				windowIDMap[oldId] = newId;
		}
		else {
			// check if IDs match
			if (existingElement->m_id != e.m_id) {
				// we need to adjust the ID name of material
				windowIDMap[e.m_id] = existingElement->m_id;
				IBK::IBK_Message( IBK::FormatString("Window '%1' with #%2 exists already -> new ID #%3.\n")
					.arg(e.m_displayName.string(),50,std::ios_base::left).arg(e.m_id).arg(existingElement->m_id),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			}
		}
	}

	// boundary conditions
	std::map<unsigned int, unsigned int> boundaryConditionsIDMap;
	for (VICUS::BoundaryCondition & e : m_project->m_embeddedDB.m_boundaryConditions) {
		const VICUS::BoundaryCondition * existingElement = db.m_boundaryConditions.findEqual(e);
		if (existingElement == nullptr) {
			unsigned int oldId = e.m_id;
			unsigned int newId = db.m_boundaryConditions.add(e, oldId);
			IBK::IBK_Message( IBK::FormatString("Boundary condition '%1' with #%2 imported -> new ID #%3.\n")
				.arg(e.m_displayName.string(),50,std::ios_base::left).arg(oldId).arg(newId),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			if (newId != oldId)
				boundaryConditionsIDMap[oldId] = newId;
		}
		else {
			if (existingElement->m_id != e.m_id) {
				boundaryConditionsIDMap[e.m_id] = existingElement->m_id;
				IBK::IBK_Message( IBK::FormatString("Boundary condition '%1' with #%2 exists already -> new ID #%3.\n")
					.arg(e.m_displayName.string(),50,std::ios_base::left).arg(e.m_id).arg(existingElement->m_id),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			}
		}
	}

	// component
	std::map<unsigned int, unsigned int> componentIDMap;
	for (VICUS::Component & e : m_project->m_embeddedDB.m_components) {
		replaceID(e.m_idConstruction, constructionIDMap);
		replaceID(e.m_idSideABoundaryCondition, boundaryConditionsIDMap);
		replaceID(e.m_idSideBBoundaryCondition, boundaryConditionsIDMap);

		const VICUS::Component * existingElement = db.m_components.findEqual(e);
		if (existingElement == nullptr) {
			unsigned int oldId = e.m_id;
			unsigned int newId = db.m_components.add(e, oldId);
			IBK::IBK_Message( IBK::FormatString("Component '%1' with #%2 imported -> new ID #%3.\n")
				.arg(e.m_displayName.string(),50,std::ios_base::left).arg(oldId).arg(newId),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			if (newId != oldId)
				componentIDMap[oldId] = newId;
		}
		else {
			// check if IDs match
			if (existingElement->m_id != e.m_id) {
				componentIDMap[e.m_id] = existingElement->m_id;
				IBK::IBK_Message( IBK::FormatString("Component '%1' with #%2 exists already -> new ID #%3.\n")
					.arg(e.m_displayName.string(),50,std::ios_base::left).arg(e.m_id).arg(existingElement->m_id),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			}
		}
	}

	// any ids modified?
	idsModified |= !materialIDMap.empty();
	idsModified |= !constructionIDMap.empty();
	idsModified |= !windowIDMap.empty();
	idsModified |= !glazingSystemsIDMap.empty();
	idsModified |= !boundaryConditionsIDMap.empty();
	idsModified |= !componentIDMap.empty();
	return idsModified;
}


void SVProjectHandler::updateSurfaceColors() {
	for (VICUS::Building &b : m_project->m_buildings) {
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (VICUS::Room &r : bl.m_rooms) {
				for (VICUS::Surface &s : r.m_surfaces) {
					s.updateColor();
					for (const VICUS::SubSurface &sub : s.subSurfaces()) {
						const_cast<VICUS::SubSurface &>(sub).updateColor();
					}
				}
			}
		}
	}

	// plain geometry surfaces will be silver
	for (VICUS::Surface &s : m_project->m_plainGeometry) {
		if (s.m_color == QColor::Invalid) {
			s.m_color = QColor("#C0C0C0");
		}
	}

}



