#include "SVPropertyWidget.h"

#include <QVBoxLayout>
#include <QToolBox>

#include <IBKMK_Vector3D.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"

#include "SVPropVertexListWidget.h"
#include "SVPropEditGeometry.h"
#include "SVPropSiteWidget.h"
#include "SVPropNetworkEditWidget.h"
#include "SVPropModeSelectionWidget.h"
#include "SVPropBuildingEditWidget.h"


#include "Vic3DNewGeometryObject.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DWireFrameObject.h"

SVPropertyWidget::SVPropertyWidget(QWidget * parent) :
	QWidget(parent)
{
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	setLayout(m_layout);
	for (QWidget * &w : m_propWidgets)
		w = nullptr;

	m_propModeSelectionWidget = new SVPropModeSelectionWidget(this);
	m_layout->addWidget(m_propModeSelectionWidget);


	setMinimumWidth(200);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVPropertyWidget::onViewStateChanged);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropertyWidget::onModified);

	connect(m_propModeSelectionWidget, &SVPropModeSelectionWidget::buildingPropertiesSelected,
			this, &SVPropertyWidget::onBuildingPropertiesSelected);
	connect(m_propModeSelectionWidget, &SVPropModeSelectionWidget::networkPropertiesSelected,
			this, &SVPropertyWidget::onNetworkPropertiesSelected);
	connect(m_propModeSelectionWidget, &SVPropModeSelectionWidget::sitePropertiesSelected,
			this, &SVPropertyWidget::onSitePropertiesSelected);

	onViewStateChanged();
}



void SVPropertyWidget::onViewStateChanged() {

	// hide all already created widgets
	for (QWidget * w : m_propWidgets) {
		if (w != nullptr)
			w->setVisible(false);
	}

	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	SVViewState::PropertyWidgetMode m = vs.m_propertyWidgetMode;

	switch (vs.m_viewMode) {
		case SVViewState::VM_GeometryEditMode:
			m_propModeSelectionWidget->setVisible(false);
		break;
		case SVViewState::VM_PropertyEditMode:
			m_propModeSelectionWidget->setVisible(true);
		break;
		case SVViewState::NUM_VM: break; // just to make compiler happy
	}


	// now show the respective property widget
	switch (m) {
		case SVViewState::PM_EditGeometry :
		case SVViewState::PM_AddGeometry : {
			showPropertyWidget<SVPropEditGeometry>(M_Geometry);

			// Note: we do not use the slot for SVViewState::PM_AddGeometry; instead we just show a different tab
			if (m == SVViewState::PM_EditGeometry)
				qobject_cast<SVPropEditGeometry *>(m_propWidgets[M_Geometry])->setCurrentTab(SVPropEditGeometry::TS_EditGeometry);
			else
				qobject_cast<SVPropEditGeometry *>(m_propWidgets[M_Geometry])->setCurrentTab(SVPropEditGeometry::TS_AddGeometry);
		} break;

		case SVViewState::PM_VertexList:
			showPropertyWidget<SVPropVertexListWidget>(M_VertexListWidget);
			setMinimumWidth(500);
		break;

		case SVViewState::PM_SiteProperties :
			showPropertyWidget<SVPropSiteWidget>(M_SiteProperties);
		break;

		case SVViewState::PM_BuildingProperties :
			showPropertyWidget<SVPropBuildingEditWidget>(M_BuildingProperties);
		break;

		case SVViewState::PM_NetworkProperties :
			showPropertyWidget<SVPropNetworkEditWidget>(M_NetworkProperties);
		break;
	}
}


void SVPropertyWidget::onModified(int modificationType, ModificationInfo * ) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {
		case SVProjectHandler::NetworkModified:
		case SVProjectHandler::AllModified:
		case SVProjectHandler::NodeStateModified:
			// Tell mode selection widget that the selection property has changed.
			// This is used to switch into a specific building/network edit mode
			// based on the current selection. For example, when "Network" mode is selected
			// and a node is selected, we automatically switch to "Node" edit mode.
			m_propModeSelectionWidget->selectionChanged();
		break;

		default: ; // just to make compiler happy
	}
}


void SVPropertyWidget::onSitePropertiesSelected() {
	// change view state to "edit site properties"
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_propertyWidgetMode = SVViewState::PM_SiteProperties;
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropertyWidget::onBuildingPropertiesSelected(int propertyIndex) {
	// change view state to "edit building properties"
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_propertyWidgetMode = SVViewState::PM_BuildingProperties;
	SVViewStateHandler::instance().setViewState(vs);
	// now the building property widget exists and is shown
	// select highlighting/edit mode -> this will send a signal to update the scene's geometry coloring
	qobject_cast<SVPropBuildingEditWidget*>(m_propWidgets[M_BuildingProperties])->setPropertyType(propertyIndex);
}


void SVPropertyWidget::onNetworkPropertiesSelected(int propertyIndex) {
	// change view state to "edit network properties"
	SVViewState vs = SVViewStateHandler::instance().viewState();

	vs.m_propertyWidgetMode = SVViewState::PM_NetworkProperties;
	SVViewStateHandler::instance().setViewState(vs);
	// now the network property widget exists and is shown
	// select highlighting/edit mode -> this will send a signal to update the scene's geometry coloring
	qobject_cast<SVPropNetworkEditWidget*>(m_propWidgets[M_NetworkProperties])->setPropertyMode(propertyIndex);
}



