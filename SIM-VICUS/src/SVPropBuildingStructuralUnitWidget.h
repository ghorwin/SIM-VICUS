#ifndef SVPROPBUILDINGSTRUCTURALUNITWIDGET_H
#define SVPROPBUILDINGSTRUCTURALUNITWIDGET_H

#include <QWidget>

namespace Ui {
class SVPropBuildingStructuralUnitWidget;
}

class SVPropBuildingStructuralUnitWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SVPropBuildingStructuralUnitWidget(QWidget *parent = nullptr);
    ~SVPropBuildingStructuralUnitWidget();

private:
    Ui::SVPropBuildingStructuralUnitWidget *ui;
};

#endif // SVPROPBUILDINGSTRUCTURALUNITWIDGET_H
