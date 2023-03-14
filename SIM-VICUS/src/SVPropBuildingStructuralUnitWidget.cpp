#include "SVPropBuildingStructuralUnitWidget.h"
#include "ui_SVPropBuildingStructuralUnitWidget.h"

SVPropBuildingStructuralUnitWidget::SVPropBuildingStructuralUnitWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SVPropBuildingStructuralUnitWidget)
{
    ui->setupUi(this);
}

SVPropBuildingStructuralUnitWidget::~SVPropBuildingStructuralUnitWidget()
{
    delete ui;
}
