#include "SVDBInternalLoadsPersonEditDialog.h"
#include "ui_SVDBInternalLoadsPersonEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QDebug>

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVDBInternalLoadTableModel.h"
#include "SVDBInternalLoadsPersonEditWidget.h"
#include "SVMainWindow.h"

SVDBInternalLoadsPersonEditDialog::SVDBInternalLoadsPersonEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBInternalLoadsPersonEditDialog)
{
	// Must only be created from main window. */
	Q_ASSERT(dynamic_cast<SVMainWindow*>(parent) != nullptr);
	m_ui->setupUi(this);
	m_ui->gridLayoutTableView->setMargin(4);

	SVStyle::formatDatabaseTableView(m_ui->tableView);
	m_ui->tableView->horizontalHeader()->setVisible(true);

	m_dbModel = new SVDBInternalLoadTableModel(this, SVSettings::instance().m_db, SVDBInternalLoadTableModel::T_Person);

	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setSourceModel(m_dbModel);
	m_ui->tableView->setModel(m_proxyModel);

	m_ui->editWidget->setup(&SVSettings::instance().m_db, m_dbModel);
	m_ui->verticalLayoutEditWidget->setMargin(0);

	// specific setup for internal loads DB table

	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBInternalLoadTableModel::ColId, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBInternalLoadTableModel::ColCheck, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBInternalLoadTableModel::ColName, QHeaderView::Stretch);

	connect(m_ui->tableView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
			this, SLOT(onCurrentIndexChanged(const QModelIndex &, const QModelIndex &)) );

	resize(1400,600);

	// set item delegate for coloring built-ins
	SVDBModelDelegate * dg = new SVDBModelDelegate(this, Role_BuiltIn);
	m_ui->tableView->setItemDelegate(dg);
}


SVDBInternalLoadsPersonEditDialog::~SVDBInternalLoadsPersonEditDialog() {
	delete m_ui;
}


void SVDBInternalLoadsPersonEditDialog::edit() {

	// hide select/cancel buttons, and show "close" button
	m_ui->pushButtonClose->setVisible(true);
	m_ui->pushButtonSelect->setVisible(false);
	m_ui->pushButtonCancel->setVisible(false);

	m_ui->tableView->blockSignals(true);
	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)
	m_ui->tableView->blockSignals(false);

	// ask database model to update its content
	m_ui->tableView->resizeColumnsToContents();
	onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex());

	exec();
}


int SVDBInternalLoadsPersonEditDialog::select(unsigned int initialIntLoadId) {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);

	m_ui->tableView->blockSignals(true);
	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)

	// select InternalLoadsPerson with given internal loads Id
	for (int i=0, count = m_dbModel->rowCount(); i<count; ++i) {
		QModelIndex sourceIndex = m_dbModel->index(i,0);
		if (m_dbModel->data(sourceIndex, Role_Id).toUInt() == initialIntLoadId) {
			// get proxy index
			QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
			if (proxyIndex.isValid())
				m_ui->tableView->setCurrentIndex(proxyIndex);
			break;
		}
	}
	m_ui->tableView->blockSignals(false);

	// ask database model to update its content
	m_ui->tableView->resizeColumnsToContents();

	onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex());

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


void SVDBInternalLoadsPersonEditDialog::on_pushButtonSelect_clicked() {
	accept();
}


void SVDBInternalLoadsPersonEditDialog::on_pushButtonCancel_clicked() {
	reject();
}


void SVDBInternalLoadsPersonEditDialog::on_pushButtonClose_clicked() {
	accept();
}


void SVDBInternalLoadsPersonEditDialog::on_toolButtonAdd_clicked() {

	// add new item
	QModelIndex sourceIndex = m_dbModel->addNewItem();
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	// finally select the newly added item
	m_ui->tableView->selectionModel()->blockSignals(true);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	m_ui->tableView->selectionModel()->blockSignals(false);
	onCurrentIndexChanged(proxyIndex, QModelIndex());
	// resize ID column
	sourceIndex = m_dbModel->index(0,SVDBInternalLoadTableModel::ColId);
	proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	if (proxyIndex.isValid())
		m_ui->tableView->resizeColumnToContents(proxyIndex.column());
}


void SVDBInternalLoadsPersonEditDialog::on_toolButtonCopy_clicked() {

	// determine current item
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);

	unsigned int id = m_dbModel->data(sourceIndex, Role_Id).toUInt();
	const VICUS::InternalLoad* intLoad = SVSettings::instance().m_db.m_internalLoads[id];

	// add item as copy
	sourceIndex = m_dbModel->addNewItem(*intLoad);
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	m_ui->tableView->selectionModel()->blockSignals(true);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	m_ui->tableView->selectionModel()->blockSignals(false);
	onCurrentIndexChanged(proxyIndex, QModelIndex());
}


void SVDBInternalLoadsPersonEditDialog::on_toolButtonRemove_clicked() {
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->deleteItem(sourceIndex);
	// last internal loads removed? clear input widget
	if (m_dbModel->rowCount() == 0)
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
}


void SVDBInternalLoadsPersonEditDialog::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/) {

	// if there is no selection, deactivate all buttons that need a selection
	if (!current.isValid()) {
		m_ui->pushButtonSelect->setEnabled(false);
		m_ui->toolButtonRemove->setEnabled(false);
		m_ui->toolButtonCopy->setEnabled(false);
		m_ui->groupBoxProperties->setEnabled(false);
		m_ui->editWidget->updateInput(-1); // nothing selected
	}
	else {
		m_ui->groupBoxProperties->setEnabled(true);
		m_ui->pushButtonSelect->setEnabled(true);
		// remove is not allowed for built-ins
		QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
		m_ui->toolButtonRemove->setEnabled(!sourceIndex.data(Role_BuiltIn).toBool());

		m_ui->toolButtonCopy->setEnabled(true);
		m_ui->tableView->selectRow(current.row());
		// retrieve current InternalLoadsPerson ID
		int intLoadId = current.data(Role_Id).toInt();
		m_ui->editWidget->updateInput(intLoadId);
	}
}


void SVDBInternalLoadsPersonEditDialog::on_pushButtonReloadUserDB_clicked() {
	if (QMessageBox::question(this, QString(), tr("Reloading the user database from file will revert all changes made in this dialog since the program was started. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// tell db to drop all user-defined InternalLoadsPersons and re-read the InternalLoadsPersons DB
		SVSettings::instance().m_db.m_internalLoads.removeUserElements();
		SVSettings::instance().m_db.readDatabases(SVDatabase::DT_InternalLoads);
		// tell model to reset completely
		m_dbModel->resetModel();
		onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex());
	}
}


void SVDBInternalLoadsPersonEditDialog::on_tableView_doubleClicked(const QModelIndex &index) {
	if (m_ui->pushButtonSelect->isVisible() && index.isValid())
		accept();
}
