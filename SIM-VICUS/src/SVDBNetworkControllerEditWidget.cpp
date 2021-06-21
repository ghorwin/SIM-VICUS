#include "SVDBNetworkControllerEditWidget.h"
#include "ui_SVDBNetworkControllerEditWidget.h"

SVDBNetworkControllerEditWidget::SVDBNetworkControllerEditWidget(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SVDBNetworkControllerEditWidget)
{
	ui->setupUi(this);
}

SVDBNetworkControllerEditWidget::~SVDBNetworkControllerEditWidget()
{
	delete ui;
}
