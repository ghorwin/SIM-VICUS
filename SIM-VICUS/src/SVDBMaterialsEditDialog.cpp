#include "SVDBMaterialsEditDialog.h"
#include "ui_SVDBMaterialsEditDialog.h"

#include "SVSettings.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>

#include "SVDBMaterialsEditWidget.h"
#include "SVStyle.h"

SVDBMaterialsEditDialog::SVDBMaterialsEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBMaterialsEditDialog)
{
	m_ui->setupUi(this);
	m_ui->setupUi(this);

//	m_ui->editWidget->setup(&SVSettings::instance().m_db, m_dbModel);

	// specific setup for construction DB table

//	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColId, QHeaderView::Fixed);
//	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColCheck, QHeaderView::Fixed);
//	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColName, QHeaderView::Stretch);
//	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColNumLayers, QHeaderView::ResizeToContents);
//	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColUValue, QHeaderView::ResizeToContents);

//	m_ui->tableView->setColumnWidth(SVDBConstructionTableModel::ColId, 60);
//#if defined(Q_OS_MAC)
//	m_ui->tableView->setColumnWidth(ConstructionDBModel::ColCheck, 26);
//#else
//	m_ui->tableView->setColumnWidth(SVDBConstructionTableModel::ColCheck, 22);
//#endif
//	m_ui->tableView->setColumnWidth(SVDBConstructionTableModel::ColName, 120);

//	connect(m_ui->tableView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
//			this, SLOT(onCurrentIndexChanged(const QModelIndex &, const QModelIndex &)) );

//	// set item delegate for coloring built-ins
//	SVDBModelDelegate * dg = new SVDBModelDelegate(this, SVDBConstructionTableModel::Role_BuiltIn);
//	m_ui->tableView->setItemDelegate(dg);

}


SVDBMaterialsEditDialog::~SVDBMaterialsEditDialog() {
	delete m_ui;
}


void SVDBMaterialsEditDialog::edit() {

	// hide select/cancel buttons, and show "close" button
	m_ui->pushButtonClose->setVisible(true);
	m_ui->pushButtonSelect->setVisible(false);
	m_ui->pushButtonCancel->setVisible(false);

	// ask database model to update its content
//	m_ui->tableView->resizeColumnsToContents();

	exec();
}


unsigned int SVDBMaterialsEditDialog::select() {

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


void SVDBMaterialsEditDialog::on_pushButtonSelect_clicked() {
	accept();
}


void SVDBMaterialsEditDialog::on_pushButtonCancel_clicked() {
	reject();
}


void SVDBMaterialsEditDialog::on_pushButtonClose_clicked() {
	accept();
}


void SVDBMaterialsEditDialog::on_toolButtonAdd_clicked() {
//	// add new construction
//	QModelIndex sourceIndex = m_dbModel->addNewItem();
//	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
//	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
//	m_ui->tableView->selectRow(proxyIndex.row());
}


void SVDBMaterialsEditDialog::on_toolButtonCopy_clicked() {
//	// determine current item
//	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
//	Q_ASSERT(currentProxyIndex.isValid());
//	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);

//	unsigned int id = m_dbModel->data(sourceIndex, SVDBConstructionTableModel::Role_Id).toUInt();
//	const VICUS::Construction * con = SVSettings::instance().m_db.m_constructions[id];

//	// add item as copy
//	sourceIndex = m_dbModel->addNewItem(*con);
//	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
//	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
}


void SVDBMaterialsEditDialog::on_toolButtonRemove_clicked() {
//	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
//	Q_ASSERT(currentProxyIndex.isValid());
//	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
//	m_dbModel->deleteItem(sourceIndex);
//	// last construction removed? clear input widget
//	if (m_dbModel->rowCount() == 0)
//		onCurrentIndexChanged(QModelIndex(), QModelIndex());
}


void SVDBMaterialsEditDialog::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/) {
//	// if there is no selection, deactivate all buttons that need a selection
//	if (!current.isValid()) {
//		m_ui->pushButtonSelect->setEnabled(false);
//		m_ui->toolButtonRemove->setEnabled(false);
//		m_ui->toolButtonCopy->setEnabled(false);
//		m_ui->editWidget->updateInput(-1); // nothing selected
//	}
//	else {
//		m_ui->pushButtonSelect->setEnabled(true);
//		// remove is not allowed for built-ins
//		QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
//		m_ui->toolButtonRemove->setEnabled(!sourceIndex.data(SVDBConstructionTableModel::Role_BuiltIn).toBool());

//		m_ui->toolButtonCopy->setEnabled(true);
//		m_ui->tableView->selectRow(current.row());
//		// retrieve current construction ID
//		int conId = current.data(SVDBConstructionTableModel::Role_Id).toInt();
//		m_ui->editWidget->updateInput(conId);
//	}
}



void SVDBMaterialsEditDialog::on_pushButtonReloadUserDB_clicked() {
	if (QMessageBox::question(this, QString(), tr("Reloading the user database from file will revert all changes made in this dialog since the program was started. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// tell db to drop all user-defined materials and re-read the construction DB
		SVSettings::instance().m_db.m_constructions.removeUserElements();
		SVSettings::instance().m_db.readDatabases(SVDatabase::DT_Constructions);
		// tell model to reset completely
//		m_dbModel->resetModel();
	}

}


