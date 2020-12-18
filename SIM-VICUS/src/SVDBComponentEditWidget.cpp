#include "SVDBComponent.h"
#include "ui_SVDBComponent.h"

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
