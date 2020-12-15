#include "SVSimulationStartNetworkSim.h"
#include "ui_SVSimulationStartNetworkSim.h"

#include <QtExt_Directories.h>

#include <QFileInfo>

#include "SVSettings.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVSimulationStartNetworkSim::SVSimulationStartNetworkSim(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSimulationStartNetworkSim)
{
	m_ui->setupUi(this);

	connect(m_ui->pushButtonClose, &QPushButton::clicked,
			this, &SVSimulationStartNetworkSim::close);
}


SVSimulationStartNetworkSim::~SVSimulationStartNetworkSim() {
	delete m_ui;
}


void SVSimulationStartNetworkSim::edit() {
	// transfer network names to ui and select the first

	m_ui->comboBoxNetwork->clear();
	for (const VICUS::Network & n : project().m_geometricNetworks) {
		m_ui->comboBoxNetwork->addItem(QString::fromStdString(n.m_name));
	}
	m_ui->comboBoxNetwork->setCurrentIndex(0);
	updateCmdLine();
	exec();
}


void SVSimulationStartNetworkSim::on_checkBoxCloseConsoleWindow_toggled(bool checked) {
	updateCmdLine();
}


void SVSimulationStartNetworkSim::updateCmdLine() {
	m_cmdLine.clear();
	m_cmdLine << QFileInfo(SVSettings::instance().m_installDir + "/NandradSolver").filePath();
	if (m_ui->checkBoxCloseConsoleWindow->isChecked())
		m_cmdLine << "-x";

	QString targetFile = QFileInfo(SVProjectHandler::instance().projectFile()).completeBaseName();

	targetFile += ".nandrad";

	m_cmdLine << targetFile;
	m_ui->lineEditCmdLine->setText(m_cmdLine.join(" "));
	m_ui->lineEditCmdLine->setCursorPosition( m_ui->lineEditCmdLine->text().length() );
}


void SVSimulationStartNetworkSim::on_pushButtonRun_clicked() {

	// generate NANDRAD project

	// save project


	// launch solver


	close(); // finally close dialog
}
