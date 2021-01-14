#ifndef SVDBPIPEEDITWIDGET_H
#define SVDBPIPEEDITWIDGET_H

#include <QWidget>

namespace Ui {
class SVDBPipeEditWidget;
}

class SVDBPipeEditWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVDBPipeEditWidget(QWidget *parent = nullptr);
	~SVDBPipeEditWidget();

private:
	Ui::SVDBPipeEditWidget *ui;
};

#endif // SVDBPIPEEDITWIDGET_H
