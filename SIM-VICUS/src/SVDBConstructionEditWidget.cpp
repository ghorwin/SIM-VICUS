#include "SVDBConstructionEditWidget.h"
#include "ui_SVDBConstructionOpaqueEditWidget.h"

SVDBConstructionOpaqueEditWidget::SVDBConstructionOpaqueEditWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SVDBConstructionOpaqueEditWidget)
{
	ui->setupUi(this);
}

SVDBConstructionOpaqueEditWidget::~SVDBConstructionOpaqueEditWidget()
{
	delete ui;
}
