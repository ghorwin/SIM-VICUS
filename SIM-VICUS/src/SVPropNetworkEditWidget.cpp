#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include "SVPropNetworkGeometryWidget.h"
#include "SVPropNetworkEdgesWidget.h"
#include "SVPropNetworkNodesWidget.h"
#include "SVPropNetworkHeatExchangeWidget.h"
#include "SVPropNetworkSubStationWidget.h"
#include "SVProjectHandler.h"
#include "SVUndoModifyActiveNetworkId.h"
#include "SVViewStateHandler.h"
#include "SVConstants.h"
#include "SVMainWindow.h"
#include "SVPreferencesDialog.h"
#include "SVPreferencesPageStyle.h"
#include "SVUndoDeleteSelected.h"


SVPropNetworkEditWidget::SVPropNetworkEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEditWidget)
{
	m_ui->setupUi(this);
	m_geometryWidget = new SVPropNetworkGeometryWidget(this);
	m_nodesWidget = new SVPropNetworkNodesWidget(this);
	m_edgesWidget = new SVPropNetworkEdgesWidget(this);
	m_subStationWidget = new SVPropNetworkSubStationWidget(this);
	m_hxWidget = new SVPropNetworkHeatExchangeWidget(this);

	m_ui->toolBox->blockSignals(true);
	m_ui->toolBox->addPage(tr("Geometry"), m_geometryWidget);
	m_ui->toolBox->addPage(tr("Nodes"), m_nodesWidget);
	m_ui->toolBox->addPage(tr("Pipes"), m_edgesWidget);
	m_ui->toolBox->addPage(tr("Sub Stations"), m_subStationWidget);
	m_ui->toolBox->addPage(tr("Heat Exchange"), m_hxWidget);
	m_ui->toolBox->blockSignals(false);
	m_ui->toolBox->setCurrentIndex(0);

	connect(m_ui->toolBox, &QtExt::ToolBox::indexChanged,
			this, &SVPropNetworkEditWidget::onPropertyTypeChanged);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropNetworkEditWidget::onModified);

	connect(SVMainWindow::instance().preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged,
			this, &SVPropNetworkEditWidget::onStyleChanged);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified);
}


SVPropNetworkEditWidget::~SVPropNetworkEditWidget() {
	delete m_ui;
}


void SVPropNetworkEditWidget::onModified(int modificationType) {

	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {

		case SVProjectHandler::AllModified:
		case SVProjectHandler::NodeStateModified:
		case SVProjectHandler::NetworkGeometryChanged: {

			findSelectedObjects();
			m_ui->toolButtonAssignToCurrent->setVisible( !m_activeNetworkSelected
														&& (!m_currentEdges.empty() || !m_currentNodes.empty()) );

			// we need the option to show only active network only in case we have multiple networks
			m_ui->checkBoxShowAllNetworks->setVisible(project().m_geometricNetworks.size() > 1);

			// now update UI
			m_nodesWidget->setWidgetsEnabled(false);
			m_edgesWidget->setWidgetsEnabled(false);
			m_subStationWidget->setWidgetsEnabled(false);
			m_hxWidget->setWidgetsEnabled(false);

			m_nodesWidget->clearUi();
			m_edgesWidget->clearUi();
			m_subStationWidget->clearUi();
			m_hxWidget->clearUi();

			m_edgesWidget->updateTableWidget();
			m_subStationWidget->updateTableWidget();
			m_hxWidget->updateTableWidget();

			m_hxWidget->updateUi();
			m_nodesWidget->updateUi();
			m_subStationWidget->updateUi();
			m_edgesWidget->updateUi();

			updateComboBoxNetworks();
			// update network geometry widget
			m_geometryWidget->updateCurrentNetwork();
			m_geometryWidget->updateUi();

		} break;
		default: ; // just to make compiler happy
	}
}


void SVPropNetworkEditWidget::onPropertyTypeChanged(int propertyType) {
	// set coloring mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	switch (NetworkPropertyTypes(propertyType)) {
		case NT_Geometry		: vs.m_objectColorMode = SVViewState::OCM_Network ; break;
		case NT_Node			: vs.m_objectColorMode = SVViewState::OCM_NetworkNode ; break;
		case NT_Edge			: vs.m_objectColorMode = SVViewState::OCM_NetworkEdge ; break;
		case NT_SubStation		: vs.m_objectColorMode = SVViewState::OCM_NetworkSubNetworks ; break;
		case NT_HeatExchange	: vs.m_objectColorMode = SVViewState::OCM_NetworkHeatExchange ; break;
	}
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropNetworkEditWidget::onStyleChanged() {
	m_ui->toolBox->updatePageBackgroundColorFromStyle();
}

unsigned int SVPropNetworkEditWidget::currentPropertyType() {
	return m_ui->toolBox->currentIndex();
}


void SVPropNetworkEditWidget::setPropertyType(int propertyType) {
	m_ui->toolBox->setCurrentIndex((unsigned int)propertyType);
}


void SVPropNetworkEditWidget::findSelectedObjects() {

	m_currentEdges.clear();
	m_currentNodes.clear();

	// get all selected objects of type network, objects must be visible
	std::set<const VICUS::Object *> objs;
	project().selectObjects(objs, VICUS::Project::SG_Network, true, true);

	// We already have the correct networkId and m_currentConstNetwork is already set correctly
	// So we only cast the nodes / edges here
	for (const VICUS::Object* o : objs) {

		// if parent does not exist, this is an entire network or something else
		if (o->m_parent == nullptr){
			continue;
		}

		// else we have nodes or edges which we want to cast
		else {
			const VICUS::NetworkEdge * edge = dynamic_cast<const VICUS::NetworkEdge*>(o);
			if (edge != nullptr) {
				m_currentEdges.push_back(edge);
				continue;
			}
			const VICUS::NetworkNode * node = dynamic_cast<const VICUS::NetworkNode*>(o);
			if (node != nullptr) {
				m_currentNodes.push_back(node);
				continue;
			}
		}
	}

	// collect all networks involved in the selection
	std::set<unsigned int> networkIds;
	for (const VICUS::NetworkEdge *e: m_currentEdges)
		networkIds.insert(e->m_parent->m_id);
	for (const VICUS::NetworkNode *n: m_currentNodes)
		networkIds.insert(n->m_parent->m_id);

	// is the currently selected network identical to the active network?
	if (networkIds.size() == 1 && *networkIds.begin() == project().m_activeNetworkId )
		m_activeNetworkSelected = true;
	else
		m_activeNetworkSelected = false;

	// if there are nodes / edge selected that belong to different networks, we don't allow editing these
	for (unsigned int id: networkIds){
		if (id != *networkIds.begin()) {
			m_currentEdges.clear();
			m_currentNodes.clear();
		}
	}
}


void SVPropNetworkEditWidget::updateComboBoxNetworks() {
	// fill combobox
	m_ui->comboBoxCurrentNetwork->blockSignals(true);
	m_ui->comboBoxCurrentNetwork->clear();
	const VICUS::Project &p = project();
	for (const VICUS::Network &n : p.m_geometricNetworks)
		m_ui->comboBoxCurrentNetwork->addItem(n.m_displayName, n.m_id);
	// reselect index
	int idx = m_ui->comboBoxCurrentNetwork->findData(p.m_activeNetworkId);
	if (idx != -1)
		m_ui->comboBoxCurrentNetwork->setCurrentIndex(idx);
	m_ui->comboBoxCurrentNetwork->blockSignals(false);
}


void SVPropNetworkEditWidget::on_comboBoxCurrentNetwork_activated(int /*index*/) {
	unsigned int id = m_ui->comboBoxCurrentNetwork->currentData().toUInt();
	if (id == project().m_activeNetworkId)
		return;
	SVUndoModifyActiveNetworkId *undo = new SVUndoModifyActiveNetworkId(id);
	undo->push();
}


void SVPropNetworkEditWidget::on_toolButtonAssignToCurrent_clicked()
{
	// add edges and nodes to active network
	const VICUS::Project &p = project();
	Q_ASSERT(VICUS::element(p.m_geometricNetworks, p.m_activeNetworkId) != nullptr);
	VICUS::Network activeNet = *VICUS::element(p.m_geometricNetworks, p.m_activeNetworkId);
	unsigned int nextId = p.nextUnusedID();
	std::set<unsigned int> idsNotDelete;
	for (const VICUS::NetworkNode *n: m_currentNodes) {
		if (n->m_type == VICUS::NetworkNode::NT_Mixer)
			idsNotDelete.insert(n->m_id); // dont delete mixers, this would delete the connected edges as well
		unsigned int id = activeNet.addNode(++nextId, n->m_position, n->m_type, true);
		activeNet.nodeById(id)->m_maxHeatingDemand = n->m_maxHeatingDemand;
		activeNet.nodeById(id)->m_idSubNetwork = n->m_idSubNetwork;
		activeNet.nodeById(id)->m_heatExchange = n->m_heatExchange;
	}
	for (const VICUS::NetworkEdge *e: m_currentEdges) {
		unsigned int id1 = activeNet.addNode(++nextId, e->m_node1->m_position, e->m_node1->m_type, true);
		unsigned int id2 = activeNet.addNode(++nextId, e->m_node2->m_position, e->m_node2->m_type, true);
		activeNet.addEdge(++nextId, id1, id2, e->m_supply, e->m_idPipe);
		activeNet.edge(id1, id2)->m_heatExchange = e->m_heatExchange;
	}

	// delete original objects
	std::set<const VICUS::Object *> selectedObjects;
	project().selectObjects(selectedObjects, VICUS::Project::SG_Network, true, true);
	std::set<const VICUS::Object *> objectsToDelete;
	for (const VICUS::Object *o: selectedObjects) {
		if (idsNotDelete.find(o->m_id) == idsNotDelete.end())
			objectsToDelete.insert(o);
	}
	SVUndoDeleteSelected *undoDel = new SVUndoDeleteSelected("", objectsToDelete);
	undoDel->push();

	// undo active network
	SVUndoModifyNetwork *undo = new SVUndoModifyNetwork("added edges", activeNet);
	undo->push();
}


void SVPropNetworkEditWidget::on_checkBoxShowAllNetworks_clicked()
{
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_showActiveNetworkOnly = !m_ui->checkBoxShowAllNetworks->isChecked();
	SVViewStateHandler::instance().setViewState(vs);
}

