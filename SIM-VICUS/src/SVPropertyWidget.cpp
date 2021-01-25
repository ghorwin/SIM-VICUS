#include "SVPropertyWidget.h"

#include <QVBoxLayout>
#include <QToolBox>

#include <IBKMK_Vector3D.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVConstants.h"

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

	onViewStateChanged();
}



void SVPropertyWidget::onViewStateChanged() {

	// hide all already created widgets
	for (QWidget * w : m_propWidgets) {
		if (w != nullptr)
			w->setVisible(false);
	}

	SVViewState vs = SVViewStateHandler::instance().viewState();

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
	SVViewState::PropertyWidgetMode m = vs.m_propertyWidgetMode;
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

		case SVViewState::PM_BuildingProperties : {
			showPropertyWidget<SVPropBuildingEditWidget>(M_BuildingProperties);
			// select highlighting/edit mode -> this will send a signal to update the scene's geometry coloring
			BuildingPropertyTypes buildingPropertyType = m_propModeSelectionWidget->currentBuildingPropertyType();
			qobject_cast<SVPropBuildingEditWidget*>(m_propWidgets[M_BuildingProperties])->setPropertyType(buildingPropertyType);
		} break;

		case SVViewState::PM_NetworkProperties : {
			showPropertyWidget<SVPropNetworkEditWidget>(M_NetworkProperties);
			// select highlighting/edit mode -> this will send a signal to update the scene's geometry coloring
			int networkPropertyType = m_propModeSelectionWidget->currentNetworkPropertyType();
			qobject_cast<SVPropNetworkEditWidget*>(m_propWidgets[M_NetworkProperties])->setPropertyMode(networkPropertyType);
		} break;
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
			// Note: the call above may results in a view state change, when the selection causes
			//		 the network property selection to change.

			// Note: the individual property widgets should listen themselves to onModified() signals
			//		 and react on selection/visibility changed signals accordingly. Alternatively,
			//		 the modification info can be triggered from here as well, but not both at the same time.
		case SVProjectHandler::BuildingGeometryChanged:
			// We have to call to update the properties in edit geometry mode
			// therefore we need to store a pointer to it
			qobject_cast<SVPropEditGeometry *>(m_propWidgets[M_Geometry])->update();
		break;

		default: ; // just to make compiler happy
	}
}

