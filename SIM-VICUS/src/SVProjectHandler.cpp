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

#include "SVSettings.h"
#include "SVConstants.h"

SVProjectHandler * SVProjectHandler::m_self = nullptr;

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


bool SVProjectHandler::newProject(QWidget * parent) {

	VICUS::Project prj;

#if 0
	SVNewProjectWizardDialog * wizard = new SVNewProjectWizardDialog(parent);

	if (!wizard->runWizard(prj))
		return false;
#endif
	createProject();
	*m_project = prj; // copy over project data from wizard

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
#if 0
				SVLogFileDialog dlg;
				dlg.setLogFile(QtExt::Directories::globalLogFile(), fileName, true);
				dlg.exec();
#endif
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

		/// \todo implement and project data checks with automatic fixes and set have_modified_project to true
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
			tr("SIM-VICUS project files (*%1);;All files (*.*)").arg(SVSettings::instance().m_projectFileSuffix)
		);


	if (filename.isEmpty()) return SaveCanceled; // canceled

	QString fnamebase = QFileInfo(filename).baseName();
	if (fnamebase.isEmpty()) {
		QMessageBox::critical(parent, tr("Invalid file name"), tr("Please enter a valid file name!"));
		return SaveCanceled;
	}

	// relay to saveProject() which updates modified flag and emits corresponding signals.
	if (saveProject(parent, filename) != SaveOK)
		return SaveFailed; // saving failed

	/// \todo signal a change of file name, for widgets that are displaying the current filename

	return SaveOK;
}


SVProjectHandler::SaveResult SVProjectHandler::saveProject(QWidget * parent, const QString & fileName, bool addToRecentFilesList) {

	// check project file ending, if there is none append it
	QString fname = fileName;
	if (!fname.endsWith(SVSettings::instance().m_projectFileSuffix))
		fname.append( SVSettings::instance().m_projectFileSuffix );

#if 0
	// updated created and lastEdited tags
	if (m_project->m_created.empty())
		m_project->m_created = QDateTime::currentDateTime().toString(Qt::TextDate).toUtf8().data();
	m_project->m_lastEdited = QDateTime::currentDateTime().toString(Qt::TextDate).toUtf8().data();
#endif

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
	/// \todo insert placeholders into map


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


		/// \todo replace all absolute file names with relative file paths (using placeholders)

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


