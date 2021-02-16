#ifndef SVSCHEDULEEDITWIDGET_H
#define SVSCHEDULEEDITWIDGET_H

#include <QWidget>

namespace Ui {
class SVScheduleEditWidget;
}

class SVScheduleEditWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVScheduleEditWidget(QWidget *parent = nullptr);
	~SVScheduleEditWidget();

private:
	Ui::SVScheduleEditWidget *ui;
};

#endif // SVSCHEDULEEDITWIDGET_H
