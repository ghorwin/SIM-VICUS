#include "SVInternalLoadsPersonDetailedWidget.h"
#include "ui_SVInternalLoadsPersonDetailedWidget.h"

SVInternalLoadsPersonDetailedWidget::SVInternalLoadsPersonDetailedWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SVInternalLoadsPersonDetailedWidget)
{
	ui->setupUi(this);
}

SVInternalLoadsPersonDetailedWidget::~SVInternalLoadsPersonDetailedWidget()
{
	delete ui;
}
