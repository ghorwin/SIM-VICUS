#include "SVPropNetworkSubStationWidget.h"
#include "ui_SVPropNetworkSubStationWidget.h"

#include <QtExt_Conversions.h>

#include "SVStyle.h"
#include "SVSettings.h"
#include "SVProjectHandler.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVUndoTreeNodeState.h"
#include "SVUndoModifyNetwork.h"
#include "SVPropNetworkEditWidget.h"


SVPropNetworkSubStationWidget::SVPropNetworkSubStationWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkSubStationWidget),
	m_pa(qobject_cast<SVPropNetworkEditWidget*>(parent))
{
	m_ui->setupUi(this);

	m_ui->tableWidgetSubNetworks->setColumnCount(2);
	m_ui->tableWidgetSubNetworks->setHorizontalHeaderLabels(QStringList() << QString() << tr("Sub Networks"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSubNetworks);
	m_ui->tableWidgetSubNetworks->setSortingEnabled(false);
	m_ui->tableWidgetSubNetworks->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetSubNetworks->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetSubNetworks->horizontalHeader()->setStretchLastSection(true);

	on_tableWidgetSubNetworks_itemSelectionChanged();
}


SVPropNetworkSubStationWidget::~SVPropNetworkSubStationWidget(){
	delete m_ui;
}


void SVPropNetworkSubStationWidget::updateUi() {
	m_ui->groupBoxSelectedSubNetwork->setEnabled(!m_pa->m_currentNodes.empty() && m_pa->m_currentEdges.empty());
	// current sub network name
	m_ui->labelSelectedSubNetwork->clear();
	const SVDatabase & db = SVSettings::instance().m_db;
	if (m_pa->uniformProperty(m_pa->m_currentNodes, &VICUS::NetworkNode::m_idSubNetwork)){
		const VICUS::SubNetwork *subNet = db.m_subNetworks[m_pa->m_currentNodes[0]->m_idSubNetwork];
		if (subNet != nullptr)
			m_ui->labelSelectedSubNetwork->setText(QtExt::MultiLangString2QString(subNet->m_displayName));
	}
}


void SVPropNetworkSubStationWidget::updateTableWidget() {

	//  *** Update sub networks table widget ***
	m_ui->groupBoxCurrentSubNetworks->setEnabled(true);

	const VICUS::Project &p=project();
	const SVDatabase & db = SVSettings::instance().m_db;

	std::vector<unsigned int> subNetworkIds;
	for (const VICUS::Network &net: p.m_geometricNetworks) {
		for (const VICUS::NetworkNode &n: net.m_nodes){
			if (std::find(subNetworkIds.begin(), subNetworkIds.end(), n.m_idSubNetwork) == subNetworkIds.end() &&
					n.m_idSubNetwork != VICUS::INVALID_ID)
				subNetworkIds.push_back(n.m_idSubNetwork);
		}
	}

	int currentRow = m_ui->tableWidgetSubNetworks->currentRow();
	m_ui->tableWidgetSubNetworks->blockSignals(true);
	m_ui->tableWidgetSubNetworks->clearContents();
	m_ui->tableWidgetSubNetworks->setRowCount((int)subNetworkIds.size());
	int row = 0;
	for (unsigned int id: subNetworkIds){
		 const VICUS::SubNetwork *subNet = db.m_subNetworks[id];
		 QTableWidgetItem * item = new QTableWidgetItem();
		 // special handling for components with "invalid" component id
		 if (subNet == nullptr)
			 item->setBackground(QColor(64,64,64));
		 else
			 item->setBackground(subNet->m_color);
		 item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		 item->setData(Qt::UserRole, id);
		 m_ui->tableWidgetSubNetworks->setItem(row, 0, item);

		 item = new QTableWidgetItem();
		 item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		 if (subNet == nullptr)
			 item->setText(tr("<invalid sub network id>"));
		 else
			 item->setText(QtExt::MultiLangString2QString(subNet->m_displayName));
		item->setData(Qt::UserRole, id);
		 m_ui->tableWidgetSubNetworks->setItem(row, 1, item);

		 ++row;
	}
	// reselect row
	m_ui->tableWidgetSubNetworks->blockSignals(false);
	m_ui->tableWidgetSubNetworks->selectRow(std::min(currentRow, m_ui->tableWidgetSubNetworks->rowCount()-1));
}


void SVPropNetworkSubStationWidget::clearUi() {
	m_ui->tableWidgetSubNetworks->clearContents();
}


void SVPropNetworkSubStationWidget::setWidgetsEnabled(bool enabled){
	m_ui->groupBoxSelectedSubNetwork->setEnabled(enabled);
	m_ui->groupBoxCurrentSubNetworks->setEnabled(enabled);
}


void SVPropNetworkSubStationWidget::on_pushButtonEditSubNetworks_clicked() {
	unsigned int currentId = 0;
	QTableWidgetItem *item = m_ui->tableWidgetSubNetworks->currentItem();
	if (item != nullptr)
		currentId = item->data(Qt::UserRole).toUInt();
	SVMainWindow::instance().dbSubNetworkEditDialog()->edit(currentId);
	updateTableWidget(); // for color update
}


void SVPropNetworkSubStationWidget::on_pushButtonExchangeSubNetwork_clicked() {

	// get id from current item
	QTableWidgetItem *item = m_ui->tableWidgetSubNetworks->currentItem();
	if (item == nullptr)
		return;
	unsigned int oldId = item->data(Qt::UserRole).toUInt();

	// get new id
	unsigned int newId = SVMainWindow::instance().dbSubNetworkEditDialog()->select(oldId);
	if (newId == oldId || newId==VICUS::INVALID_ID)
		return;

	VICUS::Project p = project();
	for (VICUS::Network &network: p.m_geometricNetworks) {
		// modify nodes
		for (VICUS::NetworkNode &n: network.m_nodes) {
			if (n.m_idSubNetwork == oldId)
				n.m_idSubNetwork = newId;
		}
		// undo
		SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), network);
		undo->push(); // modifies project and updates views
	}
	updateTableWidget(); // for color update
}


void SVPropNetworkSubStationWidget::on_pushButtonSelectNodesWithSubNetwork_clicked() {

	// get pipeId from current item
	QTableWidgetItem *item = m_ui->tableWidgetSubNetworks->currentItem();
	if (item == nullptr)
		return;
	unsigned int currentId = item->data(Qt::UserRole).toUInt();

	// collect sub networks
	std::set<unsigned int> nodeIds;
	const VICUS::Project &p = project();
	for (const VICUS::Network &network: p.m_geometricNetworks) {
		for (const VICUS::NetworkNode &n: network.m_nodes) {
			if (n.m_idSubNetwork == currentId)
				nodeIds.insert(n.m_id);
		}
	}

	const VICUS::SubNetwork * sub = SVSettings::instance().m_db.m_subNetworks[currentId];
	QString undoText;
	if (sub != nullptr)
		undoText = tr("Select nodes with sub network '%1'.").arg(QtExt::MultiLangString2QString(sub->m_displayName));
	else
		undoText = tr("Select nodes with invalid/missing sub network.");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, nodeIds, true);
	undo->push();
}


void SVPropNetworkSubStationWidget::on_tableWidgetSubNetworks_itemSelectionChanged() {
	bool enabled = m_ui->tableWidgetSubNetworks->currentRow() != -1;
	m_ui->pushButtonEditSubNetworks->setEnabled(enabled);
	m_ui->pushButtonSelectNodesWithSubNetwork->setEnabled(enabled);
	m_ui->pushButtonExchangeSubNetwork->setEnabled(enabled);
}


void SVPropNetworkSubStationWidget::on_pushButtonAssignSubNetwork_clicked() {
	unsigned int currentId = 0;
	if (m_pa->m_currentNodes.size() > 0)
		currentId = m_pa->m_currentNodes[0]->m_idSubNetwork;
	unsigned int newId = SVMainWindow::instance().dbSubNetworkEditDialog()->select(currentId);
	m_pa->modifyNodeProperty(&VICUS::NetworkNode::m_idSubNetwork, newId);
}

