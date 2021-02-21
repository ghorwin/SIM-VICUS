#ifndef SVDBSCHEDULEDAILYCYCLEEDITWIDGET_H
#define SVDBSCHEDULEDAILYCYCLEEDITWIDGET_H

#include <QWidget>

namespace Ui {
class SVDBScheduleDailyCycleEditWidget;
}

class SVDBScheduleDailyCycleEditWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVDBScheduleDailyCycleEditWidget(QWidget *parent = nullptr);
	~SVDBScheduleDailyCycleEditWidget();

private:
	Ui::SVDBScheduleDailyCycleEditWidget *m_ui;
};

#endif // SVDBSCHEDULEDAILYCYCLEEDITWIDGET_H
