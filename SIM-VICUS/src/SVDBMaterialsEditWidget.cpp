#include "SVDBMaterialsEditWidget.h"
#include "ui_SVDBMaterialsEditWidget.h"

SVDBMaterialsEditWidget::SVDBMaterialsEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBMaterialsEditWidget)
{
	m_ui->setupUi(this);
}


SVDBMaterialsEditWidget::~SVDBMaterialsEditWidget() {
	delete m_ui;
}


void SVDBMaterialsEditWidget::edit() {
	// update UI to current state of database in settings


	// finally show the widget
	show();
}

void SVDBMaterialsEditWidget::on_toolButtonAdd_clicked()
{

}

void SVDBMaterialsEditWidget::on_toolButtonCopy_clicked()
{

}
