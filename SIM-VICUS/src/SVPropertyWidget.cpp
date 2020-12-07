#include "SVPropertyWidget.h"

#include <QVBoxLayout>

#include <IBKMK_Vector3D.h>

#include "SVPropVertexListWidget.h"
#include "SVPropEditGeometry.h"
#include "SVViewStateHandler.h"

#include "Vic3DNewPolygonObject.h"

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
	setMode(M_EditGeometry);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVPropertyWidget::onViewStateChanged);
}


void SVPropertyWidget::setMode(PropertyWidgets m) {
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
				m_propWidgets[M_EditGeometry] = new SVPropEditGeometry(this);
				m_layout->addWidget(m_propWidgets[M_EditGeometry]);
			}
			m_propWidgets[M_EditGeometry]->setVisible(true);
		} break;

		case M_AddVertexesMode: {
			// create widget and add to layout, if not existing
			if (m_propWidgets[M_AddVertexesMode] == nullptr) {
				SVPropVertexListWidget * vertexListWidget = new SVPropVertexListWidget(this);
				m_propWidgets[M_AddVertexesMode] = vertexListWidget;
				m_layout->addWidget(m_propWidgets[M_AddVertexesMode]);
				SVViewStateHandler::instance().m_newPolygonObject->m_vertexListWidget = vertexListWidget;
			}
			m_propWidgets[M_AddVertexesMode]->setVisible(true);
		} break;

		default : {
			// set maximum size to 0, to avoid widget being expandible
		}
	}
}


void SVPropertyWidget::onViewStateChanged() {
	switch (SVViewStateHandler::instance().viewState().m_propertyWidgetMode) {
		case SVViewState::PM_VERTEX_LIST : setMode(M_AddVertexesMode); break;
		case SVViewState::PM_ADD_GEOMETRY : setMode(M_EditGeometry); break;
		case SVViewState::PM_EDIT_GEOMETRY : setMode(M_EditGeometry); break;
		default:;
	}
}



