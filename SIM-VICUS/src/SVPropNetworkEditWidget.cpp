#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoModifyExistingNetwork.h"
#include "SVProjectHandler.h"


SVPropNetworkEditWidget::SVPropNetworkEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEditWidget)
{
	m_ui->setupUi(this);

	m_ui->groupBoxNetwork->setVisible(false);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
}


SVPropNetworkEditWidget::~SVPropNetworkEditWidget() {
	delete m_ui;
}


void SVPropNetworkEditWidget::updateUi() {
	// get currently selected network
	unsigned int selectedNetworkUniqueId = SVViewStateHandler::instance().m_navigationTreeWidget->selectedNodeID();

	VICUS::Project p = project();
	m_network = p.element(p.m_geometricNetworks, selectedNetworkUniqueId);
	if (m_network == nullptr)
		return;

	m_ui->labelEdgeCount->setText(QString("%1").arg(m_network->m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(m_network->m_nodes.size()));
	if (m_network->checkConnectedGraph()){
		m_ui->labelNetworkConnected->setText("Network is connected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: green}");
	}
	else{
		m_ui->labelNetworkConnected->setText("Network is unconnected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: red}");
	}

	m_ui->labelTotalLength->setText(QString("%1").arg(m_network->totalLength()));
	m_ui->pushButtonConnectBuildings->setEnabled(m_network->nextUnconnectedBuilding()>=0);
	m_ui->pushButtonReduceDeadEnds->setEnabled(m_network->checkConnectedGraph() && m_network->numberOfBuildings() > 0);
	m_ui->labelLargestDiameter->setText(QString("%1").arg(m_network->largestDiameter()));
	m_ui->labelSmallestDiameter->setText(QString("%1").arg(m_network->smallestDiameter()));
	m_ui->checkBoxVisible->setChecked(m_network->m_visible);

}

void SVPropNetworkEditWidget::showNetworkProperties()
{
	m_ui->groupBoxNetwork->setVisible(true);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
}

void SVPropNetworkEditWidget::showNodeProperties()
{
	m_ui->groupBoxNetwork->setVisible(false);
	m_ui->groupBoxNode->setVisible(true);
	m_ui->groupBoxEdge->setVisible(false);
}

void SVPropNetworkEditWidget::showEdgeProperties()
{
	m_ui->groupBoxNetwork->setVisible(false);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(true);
}
