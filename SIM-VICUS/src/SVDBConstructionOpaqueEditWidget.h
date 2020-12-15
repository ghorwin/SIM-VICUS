#ifndef SVDBCONSTRUCTIONOPAQUEEDITWIDGET_H
#define SVDBCONSTRUCTIONOPAQUEEDITWIDGET_H

#include <QWidget>

namespace Ui {
class SVDBConstructionOpaqueEditWidget;
}

class SVDBConstructionOpaqueEditWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVDBConstructionOpaqueEditWidget(QWidget *parent = nullptr);
	~SVDBConstructionOpaqueEditWidget();

private:
	Ui::SVDBConstructionOpaqueEditWidget *ui;
};

#endif // SVDBCONSTRUCTIONOPAQUEEDITWIDGET_H
