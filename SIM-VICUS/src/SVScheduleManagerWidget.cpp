#include "SVScheduleManagerWidget.h"
#include "ui_SVScheduleManagerWidget.h"

SVScheduleManagerWidget::SVScheduleManagerWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SVScheduleManagerWidget)
{
	ui->setupUi(this);
}

SVScheduleManagerWidget::~SVScheduleManagerWidget()
{
	delete ui;
}
