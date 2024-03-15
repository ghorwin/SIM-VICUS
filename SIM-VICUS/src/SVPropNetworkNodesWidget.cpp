#include "SVPropNetworkNodesWidget.h"
#include "ui_SVPropNetworkNodesWidget.h"

#include "SVPropNetworkEditWidget.h"

#include "VICUS_Project.h"

SVPropNetworkNodesWidget::SVPropNetworkNodesWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkNodesWidget),
	m_pa(qobject_cast<SVPropNetworkEditWidget*>(parent))
{
	m_ui->setupUi(this);

	// setup combobox node types
	m_ui->comboBoxNodeType->clear();
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Mixer),
									VICUS::NetworkNode::NT_Mixer);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Source),
									VICUS::NetworkNode::NT_Source);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_SubStation),
									VICUS::NetworkNode::NT_SubStation);

	// validating line edits
	m_ui->lineEditNodeMaximumHeatingDemand->setup(0, std::numeric_limits<double>::max(), tr("Maximum Heating Demand"), false, true);
	m_ui->lineEditNodeXPosition->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("x position of node"), true, true);
	m_ui->lineEditNodeYPosition->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("y position of node"), true, true);
	m_ui->lineEditNodeZPosition->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("z position of node"), true, true);

	m_ui->frameSource->setStyleSheet(".QFrame { background-color: #e68a00; }"); //QColor(230, 138, 0), orange
	m_ui->frameMixer->setStyleSheet(".QFrame { background-color: #77b300; }"); //QColor(119, 179, 0), green
	m_ui->frameSubStation->setStyleSheet(".QFrame { background-color: #006bb3; }"); // QColor(0, 107, 179); // blue

	m_ui->lineEditNodeId->setReadOnly(true);

}

SVPropNetworkNodesWidget::~SVPropNetworkNodesWidget()
{
	delete m_ui;
}

void SVPropNetworkNodesWidget::updateUi() {

	// enable / disable widgets
	bool uniformNodeType = m_pa->uniformProperty(m_pa->m_currentNodes, &VICUS::NetworkNode::m_type);
	m_ui->groupBoxEditNode->setEnabled(uniformNodeType && m_pa->m_currentEdges.empty() && m_pa->m_activeNetworkSelected);

	m_ui->groupBoxPosition->setEnabled(m_pa->m_currentNodes.size() == 1 && m_pa->m_currentEdges.empty() && m_pa->m_activeNetworkSelected);

	m_ui->lineEditNodeId->clear();
	m_ui->lineEditNodeName->clear();
	m_ui->lineEditNodeXPosition->clear();
	m_ui->lineEditNodeYPosition->clear();
	m_ui->lineEditNodeZPosition->clear();
	m_ui->lineEditNodeMaximumHeatingDemand->clear();

	if (m_pa->m_currentNodes.empty())
		return;

	// if node type is not uniform, no editing will be allowed
	m_ui->comboBoxNodeType->setCurrentIndex(m_ui->comboBoxNodeType->findData(m_pa->m_currentNodes[0]->m_type));
	m_ui->lineEditNodeMaximumHeatingDemand->setEnabled(m_pa->m_currentNodes[0]->m_type == VICUS::NetworkNode::NT_SubStation);

	if (m_pa->m_currentNodes.size() == 1){
		m_ui->lineEditNodeId->setText(QString("%1").arg(m_pa->m_currentNodes[0]->m_id));
		m_ui->lineEditNodeName->setText(m_pa->m_currentNodes[0]->m_displayName);
		m_ui->lineEditNodeXPosition->setValue(m_pa->m_currentNodes[0]->m_position.m_x);
		m_ui->lineEditNodeYPosition->setValue(m_pa->m_currentNodes[0]->m_position.m_y);
		m_ui->lineEditNodeZPosition->setValue(m_pa->m_currentNodes[0]->m_position.m_z);
	}

	if (m_pa->uniformProperty(m_pa->m_currentNodes, &VICUS::NetworkNode::m_maxHeatingDemand))
		m_ui->lineEditNodeMaximumHeatingDemand->setValue(m_pa->m_currentNodes[0]->m_maxHeatingDemand.value / 1000);
}


void SVPropNetworkNodesWidget::clearUi() {
	m_ui->lineEditNodeId->clear();
	m_ui->lineEditNodeXPosition->clear();
	m_ui->lineEditNodeYPosition->clear();
	m_ui->lineEditNodeZPosition->clear();
	m_ui->lineEditNodeName->clear();
	m_ui->lineEditNodeMaximumHeatingDemand->clear();
}


void SVPropNetworkNodesWidget::setWidgetsEnabled(bool enabled) {
	m_ui->groupBoxEditNode->setEnabled(enabled);
	m_ui->groupBoxPosition->setEnabled(enabled);
}


void SVPropNetworkNodesWidget::on_comboBoxNodeType_activated(int index) {
	m_pa->modifyNodeProperty(&VICUS::NetworkNode::m_type, VICUS::NetworkNode::NodeType(
		m_ui->comboBoxNodeType->currentData().toUInt()) );
}


void SVPropNetworkNodesWidget::on_lineEditNodeXPosition_editingFinishedSuccessfully() {
	if (!m_ui->lineEditNodeXPosition->isValid() || !m_ui->lineEditNodeYPosition->isValid() || !m_ui->lineEditNodeZPosition->isValid())
		return;
	IBKMK::Vector3D	vec(m_ui->lineEditNodeXPosition->value(), m_ui->lineEditNodeYPosition->value(), m_ui->lineEditNodeZPosition->value());
	m_pa->modifyNodeProperty(&VICUS::NetworkNode::m_position, vec);
}


void SVPropNetworkNodesWidget::on_lineEditNodeYPosition_editingFinishedSuccessfully() {
	if (!m_ui->lineEditNodeXPosition->isValid() || !m_ui->lineEditNodeYPosition->isValid() || !m_ui->lineEditNodeZPosition->isValid())
		return;
	IBKMK::Vector3D	vec(m_ui->lineEditNodeXPosition->value(), m_ui->lineEditNodeYPosition->value(), m_ui->lineEditNodeZPosition->value());
	m_pa->modifyNodeProperty(&VICUS::NetworkNode::m_position, vec);
}

void SVPropNetworkNodesWidget::on_lineEditNodeZPosition_editingFinishedSuccessfully() {
	if (!m_ui->lineEditNodeXPosition->isValid() || !m_ui->lineEditNodeYPosition->isValid() || !m_ui->lineEditNodeZPosition->isValid())
		return;
	IBKMK::Vector3D	vec(m_ui->lineEditNodeXPosition->value(), m_ui->lineEditNodeYPosition->value(), m_ui->lineEditNodeZPosition->value());
	m_pa->modifyNodeProperty(&VICUS::NetworkNode::m_position, vec);
}


void SVPropNetworkNodesWidget::on_lineEditNodeMaximumHeatingDemand_editingFinishedSuccessfully() {
	if (m_ui->lineEditNodeMaximumHeatingDemand->isValid())
		m_pa->modifyNodeProperty(&VICUS::NetworkNode::m_maxHeatingDemand,
						   IBK::Parameter("MaxHeatingDemand", m_ui->lineEditNodeMaximumHeatingDemand->value()*1000, "W"));
}

void SVPropNetworkNodesWidget::on_lineEditNodeName_editingFinished()
{
	m_pa->modifyNodeProperty(&VICUS::NetworkNode::m_displayName, m_ui->lineEditNodeName->text());
}




