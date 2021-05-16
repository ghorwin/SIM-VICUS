#include "SVScheduleHolidayWidget.h"
#include "ui_SVScheduleHolidayWidget.h"

SVScheduleHolidayWidget::SVScheduleHolidayWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVScheduleHolidayWidget)
{
	m_ui->setupUi(this);
}


SVScheduleHolidayWidget::~SVScheduleHolidayWidget() {
	delete m_ui;
}
