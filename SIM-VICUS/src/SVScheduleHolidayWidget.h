#ifndef SVSCHEDULEHOLIDAYWIDGET_H
#define SVSCHEDULEHOLIDAYWIDGET_H

#include <QWidget>

namespace Ui {
class SVScheduleHolidayWidget;
}

class SVScheduleHolidayWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVScheduleHolidayWidget(QWidget *parent = nullptr);
	~SVScheduleHolidayWidget();

private:
	Ui::SVScheduleHolidayWidget *ui;
};

#endif // SVSCHEDULEHOLIDAYWIDGET_H
