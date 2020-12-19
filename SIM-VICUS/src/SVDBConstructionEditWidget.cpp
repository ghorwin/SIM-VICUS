#include "SVDBConstructionEditWidget.h"
#include "ui_SVDBConstructionEditWidget.h"

SVDBConstructionEditWidget::SVDBConstructionEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBConstructionEditWidget)
{
	m_ui->setupUi(this);
}


SVDBConstructionEditWidget::~SVDBConstructionEditWidget() {
	delete m_ui;
}


void SVDBConstructionEditWidget::setConstruction(const VICUS::Construction & con) {

}

