#include "SVLogFileDialog.h"
#include "ui_SVLogFileDialog.h"

#include <QFileInfo>
#include <QPushButton>
#include <QDialogButtonBox>

#include "SVSettings.h"
#include "SVProjectHandler.h"

SVLogFileDialog::SVLogFileDialog(QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
	m_ui(new Ui::SVLogFileDialog),
	m_pushButtonOpenFileInTextEditor(nullptr),
	m_pushButtonReloadProject(nullptr)
{
	m_ui->setupUi(this);
	resize(1400,600);

	m_pushButtonOpenFileInTextEditor = m_ui->buttonBox->addButton(tr("Open file in text editor..."), QDialogButtonBox::ActionRole);
	m_pushButtonReloadProject = m_ui->buttonBox->addButton(tr("Reload project"), QDialogButtonBox::ActionRole);

	connect(m_pushButtonOpenFileInTextEditor, SIGNAL(clicked(bool)),
			this, SLOT(onOpenFileClicked(bool)));
	connect(m_pushButtonReloadProject, SIGNAL(clicked(bool)),
			this, SLOT(onReloadprojectClicked(bool)));
}


SVLogFileDialog::~SVLogFileDialog() {
	delete m_ui;
}


void SVLogFileDialog::setLogFile(const QString & logfilepath, QString projectfilepath, bool editFileButtonVisible) {
	m_logFilePath = logfilepath;
	m_projectFilePath = projectfilepath;
	m_ui->logWidget->showLogFile(m_logFilePath);
	setWindowTitle(QFileInfo(m_logFilePath).fileName());
	if (editFileButtonVisible) {
		m_pushButtonOpenFileInTextEditor->setVisible(true);
		m_pushButtonReloadProject->setVisible(true);
		m_ui->labelOpenFileError->setVisible(true);
		m_ui->labelOpenFileError->setText(tr("Error opening file '%1'.").arg(projectfilepath));
	}
	else {
		m_pushButtonOpenFileInTextEditor->setVisible(false);
		m_pushButtonReloadProject->setVisible(false);
		m_ui->labelOpenFileError->setVisible(false);
	}
}


void SVLogFileDialog::onOpenFileClicked(bool /*checked*/) {
	SVSettings::instance().openFileInTextEditor(this, m_projectFilePath);
}

void SVLogFileDialog::onReloadprojectClicked(bool /*checked*/) {
	const char * const FUNC_ID = "[SVLogFileDialog::onReloadprojectClicked]";

	SVProjectHandler::instance().setReload();
	IBK::IBK_Message("\n------------------------------------------------------\n",IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message(IBK::FormatString("Reload project '%1'\n").arg(IBK::Path(m_projectFilePath.toStdString())),IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("------------------------------------------------------\n\n",IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	close();
}
