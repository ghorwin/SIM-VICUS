#include "SVDBWindowEditDialog.h"
#include "ui_SVDBWindowEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
//#include "SVDBWindowTableModel.h"
//#include "SVDBWindowEditWidget.h"
#include "SVMainWindow.h"

SVDBWindowEditDialog::SVDBWindowEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBWindowEditDialog)
{
	// Must only be created from main window. */
	Q_ASSERT(dynamic_cast<SVMainWindow*>(parent) != nullptr);
	m_ui->setupUi(this);

	SVStyle::formatDatabaseTableView(m_ui->tableView);
	m_ui->tableView->horizontalHeader()->setVisible(true);

	// TODO : Dirk, uncomment when implemented
#if 0

	m_dbModel = new SVDBConstructionTableModel(this, SVSettings::instance().m_db);

	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setSourceModel(m_dbModel);
	m_ui->tableView->setModel(m_proxyModel);

	m_ui->editWidget->setup(&SVSettings::instance().m_db, m_dbModel);

	// specific setup for construction DB table

	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColId, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColCheck, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColName, QHeaderView::Stretch);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColNumLayers, QHeaderView::ResizeToContents);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColUValue, QHeaderView::ResizeToContents);
#endif

	connect(m_ui->tableView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
			this, SLOT(onCurrentIndexChanged(const QModelIndex &, const QModelIndex &)) );

	// set item delegate for coloring built-ins
	SVDBModelDelegate * dg = new SVDBModelDelegate(this, Role_BuiltIn);
	m_ui->tableView->setItemDelegate(dg);

}


SVDBWindowEditDialog::~SVDBWindowEditDialog() {
	delete m_ui;
}


void SVDBWindowEditDialog::edit() {

	// hide select/cancel buttons, and show "close" button
	m_ui->pushButtonClose->setVisible(true);
	m_ui->pushButtonSelect->setVisible(false);
	m_ui->pushButtonCancel->setVisible(false);

	// TODO : Dirk, uncomment when implemented
//	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)

	// resize columns
	m_ui->tableView->resizeColumnsToContents();

	exec();
}

int SVDBWindowEditDialog::select(unsigned int initialId) {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);

	// TODO : Dirk, uncomment when implemented
#if 0
	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)

	// select construction with given matId
	for (int i=0, count = m_dbModel->rowCount(); i<count; ++i) {
		QModelIndex sourceIndex = m_dbModel->index(i,0);
		if (m_dbModel->data(sourceIndex, Role_Id).toUInt() == initialId) {
			// get proxy index
			QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
			if (proxyIndex.isValid())
				m_ui->tableView->setCurrentIndex(proxyIndex);
			break;
		}
	}
#endif
	// ask database model to update its content
	// TODO : smart resizing of columns - restore user-defined column widths if adjusted by user
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


void SVDBWindowEditDialog::on_pushButtonSelect_clicked() {
	accept();
}


void SVDBWindowEditDialog::on_pushButtonCancel_clicked() {
	reject();
}


void SVDBWindowEditDialog::on_pushButtonClose_clicked() {
	accept();
}


void SVDBWindowEditDialog::on_toolButtonAdd_clicked() {
	// TODO : Dirk, uncomment when implemented
#if 0
	// add new construction
	QModelIndex sourceIndex = m_dbModel->addNewItem();
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	// resize ID column
	sourceIndex = m_dbModel->index(0,SVDBConstructionTableModel::ColId);
	proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	if (proxyIndex.isValid())
		m_ui->tableView->resizeColumnToContents(proxyIndex.column());
#endif
}


void SVDBWindowEditDialog::on_toolButtonCopy_clicked() {
	// TODO : Dirk, uncomment when implemented
#if 0
	// determine current item
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);

	unsigned int id = m_dbModel->data(sourceIndex, Role_Id).toUInt();
	const VICUS::Construction * con = SVSettings::instance().m_db.m_constructions[id];

	// add item as copy
	sourceIndex = m_dbModel->addNewItem(*con);
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
#endif
}


void SVDBWindowEditDialog::on_toolButtonRemove_clicked() {
	// TODO : Dirk, uncomment when implemented
#if 0
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->deleteItem(sourceIndex);
	// last construction removed? clear input widget
	if (m_dbModel->rowCount() == 0)
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
#endif
}


void SVDBWindowEditDialog::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/) {
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
		// retrieve current construction ID
		int conId = current.data(Role_Id).toInt();
		m_ui->editWidget->updateInput(conId);
	}
}


void SVDBWindowEditDialog::on_pushButtonReloadUserDB_clicked() {
	// TODO : Dirk, uncomment when implemented
#if 0
	if (QMessageBox::question(this, QString(), tr("Reloading the user database from file will revert all changes made in this dialog since the program was started. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// tell db to drop all user-defined materials and re-read the construction DB
		SVSettings::instance().m_db.m_constructions.removeUserElements();
		SVSettings::instance().m_db.readDatabases(SVDatabase::DT_Windows);
		// tell model to reset completely
		m_dbModel->resetModel();
		m_ui->editWidget->updateInput(-1);
	}
#endif
}


void SVDBWindowEditDialog::on_tableView_doubleClicked(const QModelIndex &index) {
	if (m_ui->pushButtonSelect->isVisible() && index.isValid())
		accept();
}
