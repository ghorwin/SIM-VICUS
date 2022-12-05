#include "SVPropNetworkEdgesWidget.h"
#include "ui_SVPropNetworkEdgesWidget.h"

#include <VICUS_utilities.h>
#include <VICUS_Project.h>

#include <QtExt_Conversions.h>

#include "SVSettings.h"
#include "SVProjectHandler.h"
#include "SVPropNetworkEditWidget.h"
#include "SVUndoModifyNetwork.h"
#include "SVUndoTreeNodeState.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVStyle.h"


SVPropNetworkEdgesWidget::SVPropNetworkEdgesWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEdgesWidget),
	m_pa(qobject_cast<SVPropNetworkEditWidget*>(parent))
{
	m_ui->setupUi(this);

	// setup table widgets
	m_ui->tableWidgetPipes->setColumnCount(2);
	m_ui->tableWidgetPipes->setHorizontalHeaderLabels(QStringList() << QString() << tr("Pipes"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetPipes);
	m_ui->tableWidgetPipes->setSortingEnabled(false);
	m_ui->tableWidgetPipes->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetPipes->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetPipes->horizontalHeader()->setStretchLastSection(true);

	on_tableWidgetPipes_itemSelectionChanged();
}


SVPropNetworkEdgesWidget::~SVPropNetworkEdgesWidget()
{
	delete m_ui;
}


void SVPropNetworkEdgesWidget::updateUi() {
	Q_ASSERT(!m_pa->m_currentEdges.empty());

	// enable / disable wiudgets
	bool uniformEdge = m_pa->uniformProperty(m_pa->m_currentEdges, &VICUS::NetworkEdge::m_idPipe) && m_pa->m_currentNodes.empty();
	m_ui->groupBoxEdge->setEnabled(uniformEdge);
	m_ui->groupBoxSelectedPipe->setEnabled(uniformEdge);

	// update edge length and display name
	if (m_pa->m_currentEdges.size() == 1){
		m_ui->labelPipeLength->setText(QString("%1 m").arg(m_pa->m_currentEdges[0]->length()));
		m_ui->lineEditEdgeDisplayName->setText(m_pa->m_currentEdges[0]->m_displayName);
	}
	else{
		m_ui->labelPipeLength->clear();
		m_ui->lineEditEdgeDisplayName->clear();
	}

	// update supply property
	if (m_pa->uniformProperty(m_pa->m_currentEdges, &VICUS::NetworkEdge::m_supply))
		m_ui->checkBoxSupplyPipe->setChecked(m_pa->m_currentEdges[0]->m_supply);
	else
		m_ui->checkBoxSupplyPipe->setCheckState(Qt::CheckState::PartiallyChecked);

	// update pipe label
	if (m_pa->uniformProperty(m_pa->m_currentEdges, &VICUS::NetworkEdge::m_idPipe)){
		const SVDatabase  & db = SVSettings::instance().m_db;
		const VICUS::NetworkPipe * pipe = db.m_pipes[m_pa->m_currentEdges[0]->m_idPipe];
		if(pipe == nullptr)
			m_ui->labelSelectedPipe->clear();
		else
			m_ui->labelSelectedPipe->setText(QtExt::MultiLangString2QString(pipe->m_displayName));
	}
	else{
		m_ui->labelSelectedPipe->clear();
	}
}


void SVPropNetworkEdgesWidget::updateTableWidget() {

	m_ui->groupBoxCurrentPipes->setEnabled(true);
	const SVDatabase & db = SVSettings::instance().m_db;
	const VICUS::Project & p = project();

	//  *** Update pipes table widget ***

	std::vector<unsigned int> pipeIds;
	for (const VICUS::Network &net: p.m_geometricNetworks) {
		for (const VICUS::NetworkEdge &e: net.m_edges){
			if (std::find(pipeIds.begin(), pipeIds.end(), e.m_idPipe) == pipeIds.end() &&
				e.m_idPipe != VICUS::INVALID_ID)
				pipeIds.push_back(e.m_idPipe);
		}
	}
	// sort in ascending order of ids
	std::sort(pipeIds.begin(), pipeIds.end());

	int currentRow = m_ui->tableWidgetPipes->currentRow();
	m_ui->tableWidgetPipes->blockSignals(true);
	m_ui->tableWidgetPipes->clearContents();
	m_ui->tableWidgetPipes->setRowCount((int)pipeIds.size());
	int row = 0;
	for (unsigned int id: pipeIds){
		 const VICUS::NetworkPipe *pipe = db.m_pipes[id];
		 QTableWidgetItem * item = new QTableWidgetItem();
		 // special handling for components with "invalid" component id
		 if (pipe == nullptr)
			 item->setBackground(QColor(64,64,64));
		 else
			 item->setBackground(pipe->m_color);
		 item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		 item->setData(Qt::UserRole, id);
		 m_ui->tableWidgetPipes->setItem(row, 0, item);

		 item = new QTableWidgetItem();
		 if (pipe == nullptr)
			 item->setText(tr("<invalid pipe id>"));
		 else
			 item->setText(QtExt::MultiLangString2QString(pipe->m_displayName));
		 item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		 item->setData(Qt::UserRole, id);
		 m_ui->tableWidgetPipes->setItem(row, 1, item);

		 ++row;
	}
	// reselect row
	m_ui->tableWidgetPipes->blockSignals(false);
	m_ui->tableWidgetPipes->selectRow(std::min(currentRow, m_ui->tableWidgetPipes->rowCount()-1));
}


void SVPropNetworkEdgesWidget::clearUi() {
	m_ui->labelPipeLength->clear();
	m_ui->lineEditEdgeDisplayName->clear();
	m_ui->checkBoxSupplyPipe->setChecked(false);
	m_ui->tableWidgetPipes->clearContents();
}


void SVPropNetworkEdgesWidget::setWidgetsEnabled(bool enabled) {
	m_ui->groupBoxCurrentPipes->setEnabled(enabled);
	m_ui->groupBoxSelectedPipe->setEnabled(enabled);
	m_ui->groupBoxEdge->setEnabled(enabled);
}


void SVPropNetworkEdgesWidget::on_pushButtonAssignPipe_clicked()
{
	unsigned int currentId = 0;
	if (m_pa->m_currentEdges.size() > 0)
		currentId = m_pa->m_currentEdges[0]->m_idPipe;
	unsigned int newId = SVMainWindow::instance().dbPipeEditDialog()->select(currentId);
	m_pa->modifyEdgeProperty(&VICUS::NetworkEdge::m_idPipe, newId);
}

void SVPropNetworkEdgesWidget::on_pushButtonEditPipe_clicked()
{
	unsigned int currentId = 0;
	QTableWidgetItem *item = m_ui->tableWidgetPipes->currentItem();
	if (item != nullptr)
		currentId = item->data(Qt::UserRole).toUInt();
	SVMainWindow::instance().dbPipeEditDialog()->edit(currentId);
	updateTableWidget(); // for color update
}


void SVPropNetworkEdgesWidget::on_pushButtonRecalculateLength_clicked() {
	VICUS::Project p = project();
	VICUS::Network *network = VICUS::element(p.m_geometricNetworks, m_pa->m_currentNetwork->m_id);
	Q_ASSERT(network!=nullptr);

	network->updateNodeEdgeConnectionPointers();
	for (const VICUS::NetworkEdge * edge: m_pa->m_currentEdges){
		network->edge(edge->nodeId1(), edge->nodeId2())->setLengthFromCoordinates();
	}

	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), *network);
	undo->push(); // modifies project and updates views
	updateTableWidget();
}


void SVPropNetworkEdgesWidget::on_pushButtonSelectEdgesWithPipe_clicked() {

	// get pipeId from current item
	QTableWidgetItem *item = m_ui->tableWidgetPipes->currentItem();
	if (item == nullptr)
		return;
	unsigned int pipeId = item->data(Qt::UserRole).toUInt();

	// collect edges
	std::set<unsigned int> edgeIds;
	const VICUS::Project &p = project();
	for (const VICUS::Network &network: p.m_geometricNetworks) {
		for (const VICUS::NetworkEdge &e: network.m_edges) {
			if (e.m_idPipe == pipeId)
				edgeIds.insert(e.m_id);
		}
	}

	const VICUS::NetworkPipe * pipe = SVSettings::instance().m_db.m_pipes[pipeId];
	QString undoText;
	if (pipe != nullptr)
		undoText = tr("Select edges with pipe '%1'.").arg(QtExt::MultiLangString2QString(pipe->m_displayName));
	else
		undoText = tr("Select edges with invalid/missing pipe.");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, edgeIds, true);
	undo->push();
}


void SVPropNetworkEdgesWidget::on_pushButtonExchangePipe_clicked() {

	// get pipeId from current item
	QTableWidgetItem *item = m_ui->tableWidgetPipes->currentItem();
	if (item == nullptr)
		return;
	unsigned int oldId = item->data(Qt::UserRole).toUInt();

	// get new id
	unsigned int newId = SVMainWindow::instance().dbPipeEditDialog()->select(oldId);
	if (newId == oldId || newId==VICUS::INVALID_ID)
		return;

	VICUS::Project p = project();
	for (VICUS::Network &network: p.m_geometricNetworks) {
		// modify edges
		for (VICUS::NetworkEdge &e: network.m_edges) {
			if (e.m_idPipe == oldId)
				e.m_idPipe = newId;
		}
		SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), network);
		undo->push(); // modifies project and updates views
	}
}


void SVPropNetworkEdgesWidget::on_tableWidgetPipes_itemSelectionChanged() {
	bool enabled =  m_ui->tableWidgetPipes->currentRow() != -1;
	m_ui->pushButtonEditPipe->setEnabled(enabled);
	m_ui->pushButtonSelectEdgesWithPipe->setEnabled(enabled);
	m_ui->pushButtonExchangePipe->setEnabled(enabled);
}


void SVPropNetworkEdgesWidget::on_checkBoxSupplyPipe_clicked()
{
	m_pa->modifyEdgeProperty(&VICUS::NetworkEdge::m_supply, m_ui->checkBoxSupplyPipe->isChecked());
}


void SVPropNetworkEdgesWidget::on_lineEditEdgeDisplayName_editingFinished()
{
	m_pa->modifyEdgeProperty(&VICUS::NetworkEdge::m_displayName, m_ui->lineEditEdgeDisplayName->text());
}





