#include "SVOutputGridEditDialog.h"
#include "ui_SVOutputGridEditDialog.h"

SVOutputGridEditDialog::SVOutputGridEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVOutputGridEditDialog)
{
	m_ui->setupUi(this);
}


SVOutputGridEditDialog::~SVOutputGridEditDialog() {
	delete m_ui;
}


bool SVOutputGridEditDialog::edit(NANDRAD::OutputGrid & grid, int index) {
	return true;
}
