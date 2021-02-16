#include "SVScheduleEditWidget.h"
#include "ui_SVScheduleEditWidget.h"

SVScheduleEditWidget::SVScheduleEditWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SVScheduleEditWidget)
{
	ui->setupUi(this);
}

SVScheduleEditWidget::~SVScheduleEditWidget()
{
	delete ui;
}
