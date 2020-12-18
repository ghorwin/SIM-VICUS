#include "SVDBComponentEditWidget.h"
#include "ui_SVDBComponentEditWidget.h"

SVDBComponent::SVDBComponent(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SVDBComponent)
{
	ui->setupUi(this);
}

SVDBComponent::~SVDBComponent()
{
	delete ui;
}
