#include "SVSmartSelectDialog.h"
#include "ui_SVSmartSelectDialog.h"

SVSmartSelectDialog::SVSmartSelectDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSmartSelectDialog)
{
	m_ui->setupUi(this);

	m_ui->verticalLayoutNetwork->setMargin(0);
}


SVSmartSelectDialog::~SVSmartSelectDialog() {
	delete m_ui;
}
