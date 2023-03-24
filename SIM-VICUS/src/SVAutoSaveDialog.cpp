#include "SVAutoSaveDialog.h"
#include "ui_SVAutoSaveDialog.h"

#include <SVConstants.h>
#include <SVStyle.h>
#include <SVProjectHandler.h>

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

	m_ui->tableWidgetFiles->setHorizontalHeaderLabels(QStringList() << "Recovered Files");
	m_ui->tableWidgetFiles->horizontalHeader()->setStretchLastSection(true);
}

SVAutoSaveDialog::~SVAutoSaveDialog() {}

bool SVAutoSaveDialog::checkForAutoSaves(std::vector<QString> &autoSaves) {
	QString userDir = QtExt::Directories::userDataDir();

	// Path to autosaves
	QString autoSavesDirName = userDir + "/autosaves/";

	// QDir
	QDir autoSavesDir(autoSavesDirName);

	if(!autoSavesDir.exists())
		return false;

	if(autoSavesDir.isEmpty())
		return false;

	QStringList files = autoSavesDir.entryList(QDir::Files);
	for(const QString &file : qAsConst(files))
		autoSaves.push_back(file);

	return true;
}

void SVAutoSaveDialog::removeAutoSaves() {
	QString userDir = QtExt::Directories::userDataDir();

	// Path to autosaves
	QString autoSavesDir = userDir + "/autosaves/";

	// Construct Directory
	QDir autoSavesDirectory(autoSavesDir);

	// Remove all autosaves
	autoSavesDirectory.removeRecursively();
}

void SVAutoSaveDialog::handleAutoSaves() {
	std::vector<QString> files;
	if(!checkForAutoSaves(files))
		return;

	m_ui->tableWidgetFiles->setColumnCount(1);
	m_ui->tableWidgetFiles->setRowCount(files.size());
	for (unsigned int i=0; i<files.size(); ++i) {
		const QString &filename = files[i];

		QTableWidgetItem *item = new QTableWidgetItem(filename);
		m_ui->tableWidgetFiles->setItem(i, 0, item);
	}
	m_ui->tableWidgetFiles->setCurrentCell(0,0);

	exec();
}

void SVAutoSaveDialog::onTimerFinished() {
	m_timer->start(AUTO_SAVE_INTERVALL);
	emit autoSave();
}

void SVAutoSaveDialog::on_pushButtonRecoverFile_pressed() {
	SVProjectHandler::instance();

	QTableWidgetItem *item = m_ui->tableWidgetFiles->currentItem();
	Q_ASSERT(item != nullptr);

	QString filename = QtExt::Directories::userDataDir() + "/autosaves/" + item->text();
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
	int last = filename.lastIndexOf("(");
	QString newFilename = filename.mid(0, last) + SVSettings::instance().m_projectFileSuffix;

	file.rename(newFilename);

	// ask user for filename
	SVProjectHandler::instance().loadProject(this, newFilename, true);

	close();
}

