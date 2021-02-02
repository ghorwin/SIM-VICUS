#include "SVDBNetworkComponentEditDialog.h"
#include "ui_SVDBNetworkComponentEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"

#include "SVDBNetworkComponentTableModel.h"
#include "SVDBNetworkComponentEditWidget.h"

SVDBNetworkComponentEditDialog::SVDBNetworkComponentEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBNetworkComponentEditDialog)
{
	m_ui->setupUi(this);

	SVStyle::formatDatabaseTableView(m_ui->tableView);
	m_ui->tableView->horizontalHeader()->setVisible(true);

	m_dbModel = new SVDBNetworkComponentTableModel(this, SVSettings::instance().m_db);

	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setSourceModel(m_dbModel);
	m_ui->tableView->setModel(m_proxyModel);

	m_ui->editWidget->setup(&SVSettings::instance().m_db, m_dbModel);

	connect(m_ui->tableView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
			this, SLOT(onCurrentIndexChanged(const QModelIndex &, const QModelIndex &)) );

	// set item delegate for coloring built-ins
	SVDBModelDelegate * dg = new SVDBModelDelegate(this, Role_BuiltIn);
	m_ui->tableView->setItemDelegate(dg);

	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBNetworkComponentTableModel::ColCheck, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBNetworkComponentTableModel::ColColor, QHeaderView::Fixed);
}


SVDBNetworkComponentEditDialog::~SVDBNetworkComponentEditDialog() {
	delete m_ui;
}


void SVDBNetworkComponentEditDialog::edit(unsigned int initialId) {

	// hide select/cancel buttons, and show "close" button
	m_ui->pushButtonClose->setVisible(true);
	m_ui->pushButtonSelect->setVisible(false);
	m_ui->pushButtonCancel->setVisible(false);

	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)

	// select component with given id
	QModelIndex sourceIndex = m_dbModel->findItem(initialId);
	if (sourceIndex.isValid()) {
		// get proxy index
		QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
		if (proxyIndex.isValid())
			m_ui->tableView->setCurrentIndex(proxyIndex);
	}
	else
		onCurrentIndexChanged(QModelIndex(), QModelIndex());

	m_ui->tableView->resizeColumnsToContents();

	exec();
}


int SVDBNetworkComponentEditDialog::select(unsigned int initialId) {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);

	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)

	// select component with given id
	QModelIndex sourceIndex = m_dbModel->findItem(initialId);
	if (sourceIndex.isValid()) {
		// get proxy index
		QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
		if (proxyIndex.isValid())
			m_ui->tableView->setCurrentIndex(proxyIndex);
	}
	else
		onCurrentIndexChanged(QModelIndex(), QModelIndex());

	m_ui->tableView->resizeColumnsToContents();


	int res = exec();
	if (res == QDialog::Accepted) {
		// determine current item
		QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
		Q_ASSERT(currentProxyIndex.isValid());
		QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);

		// return ID
		return sourceIndex.data(Role_Id).toInt();
	}

	// nothing selected/dialog aborted
	return -1;
}


void SVDBNetworkComponentEditDialog::on_pushButtonSelect_clicked() {
	accept();
}


void SVDBNetworkComponentEditDialog::on_pushButtonCancel_clicked() {
	reject();
}


void SVDBNetworkComponentEditDialog::on_pushButtonClose_clicked() {
	accept();
}


void SVDBNetworkComponentEditDialog::on_toolButtonAdd_clicked() {
	// add new item
	QModelIndex sourceIndex = m_dbModel->addNewItem();
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	// resize ID column
	sourceIndex = m_dbModel->index(0, SVDBNetworkComponentTableModel::ColId);
	proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	if (proxyIndex.isValid())
		m_ui->tableView->resizeColumnToContents(proxyIndex.column());
}


void SVDBNetworkComponentEditDialog::on_toolButtonCopy_clicked() {
	// determine current item
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);

	unsigned int id = m_dbModel->data(sourceIndex, Role_Id).toUInt();
	const VICUS::NetworkComponent * comp = SVSettings::instance().m_db.m_networkComponents[id];

	// add item as copy
	sourceIndex = m_dbModel->addNewItem(*comp);
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
}


void SVDBNetworkComponentEditDialog::on_toolButtonRemove_clicked() {
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->deleteItem(sourceIndex);
	// last boundary condition removed? clear input widget
	if (m_dbModel->rowCount() == 0)
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
}


void SVDBNetworkComponentEditDialog::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/) {
	// if there is no selection, deactivate all buttons that need a selection
	if (!current.isValid()) {
		m_ui->pushButtonSelect->setEnabled(false);
		m_ui->toolButtonRemove->setEnabled(false);
		m_ui->toolButtonCopy->setEnabled(false);
		m_ui->editWidget->updateInput(-1); // nothing selected
	}
	else {
		m_ui->pushButtonSelect->setEnabled(true);
		// remove is not allowed for built-ins
		QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
		m_ui->toolButtonRemove->setEnabled(!sourceIndex.data(Role_BuiltIn).toBool());

		m_ui->toolButtonCopy->setEnabled(true);
		m_ui->tableView->selectRow(current.row());
		// retrieve current component ID
		int compId = current.data(Role_Id).toInt();
		m_ui->editWidget->updateInput(compId);
	}
}



void SVDBNetworkComponentEditDialog::on_pushButtonReloadUserDB_clicked() {
	if (QMessageBox::question(this, QString(), tr("Reloading the user database from file will revert all changes made in this dialog since the program was started. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// tell db to drop all user-defined elements and re-read the DB
		SVSettings::instance().m_db.m_networkComponents.removeUserElements();
		SVSettings::instance().m_db.readDatabases(SVDatabase::DT_NetworkComponents);
		// tell model to reset completely
		m_dbModel->resetModel();
		m_ui->editWidget->updateInput(-1);
	}
}


void SVDBNetworkComponentEditDialog::showEvent(QShowEvent * event) {
	QWidget::showEvent(event);
	// now resize name column to span the available space
	int width = m_ui->tableView->width()-2;
	width -= m_ui->tableView->columnWidth(0);
	width -= m_ui->tableView->columnWidth(1);
	width -= m_ui->tableView->columnWidth(2);
	width -= m_ui->tableView->columnWidth(4);
	m_ui->tableView->setColumnWidth(3, width);
}
