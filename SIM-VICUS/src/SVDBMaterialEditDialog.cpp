#include "SVDBMaterialEditDialog.h"
#include "ui_SVDBMaterialEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QDebug>

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVDBMaterialTableModel.h"
#include "SVDBMaterialEditWidget.h"
#include "SVMainWindow.h"

SVDBMaterialEditDialog::SVDBMaterialEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBMaterialEditDialog)
{
	// dialog most only be created by main window
	Q_ASSERT(dynamic_cast<SVMainWindow*>(parent) != nullptr);
	m_ui->setupUi(this);
	m_ui->gridLayoutTableView->setMargin(4);

	SVStyle::formatDatabaseTableView(m_ui->tableView);
	m_ui->tableView->horizontalHeader()->setVisible(true);

	m_dbModel = new SVDBMaterialTableModel(this, SVSettings::instance().m_db);

	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setSourceModel(m_dbModel);
	m_ui->tableView->setModel(m_proxyModel);

	m_ui->editWidget->setup(&SVSettings::instance().m_db, m_dbModel);
	m_ui->verticalLayoutEditWidget->setMargin(0);

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


SVDBMaterialEditDialog::~SVDBMaterialEditDialog() {
	delete m_ui;
}


void SVDBMaterialEditDialog::edit() {

	// hide select/cancel buttons, and show "close" button
	m_ui->pushButtonClose->setVisible(true);
	m_ui->pushButtonSelect->setVisible(false);
	m_ui->pushButtonCancel->setVisible(false);

	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)

	// ask database model to update its content
	// TODO : smart resizing of columns - restore user-defined column widths if adjusted by user
	m_ui->tableView->resizeColumnsToContents();

	m_ui->editWidget->setCurrentTabIndex(0);

	exec();
}


int SVDBMaterialEditDialog::select(unsigned int initialMatId) {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);

	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)

	// select material with given matId
	for (int i=0, count = m_dbModel->rowCount(); i<count; ++i) {
		QModelIndex sourceIndex = m_dbModel->index(i,0);
		if (m_dbModel->data(sourceIndex, Role_Id).toUInt() == initialMatId) {
			// get proxy index
			QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
			if (proxyIndex.isValid())
				m_ui->tableView->setCurrentIndex(proxyIndex);
			break;
		}
	}

	// ask database model to update its content
	// TODO : smart resizing of columns - restore user-defined column widths if adjusted by user
	m_ui->tableView->resizeColumnsToContents();

	m_ui->editWidget->setCurrentTabIndex(0);

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


void SVDBMaterialEditDialog::on_pushButtonSelect_clicked() {
	accept();
}


void SVDBMaterialEditDialog::on_pushButtonCancel_clicked() {
	reject();
}


void SVDBMaterialEditDialog::on_pushButtonClose_clicked() {
	accept();
}


void SVDBMaterialEditDialog::on_toolButtonAdd_clicked() {
	// add new item
	QModelIndex sourceIndex = m_dbModel->addNewItem();
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	// resize ID column
	sourceIndex = m_dbModel->index(0,SVDBMaterialTableModel::ColId);
	proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	if (proxyIndex.isValid())
		m_ui->tableView->resizeColumnToContents(proxyIndex.column());
}


void SVDBMaterialEditDialog::on_toolButtonCopy_clicked() {
	// determine current item
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);

	// add item as copy
	sourceIndex = m_dbModel->copyItem(sourceIndex);
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
}


void SVDBMaterialEditDialog::on_toolButtonRemove_clicked() {
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->deleteItem(sourceIndex);
	// last construction removed? clear input widget
	if (m_dbModel->rowCount() == 0)
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
}


void SVDBMaterialEditDialog::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/) {
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
		// retrieve current material ID
		int matId = current.data(Role_Id).toInt();
		m_ui->editWidget->updateInput(matId);
	}
}


void SVDBMaterialEditDialog::on_pushButtonReloadUserDB_clicked() {
	if (QMessageBox::question(this, QString(), tr("Reloading the user database from file will revert all changes made in this dialog since the program was started. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// tell db to drop all user-defined materials and re-read the construction DB
		SVSettings::instance().m_db.m_materials.removeUserElements();
		SVSettings::instance().m_db.readDatabases(SVDatabase::DT_Materials);
		// tell model to reset completely
		m_dbModel->resetModel();
		m_ui->editWidget->updateInput(-1);
	}
}


void SVDBMaterialEditDialog::showEvent(QShowEvent * /*event*/) {
	// resize splitter
	QList<int> widgetSizes;
	int w = width();
	widgetSizes << w-400 << 400;
	m_ui->splitter->setSizes(widgetSizes);
}


void SVDBMaterialEditDialog::on_tableView_doubleClicked(const QModelIndex &index) {
	if (m_ui->pushButtonSelect->isVisible() && index.isValid())
		accept();
}
