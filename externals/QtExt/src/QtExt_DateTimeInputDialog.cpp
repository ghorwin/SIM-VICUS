#include "QtExt_DateTimeInputDialog.h"
#include "ui_QtExt_DateTimeInputDialog.h"

namespace QtExt {

DateTimeInputDialog::DateTimeInputDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::DateTimeInputDialog)
{
	m_ui->setupUi(this);
}


DateTimeInputDialog::~DateTimeInputDialog() {
	delete m_ui;
}


QDate DateTimeInputDialog::requestDate(const QString & title, const QString & label, const QString & dateFormat, QDate * initialValue) {
	// create and configure dialog

	DateTimeInputDialog dlg(nullptr); // top level

	if (title.isEmpty())
		dlg.setWindowTitle(qApp->applicationName());
	else
		dlg.setWindowTitle(title);

	dlg.m_ui->label->setText(label);
	dlg.m_ui->dateEdit->setDisplayFormat(dateFormat);
	if (initialValue != nullptr && initialValue->isValid())
		dlg.m_ui->dateEdit->setDate(*initialValue);

	int res = dlg.exec();

	if (res != QDialog::Accepted)
		return QDate(); // return invalid object
	else
		return dlg.m_ui->dateEdit->date();
}

} // namespace QtExt
