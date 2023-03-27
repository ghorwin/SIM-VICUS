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

	m_timer = new QTimer;
	//m_timer->setInterval(AUTO_SAVE_INTERVALL);

	connect(m_timer, &QTimer::timeout, this, &SVAutoSaveDialog::onTimerFinished);

	m_timer->start(AUTO_SAVE_INTERVALL);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetFiles);

	m_ui->tableWidgetFiles->setColumnCount(4);
	m_ui->tableWidgetFiles->setHorizontalHeaderLabels(QStringList() << "Filename" << "Hash" << "Basepath" << "Savetime");
}

SVAutoSaveDialog::~SVAutoSaveDialog() {}

bool SVAutoSaveDialog::checkForAutoSaves() {
	FUNCID(SVAutoSaveDialog::checkForAutoSaves);

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

	if(lines.empty())
		return false;

	// Remove header
	lines.erase(lines.begin());

	// QStringList files = autoSavesDir.entryList(QDir::Files);

	std::vector<std::string> tokens;
	for(unsigned int i=0; i < lines.size(); ++i) {
		// Current line
		const std::string &line = lines[i];

		// split columns
		IBK::explode(line, tokens, "\t", IBK::EF_NoFlags | IBK::EF_TrimTokens | IBK::EF_KeepEmptyTokens);

		// FileName
		const QString fileName = autoSavesDirName + "/" + QString::fromStdString(tokens[AC_FileName]);
		QFile file(fileName);

		// Check if file can be opened
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			IBK::IBK_Message(IBK::FormatString("Cannot read auto-save file '%1' (path does not exist or missing permissions).")
							 .arg(fileName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
			continue;
		}

		const QString projectName = QString::fromStdString(tokens[0]);
		const QString hash        = QString::fromStdString(tokens[1]);
		const QString timeStamp   = QString::fromStdString(tokens[2]);
		const QString basePath    = QString::fromStdString(tokens[3]);

		m_autoSaveData.push_back(AutoSaveData(projectName, hash, timeStamp, basePath));
	}

	return true;
}

void SVAutoSaveDialog::removeProjectSepcificAutoSaves(const QString &projectName) {
	QString userDir = QtExt::Directories::userDataDir();

	// Path to autosaves
	QString autoSavesDir = userDir + "/autosaves/";

	// Construct Directory
	QDir autoSavesDirectory(autoSavesDir);

	// Remove all autosaves
	autoSavesDirectory.removeRecursively();
}

void SVAutoSaveDialog::handleAutoSaves() {
	if(!checkForAutoSaves())
		return; // no aute-saves have been found

	// Read meta-data
	updateAutoSavesInTable();

	exec();
}

void SVAutoSaveDialog::updateAutoSavesInTable() {

	m_ui->tableWidgetFiles->setRowCount(0);

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

		QTableWidgetItem *itemHash = new QTableWidgetItem(filename.m_hash);
		m_ui->tableWidgetFiles->setItem(0, AC_Hash, itemHash);

		QComboBox *cb = new QComboBox;
		for (const std::pair<QDateTime, unsigned int> &dt : dateTimes) {
			cb->addItem(dt.first.toString());
			cb->setItemData(cb->count()-1, dt.second, Qt::UserRole); // cache the current vector idx
		}

		cb->setCurrentIndex(cb->count()-1);
		m_ui->tableWidgetFiles->setCellWidget(0, AC_TimeStamp, cb);

		QTableWidgetItem *itemBasePath = new QTableWidgetItem(filename.m_basePath);
		m_ui->tableWidgetFiles->setItem(0, AC_BasePath, itemBasePath);

		QFont fontItalic;
		fontItalic.setItalic(true);
		itemBasePath->setFont(fontItalic);
	}

	m_ui->tableWidgetFiles->setCurrentCell(0,0);

	m_ui->tableWidgetFiles->hideColumn(AC_Hash);
	m_ui->tableWidgetFiles->resizeColumnsToContents();
	m_ui->tableWidgetFiles->horizontalHeader()->setStretchLastSection(true);
}

void SVAutoSaveDialog::writeAutoSaveData() {
	FUNCID(SVAutoSaveDialog::writeAutoSaveData);

	QString userDir = QtExt::Directories::userDataDir();

	// Path to autosaves
	QString autoSavesMetaDataName = userDir + "/autosaves/autosave-metadata.info";

	QString info(autoSavesMetaDataName);
	QFile autoSaveInformation(info);
	unsigned int size = autoSaveInformation.size();

	if(!autoSaveInformation.open(QIODevice::WriteOnly | QIODevice::Text)) {
		IBK::IBK_Message(IBK::FormatString("Cannot create autosave metadata file '%1' (path does not exist or missing permissions).")
						 .arg(autoSavesMetaDataName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
		return;
	}

	// Check whether file is empty
	QTextStream out(&autoSaveInformation);
	out << "FileName\tHash\tTimestamp\tPath\n";

	for(unsigned int i=0; i<m_autoSaveData.size(); ++i) {
		out << m_autoSaveData[i].m_fileName << "\t" << m_autoSaveData[i].m_hash
			<< "\t" << m_autoSaveData[i].m_timeStamp << "\t" << m_autoSaveData[i].m_basePath << "\n";
	}
	autoSaveInformation.close();
}

void SVAutoSaveDialog::onTimerFinished() {
	m_timer->start(AUTO_SAVE_INTERVALL);
	emit autoSave();
}

void SVAutoSaveDialog::on_pushButtonRecoverFile_pressed() {
	SVProjectHandler::instance();

	int currentRow = m_ui->tableWidgetFiles->currentRow();

	QComboBox *cb = dynamic_cast<QComboBox*>(m_ui->tableWidgetFiles->cellWidget(currentRow, AC_TimeStamp));
	Q_ASSERT(cb != nullptr);

	unsigned int currentIdx = cb->currentData().toUInt();

	Q_ASSERT(currentIdx < m_autoSaveData.size());

	const AutoSaveData &data = m_autoSaveData[currentIdx];

	QString tempName = data.m_fileName;

	if(tempName.endsWith(".vicus"))
		tempName = tempName.remove(".vicus");

	QString filename = QtExt::Directories::userDataDir() + "/autosaves/" + tempName + "(" + data.m_hash + ").vicus.bak";
	//filename.remove(".bak");

	QFile file(filename);
	if(!file.exists())
		return;

	VICUS::Project prj;
	try {
		prj.readXML(IBK::Path(filename.toStdString()));
	}
	catch(IBK::Exception &ex) {
		IBK::IBK_Message(IBK::FormatString("Could not recover autosaved file."));
		return;
	}

	// strip ending
	QString newFilename = data.m_basePath + "/" +data.m_fileName;
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

	// check if we have an existing and valid filename
	if (restoredFileName.isEmpty())
		return; // cancelled

	QString fnamebase = QFileInfo(restoredFileName).baseName();
	if (fnamebase.isEmpty()) {
		QMessageBox::critical(this, tr("Invalid file name"), tr("Please enter a valid file name!"));
		return;
	}

	// copy file
	file.copy(restoredFileName);
	close();

	// Remove current auto saves
	on_pushButtonRemoveAutoSave_clicked();
}


void SVAutoSaveDialog::on_pushButtonRemoveAutoSave_clicked() {
	int currentRow = m_ui->tableWidgetFiles->currentRow();

	QComboBox *cb = dynamic_cast<QComboBox*>(m_ui->tableWidgetFiles->cellWidget(currentRow, AC_TimeStamp));
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

		m_autoSaveData.erase(m_autoSaveData.begin() + idx);
	}

	updateAutoSavesInTable();

	// write updated file
	writeAutoSaveData();
}

