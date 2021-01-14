#include "SVDBPipeEditWidget.h"
#include "ui_SVDBPipeEditWidget.h"

SVDBPipeEditWidget::SVDBPipeEditWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SVDBPipeEditWidget)
{
	ui->setupUi(this);
}

SVDBPipeEditWidget::~SVDBPipeEditWidget()
{
	delete ui;
}
