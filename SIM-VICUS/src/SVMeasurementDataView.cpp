#include "SVMeasurementDataView.h"
#include "ui_SVMeasurementDataView.h"

SVMeasurementDataView::SVMeasurementDataView(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SVMeasurementDataView)
{
	ui->setupUi(this);
}

SVMeasurementDataView::~SVMeasurementDataView()
{
	delete ui;
}
