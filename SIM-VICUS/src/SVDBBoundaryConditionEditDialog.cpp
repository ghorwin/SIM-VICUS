#include "SVDBBoundaryConditionEditDialog.h"
#include "ui_SVDBBoundaryConditionEditDialog.h"

SVDBBoundaryConditionEditDialog::SVDBBoundaryConditionEditDialog(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SVDBBoundaryConditionEditDialog)
{
	ui->setupUi(this);
}

SVDBBoundaryConditionEditDialog::~SVDBBoundaryConditionEditDialog()
{
	delete ui;
}
