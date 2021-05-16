#ifndef SVSCHEDULEHOLIDAYWIDGET_H
#define SVSCHEDULEHOLIDAYWIDGET_H

#include <QWidget>

namespace Ui {
	class SVScheduleHolidayWidget;
}

// TODO Dirk, wird das gebraucht oder kann das weg?
class SVScheduleHolidayWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVScheduleHolidayWidget(QWidget *parent = nullptr);
	~SVScheduleHolidayWidget();

private:
	Ui::SVScheduleHolidayWidget *m_ui;
};

#endif // SVSCHEDULEHOLIDAYWIDGET_H
