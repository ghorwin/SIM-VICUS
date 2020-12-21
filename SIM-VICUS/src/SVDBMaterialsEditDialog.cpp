#include "SVDBMaterialsEditDialog.h"
#include "ui_SVDBMaterialsEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QDebug>

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVDBMaterialTableModel.h"
#include "SVDBMaterialsEditWidget.h"

SVDBMaterialsEditDialog::SVDBMaterialsEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBMaterialsEditDialog)
{
	m_ui->setupUi(this);

	SVStyle::formatDatabaseTableView(m_ui->tableView);
	m_ui->tableView->horizontalHeader()->setVisible(true);

	m_dbModel = new SVDBMaterialTableModel(this, SVSettings::instance().m_db);

	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setSourceModel(m_dbModel);
	m_ui->tableView->setModel(m_proxyModel);

//	m_ui->editWidget->setup(&SVSettings::instance().m_db, m_dbModel);

	// specific setup for construction DB table

	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBMaterialTableModel::ColId, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBMaterialTableModel::ColCheck, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBMaterialTableModel::ColName, QHeaderView::Stretch);

	connect(m_ui->tableView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
			this, SLOT(onCurrentIndexChanged(const QModelIndex &, const QModelIndex &)) );

	// set item delegate for coloring built-ins
	SVDBModelDelegate * dg = new SVDBModelDelegate(this, Role_BuiltIn);
	m_ui->tableView->setItemDelegate(dg);
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
	// TODO : smart resizing of columns - restore user-defined column widths if adjusted by user
	m_ui->tableView->resizeColumnsToContents();

	qDebug() << m_ui->tableView->columnWidth(SVDBMaterialTableModel::ColCheck);

	exec();
}


unsigned int SVDBMaterialsEditDialog::select() {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);

	int res = exec();
	if (res == QDialog::Accepted) {
		// determine current item
		QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
		Q_ASSERT(currentProxyIndex.isValid());
		QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);

		// return ID
		return sourceIndex.data(Role_Id).toUInt();
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
	// add new item
	QModelIndex sourceIndex = m_dbModel->addNewItem();
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	m_ui->tableView->selectRow(proxyIndex.row());
}


void SVDBMaterialsEditDialog::on_toolButtonCopy_clicked() {
	// determine current item
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);

	unsigned int id = m_dbModel->data(sourceIndex, Role_Id).toUInt();
	const VICUS::Material * mat = SVSettings::instance().m_db.m_materials[id];

	// add item as copy
	sourceIndex = m_dbModel->addNewItem(*mat);
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
}


void SVDBMaterialsEditDialog::on_toolButtonRemove_clicked() {
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->deleteItem(sourceIndex);
	// last construction removed? clear input widget
	if (m_dbModel->rowCount() == 0)
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
}


void SVDBMaterialsEditDialog::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/) {
	// if there is no selection, deactivate all buttons that need a selection
	if (!current.isValid()) {
		m_ui->pushButtonSelect->setEnabled(false);
		m_ui->toolButtonRemove->setEnabled(false);
		m_ui->toolButtonCopy->setEnabled(false);
//		m_ui->editWidget->updateInput(-1); // nothing selected
	}
	else {
		m_ui->pushButtonSelect->setEnabled(true);
		// remove is not allowed for built-ins
		QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
		m_ui->toolButtonRemove->setEnabled(!sourceIndex.data(Role_BuiltIn).toBool());

		m_ui->toolButtonCopy->setEnabled(true);
		m_ui->tableView->selectRow(current.row());
		// retrieve current construction ID
		int conId = current.data(Role_Id).toInt();
//		m_ui->editWidget->updateInput(conId);
	}
}



void SVDBMaterialsEditDialog::on_pushButtonReloadUserDB_clicked() {
	if (QMessageBox::question(this, QString(), tr("Reloading the user database from file will revert all changes made in this dialog since the program was started. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// tell db to drop all user-defined materials and re-read the construction DB
		SVSettings::instance().m_db.m_materials.removeUserElements();
		SVSettings::instance().m_db.readDatabases(SVDatabase::DT_Materials);
		// tell model to reset completely
		m_dbModel->resetModel();
	}

}


