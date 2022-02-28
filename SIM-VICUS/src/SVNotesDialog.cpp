#include "SVNotesDialog.h"
#include "ui_SVNotesDialog.h"

SVNotesDialog::SVNotesDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNotesDialog)
{
	m_ui->setupUi(this);
}

SVNotesDialog::~SVNotesDialog() {
	delete m_ui;
}
