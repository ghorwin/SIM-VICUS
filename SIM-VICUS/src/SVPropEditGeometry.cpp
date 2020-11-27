#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

SVPropEditGeometry::SVPropEditGeometry(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditGeometry)
{
	m_ui->setupUi(this);
}


SVPropEditGeometry::~SVPropEditGeometry() {
	delete m_ui;
}


void SVPropEditGeometry::on_pushButtonAddPolygon_clicked() {
	// signal, that we want to start adding a new polygon
	emit addPolyonStarted();
}
