#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

#include "SVViewStateHandler.h"
#include "Vic3DNewPolygonObject.h"

SVPropEditGeometry::SVPropEditGeometry(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditGeometry)
{
	m_ui->setupUi(this);
	layout()->setMargin(0);
}


SVPropEditGeometry::~SVPropEditGeometry() {
	delete m_ui;
}

void SVPropEditGeometry::setCurrentTab(const SVPropEditGeometry::TabState &state)
{
	m_ui->toolBoxGeometry->setCurrentIndex(state);
}

void SVPropEditGeometry::on_pushButtonAddPolygon_clicked() {
	// reset new polygon object
	SVViewStateHandler::instance().m_newPolygonObject->clear();
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropEditGeometry::on_toolButtonAddZoneBox_clicked() {
	// configure the view to go into "place first vertex" mode,
	// toggle property widget to show "placed vertexes" widget


}
