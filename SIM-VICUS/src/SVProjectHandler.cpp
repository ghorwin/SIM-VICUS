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

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include "SVSettings.h"
#include "SVConstants.h"
#include "SVLogFileDialog.h"
#include "SVViewStateHandler.h"
#include "SVUndoModifyProject.h"

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
	}
	else {
		// add default building and building level
		VICUS::Building b;
		b.m_id = 1;
		b.m_displayName = tr("Building");
		VICUS::BuildingLevel bl;
		bl.m_id = 2;
		bl.m_displayName = tr("Ground floor");
		b.m_buildingLevels.push_back(bl);
		m_project->m_buildings.push_back(b);
	}
	// update all internal pointers
	m_project->updatePointers();
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
				throw IBK::Exception(IBK::FormatString("Error reading project '%1'").arg(fileName.toStdString()), FUNC_ID);
			// project read successfully

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

	try {
		// once the project has been read, perform "post-read" actions
		bool have_modified_project = false;

		// import embedded DB into our user DB
		have_modified_project = importEmbeddedDB(*m_project); // Note: project may be modified in case IDs were adjusted

		// fix problems in the project; will set have_modified_project to true if fixes were applied
		fixProject(*m_project, have_modified_project);

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

bool SVProjectHandler::importProject(VICUS::Project * project) {
	if(project == nullptr) {
		return newProject(project);
	}

	createProject();
	*m_project = *project; // copy over project
	// update all internal pointers
	try {
		m_project->updatePointers();
	}
	catch (const IBK::Exception& e) {
		QMessageBox::critical(nullptr, tr("Error while importing ifc file"), e.what());
		return false;
	}

	// initialize viewstate
	SVViewState vs = SVViewStateHandler::instance().viewState();
	SVViewStateHandler::instance().setViewState(vs);

	// once the project has been read, perform "post-read" actions
	bool have_modified_project = false;

	// import embedded DB into our user DB
	have_modified_project = importEmbeddedDB(*m_project); // Note: project may be modified in case IDs were adjusted

	// fix problems in the project; will set have_modified_project to true if fixes were applied
	fixProject(*m_project, have_modified_project);

	// this will clear the modified flag again (since we just read the project) except if we had made some automatic
	// fixes above
	m_modified = have_modified_project;

	setModified(AllModified);

	// signal UI that we now have a project
	emit updateActions();

	return true;
}



void SVProjectHandler::reloadProject(QWidget * parent) {
	QString projectFileName = projectFile();
	m_modified = false; // so that closeProject doesn't ask questions
	closeProject(parent);
	loadProject(parent, projectFileName, false); // emits updateActions() if project was successfully loaded
}


void SVProjectHandler::importProject(VICUS::Project & other) {
	// we must have a project loaded
	IBK_ASSERT(isValid());

	importEmbeddedDB(other); // Note: project may be modified in case IDs were adjusted

	// now merge project data into our project, hereby creating mapping tables to relable building, room, surface and subsurface IDs
	std::map<unsigned int, unsigned int> IDMap; // newID = IDMap[oldID]
	IDMap[VICUS::INVALID_ID] = VICUS::INVALID_ID;

	unsigned int nextID = m_project->nextUnusedID();

	// process all buildings in 'other' and adjust IDs where conflicts with existing IDs exist
	for (VICUS::Building & b : other.m_buildings) {
		if (m_project->objectById(b.m_id) != nullptr) {
			// give building a new ID
			unsigned int newID = nextID++;
			IDMap[b.m_id] = newID;
			b.m_id = newID;
		}
		else	IDMap[b.m_id] = b.m_id;

		for (VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			if (m_project->objectById(bl.m_id) != nullptr) {
				unsigned int newID = nextID++;
				IDMap[bl.m_id] = newID;
				bl.m_id = newID;
			}
			else	IDMap[bl.m_id] = bl.m_id;

			for (VICUS::Room & r : bl.m_rooms) {
				if (m_project->objectById(r.m_id) != nullptr) {
					unsigned int newID = nextID++;
					IDMap[r.m_id] = newID;
					r.m_id = newID;
				}
				else	IDMap[r.m_id] = r.m_id;

				for (VICUS::Surface & s : r.m_surfaces) {
					if (m_project->objectById(s.m_id) != nullptr) {
						unsigned int newID = nextID++;
						IDMap[s.m_id] = newID;
						s.m_id = newID;
					}
					else	IDMap[s.m_id] = s.m_id;

					// process the subsurfaces as well - since we only modify IDs, we can const-cast the sub-surfaces vector
					for (VICUS::SubSurface & sub : const_cast<std::vector<VICUS::SubSurface>&>(s.subSurfaces()) ) {
						if (m_project->objectById(sub.m_id) != nullptr) {
							unsigned int newID = nextID++;
							IDMap[sub.m_id] = newID;
							sub.m_id = newID;
						}
						else	IDMap[sub.m_id] = sub.m_id;
					}
				}
			}
		}
	}

	// TODO Andreas, adjust Plain geometry
	// TODO Hauke, adjust Network

	// all IDs adjusted, now modify component instances
	for (VICUS::ComponentInstance & c : other.m_componentInstances) {
		c.m_idSideASurface = IDMap[c.m_idSideASurface];
		c.m_idSideBSurface = IDMap[c.m_idSideBSurface];
	}

	for (VICUS::SubSurfaceComponentInstance & c : other.m_subSurfaceComponentInstances) {
		c.m_idSideASurface = IDMap[c.m_idSideASurface];
		c.m_idSideBSurface = IDMap[c.m_idSideBSurface];
	}

	// fix problems in the project; will set have_modified_project to true if fixes were applied
	bool haveErrors;
	// update internal pointer-based links
	other.updatePointers();
	fixProject(other, haveErrors);
	// now create a project-modified undo action
	VICUS::Project mergedProject(*m_project);
	// copy all data from 'other' into project
	mergedProject.m_buildings.insert(mergedProject.m_buildings.end(), other.m_buildings.begin(), other.m_buildings.end());
	mergedProject.m_geometricNetworks.insert(mergedProject.m_geometricNetworks.end(), other.m_geometricNetworks.begin(), other.m_geometricNetworks.end());
	mergedProject.m_componentInstances.insert(mergedProject.m_componentInstances.end(), other.m_componentInstances.begin(), other.m_componentInstances.end());
	mergedProject.m_subSurfaceComponentInstances.insert(mergedProject.m_subSurfaceComponentInstances.end(), other.m_subSurfaceComponentInstances.begin(), other.m_subSurfaceComponentInstances.end());
	mergedProject.m_plainGeometry.insert(mergedProject.m_plainGeometry.end(), other.m_plainGeometry.begin(), other.m_plainGeometry.end());

	SVUndoModifyProject * undo = new SVUndoModifyProject(tr("Merged imported project"), mergedProject);
	undo->push();
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
			nullptr,
			SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
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
		m_project->m_projectInfo.m_created = QDateTime::currentDateTime().toString(Qt::TextDate).toStdString();
	m_project->m_projectInfo.m_lastEdited = QDateTime::currentDateTime().toString(Qt::TextDate).toStdString();

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

		m_project->m_placeholders.clear(); // clear placeholders again - not really necessary, but helps preventing programming errors
		return SaveFailed;
	}

	m_project->m_placeholders.clear(); // clear placeholders again - not really necessary, but helps preventing programming errors

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
	std::map<std::string, IBK::Path> mergedPlaceholders;

	mergedPlaceholders[VICUS::DATABASE_PLACEHOLDER_NAME] = QtExt::Directories::databasesDir().toStdString();
	mergedPlaceholders[VICUS::USER_DATABASE_PLACEHOLDER_NAME] = QtExt::Directories::userDataDir().toStdString();

	if (!projectFile().isEmpty())
		mergedPlaceholders["Project Directory"] =
				QFileInfo(projectFile()).absoluteDir().absolutePath().toStdString();


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


template <typename T>
void importDBElement(T & e, VICUS::Database<T> & db, std::map<unsigned int, unsigned int> & idSubstitutionMap,
					 const char * const importMsg, const char * const existingMsg)
{
	FUNCID(SVProjectHandler-importDBElement);
	// check, if element exists in built-in DB
	const T * existingElement = db.findEqual(e);
	if (existingElement == nullptr) {
		// element does not yet exist, import element; we try to keep the id from the embedded element
		// but if this is already taken, the database assigns a new unused id for use
		unsigned int oldId = e.m_id;
		unsigned int newId = db.add(e, oldId); // e.m_id gets modified here!
		IBK::IBK_Message( IBK::FormatString(importMsg)
			.arg(e.m_displayName.string(),50,std::ios_base::left).arg(oldId).arg(newId),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		if (newId != oldId)
			idSubstitutionMap[oldId] = newId;
	}
	else {
		// check if IDs match
		if (existingElement->m_id != e.m_id) {
			// we need to adjust the ID name of material
			idSubstitutionMap[e.m_id] = existingElement->m_id;
			IBK::IBK_Message( IBK::FormatString(existingMsg)
				.arg(e.m_displayName.string(),50,std::ios_base::left).arg(e.m_id).arg(existingElement->m_id),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}
}


void replaceID(unsigned int & id, const std::map<unsigned int, unsigned int> & idSubstitutionMap) {
	// only replace if set
	if (id != VICUS::INVALID_ID) {
		std::map<unsigned int, unsigned int>::const_iterator idIt = idSubstitutionMap.find(id);
		if (idIt != idSubstitutionMap.end())
			id = idIt->second; // replace ID
	}
}


bool SVProjectHandler::importEmbeddedDB(VICUS::Project & pro) {
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
	for (VICUS::Material & e : pro.m_embeddedDB.m_materials) {
		// check, if element exists in built-in DB
		importDBElement(e, db.m_materials, materialIDMap,
			"Material '%1' with #%2 imported -> new ID #%3.\n",
			"Material '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// constructions
	std::map<unsigned int, unsigned int> constructionIDMap;
	for (VICUS::Construction & e : pro.m_embeddedDB.m_constructions) {
		// apply material ID substitutions
		for (VICUS::MaterialLayer & lay : e.m_materialLayers)
			replaceID(lay.m_idMaterial, materialIDMap);

		importDBElement(e, db.m_constructions, constructionIDMap,
			"Construction type '%1' with #%2 imported -> new ID #%3.\n",
			"Construction type '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// window glazing systems
	std::map<unsigned int, unsigned int> glazingSystemsIDMap;
	for (VICUS::WindowGlazingSystem & e : pro.m_embeddedDB.m_windowGlazingSystems) {
		importDBElement(e, db.m_windowGlazingSystems, glazingSystemsIDMap,
			"Window glazing system '%1' with #%2 imported -> new ID #%3.\n",
			"Window glazing system '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// windows
	std::map<unsigned int, unsigned int> windowIDMap;
	for (VICUS::Window & e : pro.m_embeddedDB.m_windows) {
		replaceID(e.m_idGlazingSystem, glazingSystemsIDMap);
		replaceID(e.m_frame.m_id, materialIDMap);
		replaceID(e.m_divider.m_id, materialIDMap);

		importDBElement(e, db.m_windows, windowIDMap,
			"Window '%1' with #%2 imported -> new ID #%3.\n",
			"Window '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// TODO Dirk, mind renamed schedule IDs in all sub template models
	//      also, mind renamed sub-template models in zone templates
	// schedules
	std::map<unsigned int, unsigned int> schedulesIDMap;
	for (VICUS::Schedule & e : pro.m_embeddedDB.m_schedules) {

		importDBElement(e, db.m_schedules, schedulesIDMap,
			"Schedule '%1' with #%2 imported -> new ID #%3.\n",
			"Schedule '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// boundary conditions
	std::map<unsigned int, unsigned int> boundaryConditionsIDMap;
	for (VICUS::BoundaryCondition & e : pro.m_embeddedDB.m_boundaryConditions) {
		replaceID(e.m_heatConduction.m_idSchedule, schedulesIDMap);
		importDBElement(e, db.m_boundaryConditions, boundaryConditionsIDMap,
			"Boundary condition '%1' with #%2 imported -> new ID #%3.\n",
			"Boundary condition '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// component
	std::map<unsigned int, unsigned int> componentIDMap;
	for (VICUS::Component & e : pro.m_embeddedDB.m_components) {
		replaceID(e.m_idConstruction, constructionIDMap);
		replaceID(e.m_idSideABoundaryCondition, boundaryConditionsIDMap);
		replaceID(e.m_idSideBBoundaryCondition, boundaryConditionsIDMap);

		importDBElement(e, db.m_components, componentIDMap,
			"Component '%1' with #%2 imported -> new ID #%3.\n",
			"Component '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// sub-surface component
	std::map<unsigned int, unsigned int> subSurfaceComponentIDMap;
	for (VICUS::SubSurfaceComponent & e : pro.m_embeddedDB.m_subSurfaceComponents) {
		replaceID(e.m_idWindow, windowIDMap);
		replaceID(e.m_idConstruction, constructionIDMap);
		replaceID(e.m_idSideABoundaryCondition, boundaryConditionsIDMap);
		replaceID(e.m_idSideBBoundaryCondition, boundaryConditionsIDMap);

		importDBElement(e, db.m_subSurfaceComponents, subSurfaceComponentIDMap,
			"Sub-surface component '%1' with #%2 imported -> new ID #%3.\n",
			"Sub-surface component '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// network pipes
	std::map<unsigned int, unsigned int> pipesIDMap;
	for (VICUS::NetworkPipe & e : pro.m_embeddedDB.m_pipes) {

		importDBElement(e, db.m_pipes, pipesIDMap,
						"Pipe '%1' with #%2 imported -> new ID #%3.\n",
						"Pipe '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// surface heating
	std::map<unsigned int, unsigned int> surfaceHeatingIDMap;
	for (VICUS::SurfaceHeating& e : pro.m_embeddedDB.m_surfaceHeatings) {

		replaceID(e.m_idPipe, pipesIDMap);

		importDBElement(e, db.m_surfaceHeatings, surfaceHeatingIDMap,
						"Surface heating '%1' with #%2 imported -> new ID #%3.\n",
						"Surface heating '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// InternalLoad
	std::map<unsigned int, unsigned int> internalLoadIDMap;
	for (VICUS::InternalLoad & e : pro.m_embeddedDB.m_internalLoads) {
		replaceID(e.m_idActivitySchedule, schedulesIDMap);
		replaceID(e.m_idOccupancySchedule, schedulesIDMap);
		replaceID(e.m_idPowerManagementSchedule, schedulesIDMap);
		importDBElement(e, db.m_internalLoads, internalLoadIDMap,
			"Internal loads '%1' with #%2 imported -> new ID #%3.\n",
			"Internal loads '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// infiltration
	std::map<unsigned int, unsigned int> infiltrationIDMap;
	for (VICUS::Infiltration & e : pro.m_embeddedDB.m_infiltration) {
		importDBElement(e, db.m_infiltration, infiltrationIDMap,
						"Infiltration '%1' with #%2 imported -> new ID #%3.\n",
						"Infiltration '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// ventilation
	std::map<unsigned int, unsigned int> ventilationIDMap;
	for (VICUS::VentilationNatural & e : pro.m_embeddedDB.m_ventilationNatural) {
		replaceID(e.m_idSchedule, schedulesIDMap);

		importDBElement(e, db.m_ventilationNatural, ventilationIDMap,
						"Natural ventilation '%1' with #%2 imported -> new ID #%3.\n",
						"Natural ventilation '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// zone control thermostat
	std::map<unsigned int, unsigned int> thermostatIDMap;
	for (VICUS::ZoneControlThermostat & e : pro.m_embeddedDB.m_zoneControlThermostats) {
		replaceID(e.m_idHeatingSetpointSchedule,schedulesIDMap);
		replaceID(e.m_idCoolingSetpointSchedule,schedulesIDMap);
		importDBElement(e, db.m_zoneControlThermostat, thermostatIDMap,
						"Thermostat '%1' with #%2 imported -> new ID #%3.\n",
						"Thermostat '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// zone control natural ventilation
	std::map<unsigned int, unsigned int> ventilationCtrlIDMap;
	for (VICUS::ZoneControlNaturalVentilation & e : pro.m_embeddedDB.m_zoneControlVentilationNatural) {

		importDBElement(e, db.m_zoneControlVentilationNatural, ventilationCtrlIDMap,
						"Natural ventilation control '%1' with #%2 imported -> new ID #%3.\n",
						"Natural ventilation control '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// zone control shading
	std::map<unsigned int, unsigned int> shadingCtrlIDMap;
	for (VICUS::ZoneControlShading & e : pro.m_embeddedDB.m_zoneControlShading) {

		importDBElement(e, db.m_zoneControlShading, shadingCtrlIDMap,
						"Shading control '%1' with #%2 imported -> new ID #%3.\n",
						"Shading control '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// HVAC ideal heating
	std::map<unsigned int, unsigned int> idealHeatingCoolingIDMap;
	for (VICUS::ZoneIdealHeatingCooling & e : pro.m_embeddedDB.m_zoneIdealHeatingCooling) {

		importDBElement(e, db.m_zoneIdealHeatingCooling, idealHeatingCoolingIDMap,
						"HVAC ideal heating/cooling '%1' with #%2 imported -> new ID #%3.\n",
						"HVAC ideal heating/cooling '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// zone templates
	std::map<unsigned int, unsigned int> zoneTemplatesIDMap;
	for (VICUS::ZoneTemplate & e : pro.m_embeddedDB.m_zoneTemplates) {

		replaceID(e.m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson], internalLoadIDMap);
		replaceID(e.m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment], internalLoadIDMap);
		replaceID(e.m_idReferences[VICUS::ZoneTemplate::ST_IntLoadLighting], internalLoadIDMap);
		replaceID(e.m_idReferences[VICUS::ZoneTemplate::ST_Infiltration], infiltrationIDMap);
		replaceID(e.m_idReferences[VICUS::ZoneTemplate::ST_VentilationNatural], ventilationIDMap);
		replaceID(e.m_idReferences[VICUS::ZoneTemplate::ST_ControlThermostat], thermostatIDMap);
		replaceID(e.m_idReferences[VICUS::ZoneTemplate::ST_ControlVentilationNatural], ventilationCtrlIDMap);
		//TODO Dirk Shading implementieren
		//replaceID(e.m_idReferences[VICUS::ZoneTemplate::ST_Control], );
		replaceID(e.m_idReferences[VICUS::ZoneTemplate::ST_IdealHeatingCooling], idealHeatingCoolingIDMap);

		importDBElement(e, db.m_zoneTemplates, zoneTemplatesIDMap,
						"Zone template '%1' with #%2 imported -> new ID #%3.\n",
						"Zone template '%1' with #%2 exists already -> ID #%3.\n"
		);
	}


	// network fluids
	std::map<unsigned int, unsigned int> fluidsIDMap;
	for (VICUS::NetworkFluid & e : pro.m_embeddedDB.m_fluids) {

		importDBElement(e, db.m_fluids, fluidsIDMap,
						"Fluid '%1' with #%2 imported -> new ID #%3.\n",
						"Fluid '%1' with #%2 exists already -> ID #%3.\n"
		);
	}



	// network components
	std::map<unsigned int, unsigned int> netComponentsIDMap;
	for (VICUS::NetworkComponent & e : pro.m_embeddedDB.m_networkComponents) {

		importDBElement(e, db.m_networkComponents, netComponentsIDMap,
						"Network Component '%1' with #%2 imported -> new ID #%3.\n",
						"Network Component '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// network controllers
	std::map<unsigned int, unsigned int> netControllersIDMap;
	for (VICUS::NetworkController & e : pro.m_embeddedDB.m_networkControllers) {

		importDBElement(e, db.m_networkControllers, netControllersIDMap,
						"Network Controller '%1' with #%2 imported -> new ID #%3.\n",
						"Network Controller '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	// network subnetwork-components
	std::map<unsigned int, unsigned int> subNetworksIDMap;
	for (VICUS::SubNetwork & e : pro.m_embeddedDB.m_subNetworks) {

		// replace IDs referenced from NANDRAD::HydraulicNetworkElement

		for (VICUS::NetworkElement & elem : e.m_elements) {
			replaceID(elem.m_componentId, netComponentsIDMap);
			replaceID(elem.m_controlElementId, netControllersIDMap);
		}

		importDBElement(e, db.m_subNetworks, subNetworksIDMap,
						"Sub Network '%1' with #%2 imported -> new ID #%3.\n",
						"Sub Network '%1' with #%2 exists already -> ID #%3.\n"
		);
	}


	// *** Database Update Complete - now modify project to use the potentially modified IDs ***

	// now that all DB elements have been imported, we need to replace the referenced to those ID elements in the project

	// ** ComponentInstance and SubSurfaceComponentInstance **

	for (VICUS::ComponentInstance & ci : pro.m_componentInstances) {
		replaceID(ci.m_idComponent, componentIDMap);
		replaceID(ci.m_idSurfaceHeating, surfaceHeatingIDMap);
	}
	for (VICUS::SubSurfaceComponentInstance & ci : pro.m_subSurfaceComponentInstances)
		replaceID(ci.m_idSubSurfaceComponent, subSurfaceComponentIDMap);

	// ** ZoneTemplates **

	for (VICUS::Building &b : pro.m_buildings) {
		for (VICUS::BuildingLevel &bl : b.m_buildingLevels) {
			for (VICUS::Room &r : bl.m_rooms)
				replaceID(r.m_idZoneTemplate, zoneTemplatesIDMap);
		}
	}

	// ** Network (Nodes, Edges, Pipes, Fluid) **

	for (VICUS::Network & n : pro.m_geometricNetworks) {

		for (VICUS::NetworkNode & node : n.m_nodes)
			replaceID(node.m_idSubNetwork, subNetworksIDMap);

		for (VICUS::NetworkEdge & edge : n.m_edges)
			replaceID(edge.m_idPipe, pipesIDMap);

		replaceID(n.m_idFluid, fluidsIDMap);
		for (unsigned int & pipeID : n.m_availablePipes)
			replaceID(pipeID, pipesIDMap);
	}

	// any ids modified?
	idsModified |= !materialIDMap.empty();
	idsModified |= !constructionIDMap.empty();
	idsModified |= !windowIDMap.empty();
	idsModified |= !glazingSystemsIDMap.empty();
	idsModified |= !boundaryConditionsIDMap.empty();
	idsModified |= !componentIDMap.empty();
	idsModified |= !subSurfaceComponentIDMap.empty();
	idsModified |= !schedulesIDMap.empty();
	idsModified |= !internalLoadIDMap.empty();
	idsModified |= !infiltrationIDMap.empty();
	idsModified |= !ventilationIDMap.empty();
	idsModified |= !thermostatIDMap.empty();
	idsModified |= !ventilationCtrlIDMap.empty();
	idsModified |= !shadingCtrlIDMap.empty();
	idsModified |= !idealHeatingCoolingIDMap.empty();
	idsModified |= !zoneTemplatesIDMap.empty();
	idsModified |= !pipesIDMap.empty();
	idsModified |= !fluidsIDMap.empty();
	idsModified |= !surfaceHeatingIDMap.empty();
	idsModified |= !netComponentsIDMap.empty();
	idsModified |= !netControllersIDMap.empty();
	idsModified |= !subNetworksIDMap.empty();

	return idsModified;
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

	// remove temporary local DB elements
	SVSettings::instance().m_db.removeLocalElements();
}


bool SVProjectHandler::read(const QString & fname) {
	FUNCID(SVProjectHandler::read);

	// check that we have a project, should be newly created
	Q_ASSERT(isValid());

	if (!QFileInfo::exists(fname)) {
		IBK::IBK_Message(IBK::FormatString("File '%1' does not exist or permissions are missing for accessing the file.")
						 .arg(fname.toStdString()), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	try {

		// filename is converted to utf8 before calling readXML
		m_project->readXML(IBK::Path(fname.toStdString()) );
		m_projectFile = fname;

		// clear placeholders - this avoids confusion with outdates placeholders (ie when a different user has saved the
		// project file)
		// when resolving file paths with placeholders, one should always call
		// SVProjectHandler::instance().replacePathPlaceholders(...)
		m_project->m_placeholders.clear();

		m_lastReadTime = QFileInfo(fname).lastModified();

		// update the colors
		// if project has invalid colors nothing is drawn ...
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
							 .arg((fname + ".bak").toStdString()), IBK::MSG_ERROR, FUNC_ID);
	}

	// create file
	QFile file(fname);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		IBK::IBK_Message(IBK::FormatString("Cannot create/write file '%1' (path does not exists or missing permissions).")
						 .arg(fname.toStdString()), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}
	file.close();

	try {
		IBK::Path filename(fname.toStdString());
		IBK::Path newProjectDir = filename.parentPath();
		IBK::Path oldProjectDir(m_projectFile.toStdString());
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


void SVProjectHandler::updateSurfaceColors() {
	for (VICUS::Building &b : m_project->m_buildings) {
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (VICUS::Room &r : bl.m_rooms) {
				for (VICUS::Surface &s : r.m_surfaces) {
					if (!s.m_displayColor.isValid())
						s.initializeColorBasedOnInclination();
					s.m_color = s.m_displayColor;
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


void SVProjectHandler::fixProject(VICUS::Project & project, bool & haveModifiedProject) {
	FUNCID(SVProjectHandler::fixProject);

	// Note: the pointer interlinks have been updated in Project::readXML() already

	const SVDatabase & db = SVSettings::instance().m_db;

	// remove/fix invalid CI
	std::vector<VICUS::ComponentInstance> fixedCI;
	for (const VICUS::ComponentInstance & ci : project.m_componentInstances) {
		// surface IDs are the same on both sides?
		if (ci.m_idSideASurface == ci.m_idSideBSurface) {
			IBK::IBK_Message(IBK::FormatString("Removing ComponentInstance #%1 because both assigned surfaces have the same ID.").arg(ci.m_id),
							 IBK::MSG_WARNING, FUNC_ID);
			continue;
		}

		if (ci.m_sideASurface == nullptr && ci.m_idSideASurface != VICUS::INVALID_ID) {
			IBK::IBK_Message(IBK::FormatString("Removing ComponentInstance #%1 because surface A is referenced with invalid ID.").arg(ci.m_id),
							 IBK::MSG_WARNING, FUNC_ID);
			continue;
		}

		if (ci.m_sideBSurface == nullptr && ci.m_idSideBSurface != VICUS::INVALID_ID) {
			IBK::IBK_Message(IBK::FormatString("Removing ComponentInstance #%1 because surface B is referenced with invalid ID.").arg(ci.m_id),
							 IBK::MSG_WARNING, FUNC_ID);
			continue;
		}

		// component ID invalid?
		if (ci.m_idComponent != VICUS::INVALID_ID) {
			if (db.m_components[ci.m_idComponent] == nullptr) { // no such component in DB
				IBK::IBK_Message(IBK::FormatString("Removing Component reference from ComponentInstance #%1, because no such component exists (anylonger)").arg(ci.m_id),
								 IBK::MSG_WARNING, FUNC_ID);

				VICUS::ComponentInstance modCI(ci);
				modCI.m_idComponent = VICUS::INVALID_ID;
				fixedCI.push_back(modCI);
				continue;
			}
		}

		// all ok, keep CI
		fixedCI.push_back(ci);
	}

	if (fixedCI.size() != project.m_componentInstances.size()) {
		haveModifiedProject = true;
		project.m_componentInstances.swap(fixedCI);
	}


	std::vector<VICUS::SubSurfaceComponentInstance> fixedSCI;
	for (const VICUS::SubSurfaceComponentInstance & ci : project.m_subSurfaceComponentInstances) {
		// surface IDs are the same on both sides?
		if (ci.m_idSideASurface == ci.m_idSideBSurface) {
			IBK::IBK_Message(IBK::FormatString("Removing SubSurfaceComponentInstance #%1 because both assigned surfaces have the same ID.").arg(ci.m_id),
							 IBK::MSG_WARNING, FUNC_ID);
			continue;
		}

		if (ci.m_sideASubSurface == nullptr && ci.m_idSideASurface != VICUS::INVALID_ID) {
			IBK::IBK_Message(IBK::FormatString("Removing SubSurfaceComponentInstance #%1 because surface A is referenced with invalid ID.").arg(ci.m_id),
							 IBK::MSG_WARNING, FUNC_ID);
			continue;
		}

		if (ci.m_sideBSubSurface == nullptr && ci.m_idSideBSurface != VICUS::INVALID_ID) {
			IBK::IBK_Message(IBK::FormatString("Removing SubSurfaceComponentInstance #%1 because surface B is referenced with invalid ID.").arg(ci.m_id),
							 IBK::MSG_WARNING, FUNC_ID);
			continue;
		}

		// component ID invalid?
		if (ci.m_idSubSurfaceComponent != VICUS::INVALID_ID) {
			if (db.m_subSurfaceComponents[ci.m_idSubSurfaceComponent] == nullptr) { // no such component in DB
				IBK::IBK_Message(IBK::FormatString("Removing SubSurfaceComponent reference from SubSurfaceComponentInstance #%1, because no such sub-surface component exists (anylonger)").arg(ci.m_id),
								 IBK::MSG_WARNING, FUNC_ID);

				VICUS::SubSurfaceComponentInstance modCI(ci);
				modCI.m_idSubSurfaceComponent = VICUS::INVALID_ID;
				fixedSCI.push_back(modCI);
				continue;
			}
		}

		// all ok, keep SCI
		fixedSCI.push_back(ci);
	}

	if (fixedSCI.size() != project.m_subSurfaceComponentInstances.size()) {
		haveModifiedProject = true;
		project.m_subSurfaceComponentInstances.swap(fixedSCI);
	}

	/// \todo Hauke, check uniqueness of IDs in networks
}


