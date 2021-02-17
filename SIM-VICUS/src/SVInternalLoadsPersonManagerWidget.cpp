#include "SVInternalLoadsPersonManagerWidget.h"
#include "ui_SVInternalLoadsPersonManagerWidget.h"

SVInternalLoadsPersonManagerWidget::SVInternalLoadsPersonManagerWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SVInternalLoadsPersonManagerWidget)
{
	ui->setupUi(this);
}

SVInternalLoadsPersonManagerWidget::~SVInternalLoadsPersonManagerWidget()
{
	delete ui;
}
