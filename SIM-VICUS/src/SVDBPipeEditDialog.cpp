#include "SVDBPipeEditDialog.h"
#include "ui_SVDBPipeEditDialog.h"

SVDBPipeEditDialog::SVDBPipeEditDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SVDBPipeEditDialog)
{
	ui->setupUi(this);
}

SVDBPipeEditDialog::~SVDBPipeEditDialog()
{
	delete ui;
}
