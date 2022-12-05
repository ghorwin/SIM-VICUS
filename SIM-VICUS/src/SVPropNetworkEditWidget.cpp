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
#include "SVUndoModifyNetwork.h"
#include "SVViewStateHandler.h"
#include "SVConstants.h"


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
	m_ui->toolBox->addPage(tr("Edges"), m_edgesWidget);
	m_ui->toolBox->addPage(tr("Sub Stations"), m_subStationWidget);
	m_ui->toolBox->addPage(tr("Heat Exchange"), m_hxWidget);
	m_ui->toolBox->blockSignals(false);
	m_ui->toolBox->setCurrentIndex(0);

	connect(m_ui->toolBox, &QtExt::ToolBox::indexChanged,
			this, &SVPropNetworkEditWidget::onPropertyTypeChanged);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropNetworkEditWidget::onModified);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified);
}


SVPropNetworkEditWidget::~SVPropNetworkEditWidget() {
	delete m_ui;
}


void SVPropNetworkEditWidget::onModified(int modificationType) {

	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {

		case SVProjectHandler::NodeStateModified:
		case SVProjectHandler::AllModified:
		case SVProjectHandler::NetworkGeometryChanged: {

			findSelectedObjects();

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

			// node(s) selected
			if (!m_currentNodes.empty()){
				m_nodesWidget->updateUi();
				m_subStationWidget->updateUi();
			}
			// edge(s) selected
			if(!m_currentEdges.empty()){
				m_edgesWidget->updateUi();
			}

			// update network geometry widget
			m_geometryWidget->updateComboBoxNetworks();
			if (m_currentNetwork != nullptr)
				m_geometryWidget->setCurrentNetwork(m_currentNetwork->m_id);
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


void SVPropNetworkEditWidget::findSelectedObjects() {

	m_currentEdges.clear();
	m_currentNodes.clear();
	m_currentNetwork = nullptr;

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

	// We check if the selected nodes, edges belong to different networks,
	// if so: we clear everything. We don't allow editing of different networks at the same time
	std::vector<unsigned int> networkIds;
	for (const VICUS::NetworkEdge *e: m_currentEdges)
		networkIds.push_back(e->m_parent->m_id);
	for (const VICUS::NetworkNode *n: m_currentNodes)
		networkIds.push_back(n->m_parent->m_id);

	if (networkIds.size()>0)
		m_currentNetwork = dynamic_cast<const VICUS::Network*>(project().objectById(networkIds[0]));

	for (unsigned int id: networkIds){
		if (id != networkIds[0]) {
			m_currentEdges.clear();
			m_currentNodes.clear();
			m_currentNetwork = nullptr;
		}
	}
}

