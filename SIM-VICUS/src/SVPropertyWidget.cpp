#include "SVPropertyWidget.h"

#include <QVBoxLayout>
#include <QToolBox>

#include <IBKMK_Vector3D.h>

#include "SVViewStateHandler.h"

#include "SVPropVertexListWidget.h"
#include "SVPropEditGeometry.h"
#include "SVPropSiteWidget.h"
#include "SVPropNetworkEditWidget.h"
#include "Vic3DNewGeometryObject.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DWireFrameObject.h"

SVPropertyWidget::SVPropertyWidget(QWidget * parent) :
	QWidget(parent)
{
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setMargin(0);
	setLayout(m_layout);
	for (QWidget * &w : m_propWidgets)
		w = nullptr;

	setMinimumWidth(200);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVPropertyWidget::onViewStateChanged);

	onViewStateChanged();
}


void SVPropertyWidget::onViewStateChanged() {

	// hide all already created widgets
	for (unsigned int i=0; i<SVViewState::NUM_PM; ++i) {
		QWidget * w = m_propWidgets[i];
		if (w != nullptr)
			w->setVisible(false);
	}

	// catch changes in object selection
	std::set<const VICUS::Object*> selObjects = SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects;
	for (const VICUS::Object * o : selObjects) {
		if (dynamic_cast<const VICUS::NetworkNode *>(o) != nullptr) {
			showNetworkPropertyWidget();
			qobject_cast<SVPropNetworkEditWidget *>(m_propWidgets[SVViewState::PM_NetworkProperties])->updateUi(S_MultipleObjects);
			qobject_cast<SVPropNetworkEditWidget *>(m_propWidgets[SVViewState::PM_NetworkProperties])->showNodeProperties();
			return;
		}
		if (dynamic_cast<const VICUS::NetworkEdge *>(o) != nullptr) {
			showNetworkPropertyWidget();
			qobject_cast<SVPropNetworkEditWidget *>(m_propWidgets[SVViewState::PM_NetworkProperties])->updateUi(S_MultipleObjects);
			qobject_cast<SVPropNetworkEditWidget *>(m_propWidgets[SVViewState::PM_NetworkProperties])->showEdgeProperties();
			return;
		}
	}


	// TODO Andreas: still needed ?

//	// cache visibility state
//	bool visible[SVViewState::NUM_PM];
//	for (unsigned int i=0; i<SVViewState::NUM_PM; ++i) {
//		QWidget * w = m_propWidgets[i];
//		if (w != nullptr) {
//			visible[i] = w->isVisibleTo(this);
//			w->setVisible(false);
//		}
//		else
//			visible[i] = false;
//	}

	SVViewState::PropertyWidgetMode m = SVViewStateHandler::instance().viewState().m_propertyWidgetMode;

	switch (m) {
		case SVViewState::PM_EditGeometry :
		case SVViewState::PM_AddGeometry : {
			// create widget and add to layout, if not existing
			if (m_propWidgets[SVViewState::PM_EditGeometry] == nullptr) {
				SVPropEditGeometry *propEditGeometry = new SVPropEditGeometry(this);
				m_propWidgets[SVViewState::PM_EditGeometry] = propEditGeometry;
				m_layout->addWidget(m_propWidgets[SVViewState::PM_EditGeometry]);
			}
			m_propWidgets[SVViewState::PM_EditGeometry]->setVisible(true);

			// Note: we do not use the slot for SVViewState::PM_AddGeometry; instead we just show a different tab
			if (m == SVViewState::PM_EditGeometry)
				((SVPropEditGeometry *) m_propWidgets[SVViewState::PM_EditGeometry])->setCurrentTab(SVPropEditGeometry::TS_EditGeometry);
			else
				((SVPropEditGeometry *) m_propWidgets[SVViewState::PM_EditGeometry])->setCurrentTab(SVPropEditGeometry::TS_AddGeometry);

		} break;


		case SVViewState::PM_VertexList: {
			// create widget and add to layout, if not existing
			if (m_propWidgets[SVViewState::PM_VertexList] == nullptr) {
				SVPropVertexListWidget * vertexListWidget = new SVPropVertexListWidget(this);
				m_propWidgets[SVViewState::PM_VertexList] = vertexListWidget;
				m_layout->addWidget(m_propWidgets[SVViewState::PM_VertexList]);
			}
			m_propWidgets[SVViewState::PM_VertexList]->setVisible(true);
			setMinimumWidth(500);
		} break;


		case SVViewState::PM_SiteProperties : {
			// create widget and add to layout, if not existing
			if (m_propWidgets[SVViewState::PM_SiteProperties] == nullptr) {
				SVPropSiteWidget *propSiteWidget = new SVPropSiteWidget(this);
				m_propWidgets[SVViewState::PM_SiteProperties] = propSiteWidget;
				m_layout->addWidget(m_propWidgets[SVViewState::PM_SiteProperties]);
			}
			m_propWidgets[SVViewState::PM_SiteProperties]->setVisible(true);
		} break;


		case SVViewState::PM_NetworkProperties :
		case SVViewState::PM_NetworkNodeProperties :
		case SVViewState::PM_NetworkEdgeProperties : {
			showNetworkPropertyWidget();
			// tell widget to update its content
			qobject_cast<SVPropNetworkEditWidget *>(m_propWidgets[SVViewState::PM_NetworkProperties])->updateUi(S_SingleObject);
			if (m == SVViewState::PM_NetworkProperties)
				qobject_cast<SVPropNetworkEditWidget *>(m_propWidgets[SVViewState::PM_NetworkProperties])->showNetworkProperties();
			else if (m == SVViewState::PM_NetworkNodeProperties)
				qobject_cast<SVPropNetworkEditWidget *>(m_propWidgets[SVViewState::PM_NetworkProperties])->showNodeProperties();
			else
				qobject_cast<SVPropNetworkEditWidget *>(m_propWidgets[SVViewState::PM_NetworkProperties])->showEdgeProperties();
		} break;

		default : {
			// set maximum size to 0, to avoid widget being expandible
		}
	}
}

void SVPropertyWidget::showNetworkPropertyWidget()
{
	// create widget and add to layout, if not existing
	if (m_propWidgets[SVViewState::PM_NetworkProperties] == nullptr) {
		SVPropNetworkEditWidget *propWidget = new SVPropNetworkEditWidget(this);
		m_propWidgets[SVViewState::PM_NetworkProperties] = propWidget;
		m_layout->addWidget(m_propWidgets[SVViewState::PM_NetworkProperties]);
	}
	m_propWidgets[SVViewState::PM_NetworkProperties]->setVisible(true);
}





