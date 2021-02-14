#ifndef SVFLOORMANAGERWIDGET_H
#define SVFLOORMANAGERWIDGET_H

#include <QWidget>

namespace Ui {
class SVFloorManagerWidget;
}

class SVFloorManagerWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVFloorManagerWidget(QWidget *parent = nullptr);
	~SVFloorManagerWidget();

private slots:
	void on_treeWidget_itemSelectionChanged();

private:
	Ui::SVFloorManagerWidget *m_ui;
};

#endif // SVFLOORMANAGERWIDGET_H
