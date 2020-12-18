#include "SVDBConstructionEditDialog.h"
#include "ui_SVDBConstructionEditDialog.h"

#include "SVSettings.h"
#include "SVDBConstructionTreeModel.h"

#include "SVDBConstructionEditWidget.h"

SVDBConstructionEditDialog::SVDBConstructionEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBConstructionEditDialog)
{
	m_ui->setupUi(this);

	m_constructionTreeModel = SVSettings::instance().constructionTreeModel();

	// \todo insert sortfilterproxymodel later

	m_ui->treeView->setModel(m_constructionTreeModel);
}


SVDBConstructionEditDialog::~SVDBConstructionEditDialog() {
	delete m_ui;
}


void SVDBConstructionEditDialog::edit() {

	// hide select/cancel buttons, and show "close" button
	m_ui->pushButtonClose->setVisible(true);
	m_ui->pushButtonSelect->setVisible(false);
	m_ui->pushButtonCancel->setVisible(false);

	// ask database model to update its content

	exec();
}


unsigned int SVDBConstructionEditDialog::select() {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);

	int res = exec();
	if (res == QDialog::Accepted) {
		// get selected construction

		// return ID
	}

	// nothing selected/dialog aborted
	return 0;
}


void SVDBConstructionEditDialog::on_pushButtonSelect_clicked() {
	accept();
}


void SVDBConstructionEditDialog::on_pushButtonCancel_clicked() {
	reject();
}


void SVDBConstructionEditDialog::on_pushButtonClose_clicked() {
	accept();
}


void SVDBConstructionEditDialog::on_toolButtonAdd_clicked() {
	// add new construction

}


void SVDBConstructionEditDialog::on_toolButtonCopy_clicked() {

}


void SVDBConstructionEditDialog::on_toolButtonRemove_clicked() {

}


void SVDBConstructionEditDialog::updateTreeWidget() {

}
