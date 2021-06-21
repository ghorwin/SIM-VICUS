#ifndef SVDBNETWORKCONTROLLEREDITWIDGET_H
#define SVDBNETWORKCONTROLLEREDITWIDGET_H

#include <QDialog>

namespace Ui {
class SVDBNetworkControllerEditWidget;
}

class SVDBNetworkControllerEditWidget : public QDialog
{
	Q_OBJECT

public:
	explicit SVDBNetworkControllerEditWidget(QWidget *parent = nullptr);
	~SVDBNetworkControllerEditWidget();

private:
	Ui::SVDBNetworkControllerEditWidget *ui;
};

#endif // SVDBNETWORKCONTROLLEREDITWIDGET_H
