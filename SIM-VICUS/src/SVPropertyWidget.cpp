#include "SVPropertyWidget.h"

#include <QVBoxLayout>
#include <QToolBox>

#include <IBKMK_Vector3D.h>

#include "SVPropVertexListWidget.h"
#include "SVPropEditGeometry.h"
#include "SVViewStateHandler.h"

#include "Vic3DNewPolygonObject.h"
#include "Vic3DCoordinateSystemObject.h"

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
	setWidgetVisible(M_AddGeometry);


	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVPropertyWidget::onViewStateChanged);
}


void SVPropertyWidget::setWidgetVisible(PropertyWidgets m) {
	for (QWidget * w : m_propWidgets)
		if (w != nullptr)
			w->setVisible(false);

	switch (m) {
		case M_ThermalSimulationProperties : {
			// TODO : show edit widgets for thermal simulation properties
		} break;


		case M_EditGeometry : {
			// create widget and add to layout, if not existing
			if (m_propWidgets[M_EditGeometry] == nullptr) {
				SVPropEditGeometry *propEditGeometry = new SVPropEditGeometry(this);
				m_propWidgets[M_EditGeometry] = propEditGeometry;
				m_layout->addWidget(m_propWidgets[M_EditGeometry]);
				SVViewStateHandler::instance().m_coordinateSystemObject->m_propEditGeometry = propEditGeometry;
//				SVViewStateHandler::instance().m_newPolygonObject->m_vertexListWidget = vertexListWidget;
			}
			m_propWidgets[M_EditGeometry]->setVisible(true);

			((SVPropEditGeometry *) m_propWidgets[M_EditGeometry])->setCurrentTab(SVPropEditGeometry::TS_EditGeometry);

		} break;

		case M_AddGeometry : {
			// create widget and add to layout, if not existing
			if (m_propWidgets[M_EditGeometry] == nullptr) {
				m_propWidgets[M_EditGeometry] = new SVPropEditGeometry(this);
				m_layout->addWidget(m_propWidgets[M_EditGeometry]);
			}
			m_propWidgets[M_EditGeometry]->setVisible(true);

			((SVPropEditGeometry *) m_propWidgets[M_EditGeometry])->setCurrentTab(SVPropEditGeometry::TS_AddGeometry);

		} break;

		case M_AddVertexesMode: {
			// create widget and add to layout, if not existing
			if (m_propWidgets[M_AddVertexesMode] == nullptr) {
				SVPropVertexListWidget * vertexListWidget = new SVPropVertexListWidget(this);
				m_propWidgets[M_AddVertexesMode] = vertexListWidget;
				m_layout->addWidget(m_propWidgets[M_AddVertexesMode]);
				SVViewStateHandler::instance().m_newPolygonObject->m_vertexListWidget = vertexListWidget;
			}
			if (!m_propWidgets[M_AddVertexesMode]->isVisibleTo(this)) {
				// when shown, we always reset the widget to "new surface" mode
				((SVPropVertexListWidget *)m_propWidgets[M_AddVertexesMode])->onNewVertexListStart();
				m_propWidgets[M_AddVertexesMode]->setVisible(true);
			}
		} break;

		default : {
			// set maximum size to 0, to avoid widget being expandible
		}
	}
}


void SVPropertyWidget::onViewStateChanged() {
	switch (SVViewStateHandler::instance().viewState().m_propertyWidgetMode) {
		case SVViewState::PM_VertexList		: setWidgetVisible(M_AddVertexesMode); break;
		case SVViewState::PM_AddGeometry	:
			setWidgetVisible(M_AddGeometry);
		break;
		case SVViewState::PM_EditGeometry	:
			setWidgetVisible(M_EditGeometry);
		break;
		default:;
	}
}



