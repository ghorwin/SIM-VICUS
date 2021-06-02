#include "SVNetworkControllerDialog.h"
#include "ui_SVNetworkControllerDialog.h"

#include "SVProjectHandler.h"
#include "SVUndoModifyNetwork.h"

#include <VICUS_Network.h>
#include <VICUS_Project.h>
#include <VICUS_KeywordList.h>

#include <algorithm>

SVNetworkControllerDialog::SVNetworkControllerDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkControllerDialog)
{
	m_ui->setupUi(this);

	m_ui->comboBoxProperty->blockSignals(true);
	m_ui->comboBoxProperty->clear();
	for (int i = 0; i < VICUS::NetworkController::NUM_CP; ++i) {
		m_ui->comboBoxProperty->addItem(VICUS::KeywordList::Keyword("NetworkController::ControlledProperty", i), i);
	}
	m_ui->comboBoxProperty->blockSignals(false);

	m_ui->comboBoxControllerType->blockSignals(true);
	m_ui->comboBoxControllerType->clear();
	for (int i = 0; i < 1; ++i) {
		m_ui->comboBoxControllerType->addItem(VICUS::KeywordList::Keyword("NetworkController::ControllerType", i), i);
	}
	m_ui->comboBoxControllerType->blockSignals(false);


}

SVNetworkControllerDialog::~SVNetworkControllerDialog()
{
	delete m_ui;
}

unsigned int SVNetworkControllerDialog::select(unsigned int networkId, unsigned int controllerId)
{
	VICUS::Project p = project();

	Q_ASSERT(p.element(project().m_geometricNetworks, networkId) != nullptr);
	VICUS::Network nw = *p.element(project().m_geometricNetworks, networkId);
	VICUS::NetworkController *currentController = p.element(nw.m_controllers, controllerId);

	if (currentController != nullptr)
		m_controller = *currentController;
	else {
		m_controller = VICUS::NetworkController();
		m_controller.m_id = 1;
	}

	update();

	int res = exec();
	if (res != QDialog::Accepted)
		return 0;

	nw.m_controllers.clear();
	nw.m_controllers.push_back(m_controller);

	// undo action
	nw.updateNodeEdgeConnectionPointers();

	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(),
											  project().element(project().m_geometricNetworks, networkId));
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, nw);
	undo->push(); // modifies project and updates views

	return m_controller.m_id;
}


void SVNetworkControllerDialog::update()
{
	m_ui->lineEditName->setString(m_controller.m_displayName);
	m_ui->lineEditSetpoint->setValue(m_controller.m_setPoint.value);
	m_ui->lineEditKp->setValue(m_controller.m_para[VICUS::NetworkController::P_Kp].value);
	m_ui->lineEditKi->setValue(m_controller.m_para[VICUS::NetworkController::P_Ki].value);
	int propIdx = m_ui->comboBoxProperty->findData(m_controller.m_controlledProperty);
	m_ui->comboBoxProperty->setCurrentIndex(propIdx);
	int typeIdx = m_ui->comboBoxControllerType->findData(m_controller.m_controllerType);
	m_ui->comboBoxControllerType->setCurrentIndex(typeIdx);
}



void SVNetworkControllerDialog::on_lineEditName_editingFinished()
{
	m_controller.m_displayName = m_ui->lineEditName->string();
}

void SVNetworkControllerDialog::on_lineEditSetpoint_editingFinished()
{
	if (m_ui->lineEditSetpoint->isValid())
		m_controller.m_setPoint.set("SetPoint", m_ui->lineEditSetpoint->value(), "---");
}

void SVNetworkControllerDialog::on_lineEditKp_editingFinished()
{
	if (m_ui->lineEditKp->isValid())
		VICUS::KeywordList::setParameter(m_controller.m_para, "NetworkController::para_t",
										 VICUS::NetworkController::P_Kp, m_ui->lineEditKp->value());
}

void SVNetworkControllerDialog::on_lineEditKi_editingFinished()
{
	if (m_ui->lineEditKi->isValid())
		VICUS::KeywordList::setParameter(m_controller.m_para, "NetworkController::para_t",
										 VICUS::NetworkController::P_Ki, m_ui->lineEditKi->value());
}

void SVNetworkControllerDialog::on_comboBoxProperty_currentIndexChanged(int /*index*/)
{
	m_controller.m_controlledProperty =
	VICUS::NetworkController::ControlledProperty(m_ui->comboBoxProperty->currentData().toInt());
}

void SVNetworkControllerDialog::on_comboBoxControllerType_currentIndexChanged(int /*index*/)
{
	m_controller.m_controllerType =
	VICUS::NetworkController::ControllerType(m_ui->comboBoxControllerType->currentData().toInt());
}
