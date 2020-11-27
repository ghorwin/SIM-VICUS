#include "SVPropertyWidget.h"

#include <QVBoxLayout>

#include "SVPropAddPolygonWidget.h"

SVPropertyWidget::SVPropertyWidget(QWidget * parent) :
	QWidget(parent)
{
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setMargin(0);
	setLayout(m_layout);
	for (QWidget * &w : m_propWidgets)
		w = nullptr;
}

void SVPropertyWidget::setMode(PropertyWidgetMode m) {
	for (QWidget * w : m_propWidgets)
		if (w != nullptr)
			w->setVisible(false);

	switch (m) {
		case M_ThermalSimulationProperties : {
			// TODO : show edit widgets for thermal simulation properties
		} break;

		case M_AddVertexesMode: {
			// create widget and add to layout, if not existing
			if (m_propWidgets[M_AddVertexesMode] == nullptr) {
				m_propWidgets[M_AddVertexesMode] = new SVPropAddPolygonWidget(this);
				m_layout->addWidget(m_propWidgets[M_AddVertexesMode]);
			}
			m_propWidgets[M_AddVertexesMode]->setVisible(true);
		} break;
		default : {
			// set maximum size to 0, to avoid widget being expandible
		}
	}
}
