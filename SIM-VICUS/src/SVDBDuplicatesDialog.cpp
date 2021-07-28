#include "SVDBDuplicatesDialog.h"
#include "ui_SVDBDuplicatesDialog.h"

SVDBDuplicatesDialog::SVDBDuplicatesDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBDuplicatesDialog)
{
	m_ui->setupUi(this);
}


SVDBDuplicatesDialog::~SVDBDuplicatesDialog() {
	delete m_ui;
}


void SVDBDuplicatesDialog::removeDuplicates(SVDatabase::DatabaseTypes dbType) {
	// populate table

}
