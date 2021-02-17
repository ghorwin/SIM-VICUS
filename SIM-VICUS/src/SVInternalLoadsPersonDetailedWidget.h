#ifndef SVINTERNALLOADSPERSONDETAILEDWIDGET_H
#define SVINTERNALLOADSPERSONDETAILEDWIDGET_H

#include <QWidget>

namespace Ui {
class SVInternalLoadsPersonDetailedWidget;
}

class SVInternalLoadsPersonDetailedWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVInternalLoadsPersonDetailedWidget(QWidget *parent = nullptr);
	~SVInternalLoadsPersonDetailedWidget();

private:
	Ui::SVInternalLoadsPersonDetailedWidget *ui;
};

#endif // SVINTERNALLOADSPERSONDETAILEDWIDGET_H
