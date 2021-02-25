#include "SVPropEditCopyDialog.h"
#include "ui_SVPropEditCopyDialog.h"


SVPropEditCopyDialog::SVPropEditCopyDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVPropEditCopyDialog)
{
	m_ui->setupUi(this);

//	connect( this, SIGNAL(on_pushButton1_clicked()), this, SLOT(accept()) );
//	connect( this, &SVPropEditCopyDialog::on_pushButton2_clicked, this, &SVPropEditCopyDialog::reject );
}

SVPropEditCopyDialog::~SVPropEditCopyDialog() {
	delete m_ui;
}

int SVPropEditCopyDialog::requestCopyMethod(const QString &title, const QString &buttonTitle1, const QString &buttonTitle2) {
	SVPropEditCopyDialog dlg(nullptr); // top level

	if (title.isEmpty())
		dlg.setWindowTitle(qApp->applicationName());
	else
		dlg.setWindowTitle(title);

	dlg.m_ui->pushButton1->setText(buttonTitle1);
	dlg.m_ui->pushButton2->setText(buttonTitle2);

	int res = dlg.exec();

	if (res != QDialog::Accepted)
		return -1; // return invalid object
	else
		return (int)dlg.m_copyMethod;
}


void SVPropEditCopyDialog::on_pushButton1_clicked() {
	m_copyMethod = true;
	QDialog::accept();
}

void SVPropEditCopyDialog::on_pushButton2_clicked() {
	m_copyMethod = false;
	QDialog::accept();
}
