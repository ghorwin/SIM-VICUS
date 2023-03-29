#include "SVAutoSaveDialog.h"
#include "ui_SVAutoSaveDialog.h"

#include <SVConstants.h>
#include <SVStyle.h>
#include <SVProjectHandler.h>

#include <IBK_FileReader.h>

#include <QComboBox>
#include <QDir>
#include <QFileDialog>

#include <QtExt_Directories.h>


SVAutoSaveDialog::SVAutoSaveDialog(QDialog *parent) :
	QDialog(parent),
	m_ui(new Ui::SVAutoSaveDialog)
{
	m_ui->setupUi(this);

	// Format database
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetFiles);

	m_ui->tableWidgetFiles->setColumnCount(2);
	m_ui->tableWidgetFiles->setHorizontalHeaderLabels(QStringList() << "Filename" << "Basepath");

	m_ui->tableWidgetFiles->setColumnWidth(AC_BasePath, 280);
	m_ui->tableWidgetFiles->setColumnWidth(AC_FileName, 200);
	m_ui->tableWidgetFiles->horizontalHeader()->setStretchLastSection(true);

	// Connect Signal / Slots
	connect(&SVProjectHandler::instance(), &SVProjectHandler::removeProjectAutoSaves, this, &SVAutoSaveDialog::onRemoveProjectSepcificAutoSaves);

	// Update ui
	updateUi();
}


SVAutoSaveDialog::~SVAutoSaveDialog() {
	delete m_ui;
}


bool SVAutoSaveDialog::checkForAutoSaves() {
	FUNCID(SVAutoSaveDialog::checkForAutoSaves);

	// Clear all current auto saves.
	m_autoSaveData.clear();

	QString userDir = QtExt::Directories::userDataDir();

	// Path to autosaves
	QString autoSavesDirName = userDir + "/autosaves/";
	QString autoSavesMetaDataName = userDir + "/autosaves/autosave-metadata.info";

	// QDir
	QDir autoSavesDir(autoSavesDirName);

	if(!autoSavesDir.exists())
		return false;

	if(autoSavesDir.isEmpty())
		return false;

	QFile autoSaveInformation(autoSavesMetaDataName);
	if(!autoSaveInformation.open(QIODevice::ReadOnly | QIODevice::Text)) {
		IBK::IBK_Message(IBK::FormatString("Cannot read metadata file '%1' (path does not exist or missing permissions).")
						 .arg(autoSavesMetaDataName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}
	autoSaveInformation.close();

	// Read data
	IBK::Path metaData(autoSavesMetaDataName.toStdString());
	std::vector<std::string> lines;
	IBK::FileReader::readAll(metaData, lines, std::vector<std::string>());

	if(lines.size() < 2)
		return false;

	// Remove header
	lines.erase(lines.begin());

	// QStringList files = autoSavesDir.entryList(QDir::Files);

	std::vector<std::string> tokens;
	for (unsigned int i=0; i < lines.size(); ++i) {
		// Current line
		const std::string &line = lines[i];

		// split columns
		IBK::explode(line, tokens, "\t", IBK::EF_NoFlags | IBK::EF_TrimTokens | IBK::EF_KeepEmptyTokens);

		// fast access
		const QString projectName = QString::fromStdString(tokens[0]);
		const QString hash        = QString::fromStdString(tokens[1]);
		const QString timeStamp   = QString::fromStdString(tokens[2]);
		const QString basePath    = QString::fromStdString(tokens[3]);

		// FileName
		QString fileName = QString::fromStdString(tokens[AC_FileName]);

		if(fileName.endsWith(".vicus"))
			fileName.remove(".vicus");

		const QString autoSaveFile = autoSavesDirName + "/" + fileName + "(" + hash + ").vicus.bak" ;
		QFile autoSavefile(autoSaveFile);

//		// Check if file can be opened
//		if(!autoSavefile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//			IBK::IBK_Message(IBK::FormatString("Cannot read auto-save file '%1' (path does not exist or missing permissions).")
//							 .arg(fileName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
//			continue;
//		}

		m_autoSaveData.push_back(AutoSaveData(projectName, hash, timeStamp, basePath));
	}

	return !m_autoSaveData.empty();
}


void SVAutoSaveDialog::onRemoveProjectSepcificAutoSaves(const QString &projectName) {
	IBK::Path path(projectName.toStdString());

	QString basePath, projectFile;
	if(projectName.isEmpty()) {
		basePath = "";
		projectFile = "UnnamedProject";
	}
	else {
		basePath = QString::fromStdString(path.parentPath().str() );
		projectFile = QString::fromStdString(path.filename().str() );
	}

	removeProjectFiles(basePath, projectFile);
}


void SVAutoSaveDialog::handleAutoSaves() {
	if(!checkForAutoSaves()) {
		writeAutoSaveData(); // update auto save metadate (remove all entries)
		return; // no auto-saves have been found
	}

	// Read meta-data
	updateUi();

	// Exec dialog
	exec();
}


void SVAutoSaveDialog::updateAutoSavesInTable() {

	m_ui->tableWidgetFiles->setRowCount(0);
	m_ui->comboBoxTimestamps->clear();

	std::set<unsigned int> handledLines;
	for (unsigned int i=0; i<m_autoSaveData.size(); ++i) {

		if(handledLines.find(i) != handledLines.end())
			continue;

		const AutoSaveData &currentData = m_autoSaveData[i];

		QDateTime dt = QDateTime::fromString(currentData.m_timeStamp, "yyyyMMdd-hhmmss");

		std::vector<std::pair<QDateTime, unsigned int>> dateTimes;
		dateTimes.push_back(std::pair<QDateTime, unsigned int>(dt, i));
		// look for different time stamps
		for(unsigned int j=0; j<m_autoSaveData.size(); ++j) {
			if(i==j)
				continue;

			const AutoSaveData &data = m_autoSaveData[j];
			if(currentData.m_basePath == data.m_basePath &&
					currentData.m_fileName == data.m_fileName) {
				QDateTime dt = QDateTime::fromString(data.m_timeStamp, "yyyyMMdd-hhmmss");
				dateTimes.push_back(std::pair<QDateTime, unsigned int>(dt, j));
				handledLines.insert(j);
			}
		}

		m_ui->tableWidgetFiles->insertRow(0);
		const AutoSaveData &filename = m_autoSaveData[i];

		QTableWidgetItem *itemFileName = new QTableWidgetItem(filename.m_fileName);
		m_ui->tableWidgetFiles->setItem(0, AC_FileName, itemFileName);
		QFont font;
		font.setBold(true);
		itemFileName->setFont(font);

		QComboBox *cb = m_ui->comboBoxTimestamps;
		cb->clear();
		for (const std::pair<QDateTime, unsigned int> &dt : dateTimes) {
			cb->addItem(dt.first.toString());
			cb->setItemData(cb->count()-1, dt.second, Qt::UserRole); // cache the current vector idx
		}

		QTableWidgetItem *itemBasePath = new QTableWidgetItem(filename.m_basePath);
		m_ui->tableWidgetFiles->setItem(0, AC_BasePath, itemBasePath);

		QFont fontItalic;
		fontItalic.setItalic(true);
		itemBasePath->setFont(fontItalic);
	}

	bool takeLatest = m_ui->radioButtonLatestAutoSave->isChecked();
	m_ui->comboBoxTimestamps->setEnabled(!takeLatest && !m_autoSaveData.empty());
	if(takeLatest)
		m_ui->comboBoxTimestamps->setCurrentIndex(m_ui->comboBoxTimestamps->count()-1);

	m_ui->tableWidgetFiles->setCurrentCell(0,0);
}


void SVAutoSaveDialog::writeAutoSaveData() {
	FUNCID(SVAutoSaveDialog::writeAutoSaveData);

	QString userDir = QtExt::Directories::userDataDir();

	// Path to autosaves
	QString autoSavesMetaDataName = userDir + "/autosaves/autosave-metadata.info";

	QString info(autoSavesMetaDataName);
	QFile autoSaveInformation(info);

	if(!autoSaveInformation.open(QIODevice::WriteOnly | QIODevice::Text)) {
		IBK::IBK_Message(IBK::FormatString("Cannot create autosave metadata file '%1' (path does not exist or missing permissions).")
						 .arg(autoSavesMetaDataName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
		return;
	}

	// Check whether file is empty
	QTextStream out(&autoSaveInformation);
	out << "Filename\tHash\tTimestamp\tPath\n";

	for(unsigned int i=0; i<m_autoSaveData.size(); ++i) {
		out << m_autoSaveData[i].m_fileName << "\t" << m_autoSaveData[i].m_hash
			<< "\t" << m_autoSaveData[i].m_timeStamp << "\t" << m_autoSaveData[i].m_basePath << "\n";
	}
	autoSaveInformation.close();
}

void SVAutoSaveDialog::updateUi() {
	// Update Auto-saves in tables
	updateAutoSavesInTable();

	int rowCount = m_ui->tableWidgetFiles->rowCount();

	m_ui->pushButtonRecoverFile->setEnabled(rowCount != 0);
	m_ui->pushButtonRemoveAutoSave->setEnabled(rowCount != 0);
}

void SVAutoSaveDialog::removeProjectFiles(const QString &basePath, const QString &projectName) {
	if(!checkForAutoSaves())
		return;

	for (unsigned int i=m_autoSaveData.size(); i > 0; --i) {
		const AutoSaveData &data = m_autoSaveData[i-1];

		if((data.m_fileName != "UnnamedProject" &&
			(data.m_basePath != basePath)) || (data.m_fileName != projectName))
			continue;

		QString tempName = data.m_fileName;

		if(tempName.endsWith(".vicus"))
			tempName = tempName.remove(".vicus");

		QString filename = QtExt::Directories::userDataDir() + "/autosaves/" + tempName + "(" + data.m_hash + ").vicus.bak";
		// remove file
		QFile::remove(filename);
		// delete idx in vector
		m_autoSaveData.erase(m_autoSaveData.begin() + i);
	}

	writeAutoSaveData();
}

bool SVAutoSaveDialog::recoverFile(){
	FUNCID(SVAutoSaveDialog::recoverFile);

	unsigned int currentIdx = 0;
	bool ok = false;
	QComboBox *cb = m_ui->comboBoxTimestamps;
	Q_ASSERT(cb != nullptr);
	currentIdx = cb->currentData().toUInt(&ok);

	// Conversion error
	if(!ok) {
		IBK::IBK_Message(IBK::FormatString("Error in converting timestamp data."), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	Q_ASSERT(currentIdx < m_autoSaveData.size());

	const AutoSaveData &data = m_autoSaveData[currentIdx];

	QString tempName = data.m_fileName;

	if(tempName.endsWith(".vicus"))
		tempName = tempName.remove(".vicus");

	QString filename = QtExt::Directories::userDataDir() + "/autosaves/" + tempName + "(" + data.m_hash + ").vicus.bak";
	//filename.remove(".bak");

	QFile file(filename);
	if(!file.exists()) {
		QMessageBox::critical(this, tr("Error recovering autosave."), tr("Could not recover autosaved file '%1'.\nFile does not exist anymore. Entries will be removed.").arg(data.m_fileName));
		return false;
	}

	VICUS::Project prj;
	try {
		prj.readXML(IBK::Path(filename.toStdString()));
	}
	catch(IBK::Exception &ex) {
		IBK::IBK_Message(IBK::FormatString("Could not recover autosaved file."));
		return false;
	}

	// strip ending
	QString newProjectName = "(Autosaved)" + data.m_fileName;

	if (newProjectName.endsWith(".vicus"))
		newProjectName.remove(".vicus");

	newProjectName += ".vicus";

	QString newFilename = data.m_basePath + "/" + newProjectName;
	file.rename(newFilename);

	// ask user for filename
	QString restoredFileName = QFileDialog::getSaveFileName(
				this,
				tr("Specify SIM-VICUS project file"),
				newFilename,
				tr("SIM-VICUS project files (*%1);;All files (*.*)").arg(SVSettings::instance().m_projectFileSuffix),
				nullptr,
				SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
															);
	QString fnamebase = QFileInfo(restoredFileName).baseName();
	if (fnamebase.isEmpty()) {
		QMessageBox::critical(this, tr("Invalid file name"), tr("Please enter a valid file name!"));
		recoverFile(); // reenter correct name
	}

	// copy file
	file.copy(restoredFileName);
	file.remove();

	SVProjectHandler::instance().loadProject(this, restoredFileName, true);
	accept();

	// to make compiler happy
	return true;
}


void SVAutoSaveDialog::removeAutosave() {
	QComboBox *cb = m_ui->comboBoxTimestamps;
	Q_ASSERT(cb != nullptr);

	for(unsigned int i = (unsigned int)cb->count(); i > 0; --i) {
		unsigned int idx = cb->itemData(i-1).toUInt();

		const AutoSaveData &data = m_autoSaveData[idx];

		QString tempName = data.m_fileName;

		if(tempName.endsWith(".vicus"))
			tempName = tempName.remove(".vicus");

		QString filename = QtExt::Directories::userDataDir() + "/autosaves/" + tempName + "(" + data.m_hash + ").vicus.bak";
		// remove file
		QFile::remove(filename);
		// delete idx in vector
		m_autoSaveData.erase(m_autoSaveData.begin() + idx);
	}
}


void SVAutoSaveDialog::on_pushButtonRecoverFile_pressed() {
	// Recovers file and cloes
	if(recoverFile())
		return;

	// Remove current auto saves
	removeAutosave();

	// Update Ui with cleanded auto saves
	updateUi();

	// Write updated auto-save data
	writeAutoSaveData();
}


void SVAutoSaveDialog::on_pushButtonRemoveAutoSave_clicked() {
	// Remove auto-save file
	removeAutosave();

	// update ui
	updateUi();

	// write updated file
	writeAutoSaveData();
}


void SVAutoSaveDialog::on_pushButtonDiscard_clicked() {
	reject();
}


void SVAutoSaveDialog::on_radioButtonLatestAutoSave_toggled(bool checked){
	m_showOnlyLatestAutosave = checked;

	// Update Ui
	updateUi();
}
