#ifndef SVINTERNALLOADSPERSONMANAGERWIDGET_H
#define SVINTERNALLOADSPERSONMANAGERWIDGET_H

#include <QWidget>

namespace Ui {
class SVInternalLoadsPersonManagerWidget;
}

class SVInternalLoadsPersonManagerWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVInternalLoadsPersonManagerWidget(QWidget *parent = nullptr);
	~SVInternalLoadsPersonManagerWidget();

private:
	Ui::SVInternalLoadsPersonManagerWidget *ui;
};

#endif // SVINTERNALLOADSPERSONMANAGERWIDGET_H
