#include "SVScheduleHolidayWidget.h"
#include "ui_SVScheduleHolidayWidget.h"

SVScheduleHolidayWidget::SVScheduleHolidayWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SVScheduleHolidayWidget)
{
	ui->setupUi(this);
}

SVScheduleHolidayWidget::~SVScheduleHolidayWidget()
{
	delete ui;
}
