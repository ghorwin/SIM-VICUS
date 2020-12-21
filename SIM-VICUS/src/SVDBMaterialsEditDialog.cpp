#include "SVDBMaterialsEditDialog.h"
#include "ui_SVDBMaterialsEditDialog.h"

SVDBMaterialsEditDialog::SVDBMaterialsEditDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SVDBMaterialsEditDialog)
{
	ui->setupUi(this);
}

SVDBMaterialsEditDialog::~SVDBMaterialsEditDialog()
{
	delete ui;
}
