#include "SVDBConstructionEditDialog.h"
#include "ui_SVDBConstructionEditDialog.h"

#include "SVSettings.h"
#include "SVDBConstructionTableModel.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>

#include "SVDBConstructionEditWidget.h"
#include "SVStyle.h"
#include "SVDBModelDelegate.h"

SVDBConstructionEditDialog::SVDBConstructionEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBConstructionEditDialog)
{
	m_ui->setupUi(this);

	SVStyle::formatDatabaseTableView(m_ui->tableView);

	m_dbModel = new DBConstructionTableModel(this, SVSettings::instance().m_db);

	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setSourceModel(m_dbModel);
	m_ui->tableView->setModel(m_proxyModel);

	// specific setup for construction DB table

	m_ui->tableView->horizontalHeader()->setSectionResizeMode(DBConstructionTableModel::ColId, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(DBConstructionTableModel::ColCheck, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(DBConstructionTableModel::ColName, QHeaderView::Stretch);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(DBConstructionTableModel::ColNumLayers, QHeaderView::ResizeToContents);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(DBConstructionTableModel::ColUValue, QHeaderView::ResizeToContents);

	m_ui->tableView->setColumnWidth(DBConstructionTableModel::ColId, 60);
#if defined(Q_OS_MAC)
	m_ui->tableView->setColumnWidth(ConstructionDBModel::ColCheck, 26);
#else
	m_ui->tableView->setColumnWidth(DBConstructionTableModel::ColCheck, 22);
#endif
	m_ui->tableView->setColumnWidth(DBConstructionTableModel::ColName, 120);
	m_ui->tableView->setAlternatingRowColors(true);

	// set item delegate for coloring built-ins
	DBModelDelegate * dg = new DBModelDelegate(this, DBConstructionTableModel::Role_BuiltIn);
	m_ui->tableView->setItemDelegate(dg);

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

//	onSelectionChanged(QItemSelection(), QItemSelection());
	exec();
}


unsigned int SVDBConstructionEditDialog::select() {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);

//	onSelectionChanged(QItemSelection(), QItemSelection());

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


//void SVDBConstructionEditDialog::onCurrentIndexChanged(QModelIndex current,QModelIndex) {
//	// if there is no selection, deactivate all buttons that need a selection
//	if (current.isValid()) {
//		m_ui->pushButtonSelect->setEnabled(false);
//		m_ui->toolButtonRemove->setEnabled(false);
//		m_ui->toolButtonCopy->setEnabled(false);

//		// hide edit widget and show placeholder
//		m_ui->label->show();
//		m_ui->editWidget->hide();
//		m_ui->verticalLayout->addStretch(1);
//	}
//	else {
//		m_ui->pushButtonSelect->setEnabled(true);
//		m_ui->toolButtonRemove->setEnabled(true);
//		m_ui->toolButtonCopy->setEnabled(true);

//		// show and activate edit widget
//		m_ui->label->hide();
//		m_ui->editWidget->show();
//		m_ui->verticalLayout->removeItem(m_ui->verticalLayout->itemAt(2));
//	}
//}


