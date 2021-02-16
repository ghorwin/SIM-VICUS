#ifndef SVSCHEDULEMANAGERWIDGET_H
#define SVSCHEDULEMANAGERWIDGET_H

#include <QWidget>

namespace Ui {
class SVScheduleManagerWidget;
}

class SVScheduleManagerWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVScheduleManagerWidget(QWidget *parent = nullptr);
	~SVScheduleManagerWidget();

private:
	Ui::SVScheduleManagerWidget *ui;
};

#endif // SVSCHEDULEMANAGERWIDGET_H
