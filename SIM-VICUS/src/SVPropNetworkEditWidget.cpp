#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoModifyExistingNetwork.h"
#include "SVProjectHandler.h"

#include "NANDRAD_HydraulicNetworkComponent.h"

#include "VICUS_KeywordList.h"


SVPropNetworkEditWidget::SVPropNetworkEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEditWidget)
{
	m_ui->setupUi(this);

	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);

	// setup combobox pipe models
	m_mapPipeModels.clear();
	m_mapPipeModels.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_StaticPipe),
														NANDRAD::HydraulicNetworkComponent::MT_StaticPipe);
	m_mapPipeModels.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe),
														NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe);
	m_ui->comboBoxPipeModel->clear();
	m_ui->comboBoxPipeModel->addItems(m_mapPipeModels.keys());

	// setup combobox node types
	m_mapNodeTypes.clear();
	m_mapNodeTypes.insert(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Mixer),
						  VICUS::NetworkNode::NT_Mixer);
	m_mapNodeTypes.insert(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Source),
						  VICUS::NetworkNode::NT_Source);
	m_mapNodeTypes.insert(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Building),
						  VICUS::NetworkNode::NT_Building);
	m_ui->comboBoxNodeType->clear();
	m_ui->comboBoxNodeType->addItems(m_mapNodeTypes.keys());
}


SVPropNetworkEditWidget::~SVPropNetworkEditWidget() {
	delete m_ui;
}


void SVPropNetworkEditWidget::updateUi() {

	// update network ui
	networkFromId();
	if (m_network != nullptr){
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
		m_ui->labelTotalLength->setText(QString("%1 m").arg(m_network->totalLength()));
		m_ui->pushButtonConnectBuildings->setEnabled(m_network->nextUnconnectedBuilding()>=0);
		m_ui->pushButtonReduceDeadEnds->setEnabled(m_network->checkConnectedGraph() && m_network->numberOfBuildings() > 0);
		m_ui->labelLargestDiameter->setText(QString("%1 mm").arg(m_network->largestDiameter()));
		m_ui->labelSmallestDiameter->setText(QString("%1 mm").arg(m_network->smallestDiameter()));
		m_ui->checkBoxVisible->setChecked(m_network->m_visible);

		updateSizingParams();
	}

	// update edge ui
	const VICUS::NetworkEdge *edge = dynamic_cast<const VICUS::NetworkEdge *>(m_obj);
	if (edge != nullptr){
		m_ui->labelPipeLength->setText(QString("%1 m").arg(edge->length()));
		m_ui->comboBoxPipeModel->setCurrentText(m_mapPipeModels.key(edge->m_modelType));
		setupComboboxPipeDB();
		m_ui->comboBoxPipeDB->setCurrentText(m_mapDBPipes.key(edge->m_pipeId));
		m_ui->checkBoxSupplyPipe->setChecked(edge->m_supply);
	}

	// update node ui
	const VICUS::NetworkNode *node = dynamic_cast<const VICUS::NetworkNode *>(m_obj);
	if (node != nullptr){
		setupComboboxComponents();
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(node->m_componentId));
		m_ui->comboBoxNodeType->setCurrentText(m_mapNodeTypes.key(node->m_type));
		m_ui->labelNodeId->setText(QString("%1").arg(node->m_id));
		m_ui->label->setText(QString("%1").arg(node->m_id));
		m_ui->doubleSpinBoxNodeNeatingDemand->setValue(node->m_maxHeatingDemand);
		m_ui->lineEditNodeX->setText(QString("%1").arg(node->m_position.m_x));
		m_ui->lineEditNodeY->setText(QString("%1").arg(node->m_position.m_y));
		m_ui->lineEditNodeZ->setText(QString("%1").arg(node->m_position.m_z));
	}
}


void SVPropNetworkEditWidget::updateSizingParams()
{
//	if (m_network->m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].empty())
//		m_network->setDefaultSizingParams();
	m_ui->doubleSpinBoxTemperatureSetpoint->setValue(m_network->m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].get_value("C"));
	m_ui->doubleSpinBoxTemperatureDifference->setValue(m_network->m_sizingPara[VICUS::Network::SP_TemperatureDifference].get_value("K"));
	m_ui->doubleSpinBoxMaximumPressureLoss->setValue(m_network->m_sizingPara[VICUS::Network::SP_MaxPressureLoss].get_value("Pa/m"));
}

void SVPropNetworkEditWidget::modifyUI()
{
	// get currently selected network object
	networkFromId();
	if (m_network == nullptr)
		return;
	VICUS::Network network = *m_network;

	network.m_visible = m_ui->checkBoxVisible->isChecked();
	network.m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].set(VICUS::KeywordList::Keyword("Network::SizingParam", VICUS::Network::SP_TemperatureSetpoint),
																	   m_ui->doubleSpinBoxTemperatureSetpoint->value(),
																	   IBK::Unit("C"));
	VICUS::KeywordList::setParameter(network.m_sizingPara, "Network::SizingParam", VICUS::Network::SP_TemperatureDifference,
									 m_ui->doubleSpinBoxTemperatureDifference->value());
	VICUS::KeywordList::setParameter(network.m_sizingPara, "Network::SizingParam", VICUS::Network::SP_MaxPressureLoss,
									 m_ui->doubleSpinBoxMaximumPressureLoss->value());

	const VICUS::NetworkEdge *edge = dynamic_cast<const VICUS::NetworkEdge *>(m_obj);
	if (edge != nullptr){
		VICUS::NetworkEdge *e = network.edge(edge->nodeId1(), edge->nodeId2());
		e->m_supply = m_ui->checkBoxSupplyPipe->isChecked();
		e->m_modelType = NANDRAD::HydraulicNetworkComponent::modelType_t(m_mapPipeModels.value(m_ui->comboBoxPipeModel->currentText()));
		e->m_pipeId = m_mapDBPipes.value(m_ui->comboBoxPipeDB->currentText());
	}

	const VICUS::NetworkNode *node = dynamic_cast<const VICUS::NetworkNode *>(m_obj);
	if (node != nullptr){
		VICUS::NetworkNode *n = &network.m_nodes[node->m_id];
		n->m_type = VICUS::NetworkNode::NodeType(m_mapNodeTypes.value(m_ui->comboBoxNodeType->currentText()));
		n->m_componentId = NANDRAD::HydraulicNetworkComponent::modelType_t(m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
		n->m_maxHeatingDemand = m_ui->doubleSpinBoxNodeNeatingDemand->value();
		n->m_position.m_x = QLocale().toDouble(m_ui->lineEditNodeX->text());
		n->m_position.m_y = QLocale().toDouble(m_ui->lineEditNodeY->text());
//		n->m_position.m_z = QLocale().toDouble(m_ui->lineEditNodeZ->text());
	}

	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::setupComboboxPipeDB()
{
	networkFromId();
	if (m_network == nullptr)
		return;
	m_mapDBPipes.clear();
	for (auto it = m_network ->m_networkPipeDB.begin(); it!= m_network ->m_networkPipeDB.end(); ++it){
		m_mapDBPipes.insert(QString::fromStdString(""+IBK::FormatString("%1 [%2 mm]").arg(it->m_displayName)
												   .arg(it->m_diameterOutside)), it->m_id);
	}
	m_ui->comboBoxPipeDB->clear();
	m_ui->comboBoxPipeDB->addItems(m_mapDBPipes.keys());
}


void SVPropNetworkEditWidget::setupComboboxComponents()
{
	networkFromId();
	if (m_network == nullptr)
		return;
	m_mapComponents.clear();
	for (const NANDRAD::HydraulicNetworkComponent &comp: m_network->m_hydraulicComponents)
		m_mapComponents.insert(QString::fromStdString(comp.m_displayName), comp.m_id);
	m_ui->comboBoxComponent->clear();
	m_ui->comboBoxComponent->addItems(m_mapComponents.keys());
}


void SVPropNetworkEditWidget::networkFromId()
{
	unsigned int id = SVViewStateHandler::instance().m_navigationTreeWidget->selectedNodeID();
	if (id != 0)
		m_treeItemId = id;
	const VICUS::Project &p = project();
	m_obj = p.objectById(m_treeItemId);
	if (dynamic_cast<const VICUS::Network *>(m_obj) != nullptr)
		m_network = dynamic_cast<const VICUS::Network *>(m_obj);
	else if (dynamic_cast<const VICUS::Network *>(m_obj->m_parent) != nullptr)
		m_network = dynamic_cast<const VICUS::Network *>(m_obj->m_parent);
	else
		return;
}


void SVPropNetworkEditWidget::showNetworkProperties()
{
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
}

void SVPropNetworkEditWidget::showNodeProperties()
{
	m_ui->groupBoxNode->setVisible(true);
	m_ui->groupBoxEdge->setVisible(false);
}

void SVPropNetworkEditWidget::showEdgeProperties()
{
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(true);
}

void SVPropNetworkEditWidget::on_comboBoxNodeType_activated(int index)
{
	modifyUI();
}

void SVPropNetworkEditWidget::on_comboBoxComponent_activated(int index)
{
	modifyUI();
}

void SVPropNetworkEditWidget::on_doubleSpinBoxNodeNeatingDemand_editingFinished()
{
	modifyUI();
}

void SVPropNetworkEditWidget::on_lineEditNodeX_editingFinished()
{
	modifyUI();
}

void SVPropNetworkEditWidget::on_comboBoxPipeModel_activated(int index)
{
	modifyUI();
}

void SVPropNetworkEditWidget::on_comboBoxPipeDB_activated(int index)
{
	modifyUI();
}

void SVPropNetworkEditWidget::on_checkBoxSupplyPipe_clicked()
{
	modifyUI();
}

void SVPropNetworkEditWidget::on_pushButtonSizePipeDimensions_clicked()
{

}

void SVPropNetworkEditWidget::on_pushButtonGenerateIntersections_clicked()
{

}

void SVPropNetworkEditWidget::on_pushButtonConnectBuildings_clicked()
{

}

void SVPropNetworkEditWidget::on_pushButtonReduceDeadEnds_clicked()
{

}

void SVPropNetworkEditWidget::on_pushButtonReduceRedundantNodes_clicked()
{

}

void SVPropNetworkEditWidget::on_lineEditNodeY_editingFinished()
{
	modifyUI();
}
