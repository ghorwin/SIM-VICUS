#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

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


void SVPropEditGeometry::on_pushButtonAddPolygon_clicked() {
	// signal, that we want to start adding a new polygon
}

void SVPropEditGeometry::on_toolButtonAddZoneBox_clicked() {
	// configure the view to go into "place first vertex" mode,
	// toggle property widget to show "placed vertexes" widget


}
