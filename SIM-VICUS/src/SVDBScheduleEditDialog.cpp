#include "SVDBScheduleEditDialog.h"
#include "ui_SVDBScheduleEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"

#include "SVDBScheduleTableModel.h"
#include "SVDBScheduleEditWidget.h"
#include "SVDBScheduleAddDialog.h"

SVDBScheduleEditDialog::SVDBScheduleEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBScheduleEditDialog)
{
	m_ui->setupUi(this);

	SVStyle::formatDatabaseTableView(m_ui->tableView);
	m_ui->tableView->horizontalHeader()->setVisible(true);

	m_dbModel = new SVDBScheduleTableModel(this, SVSettings::instance().m_db);

	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setSourceModel(m_dbModel);
	m_ui->tableView->setModel(m_proxyModel);

	m_ui->editWidget->setup(&SVSettings::instance().m_db, m_dbModel);

	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBScheduleTableModel::ColId, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBScheduleTableModel::ColCheck, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBScheduleTableModel::ColAnnualSplineData, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBScheduleTableModel::ColInterpolation, QHeaderView::Fixed);
	m_ui->tableView->horizontalHeader()->setSectionResizeMode(SVDBScheduleTableModel::ColName, QHeaderView::Stretch);

	connect(m_ui->tableView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
			this, SLOT(onCurrentIndexChanged(const QModelIndex &, const QModelIndex &)) );

	resize(1400,600);

	m_ui->editWidget->layout()->setMargin(0);

	QList<int> sizes;
	int widthSchedule = 250;
	int availableWidth = m_ui->splitter->width();
	sizes << widthSchedule << (availableWidth - widthSchedule);
	m_ui->splitter->setSizes(sizes);

	// set item delegate for coloring built-ins
	SVDBModelDelegate * dg = new SVDBModelDelegate(this, Role_BuiltIn);
	m_ui->tableView->setItemDelegate(dg);
}


SVDBScheduleEditDialog::~SVDBScheduleEditDialog() {
	delete m_ui;
}

///TODO Andreas es wird noch nicht direkt das valid symbol in der Auswahltabelle der Zeitpläne angepasst sobald der Zeitplan valid ist. Dazu muss man einmal die ScheduleZeile wechseln
/// Bitte anpassen

void SVDBScheduleEditDialog::edit() {

	// hide select/cancel buttons, and show "close" button
	m_ui->pushButtonClose->setVisible(true);
	m_ui->pushButtonSelect->setVisible(false);
	m_ui->pushButtonCancel->setVisible(false);

	m_ui->tableView->blockSignals(true);
	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)
	m_ui->tableView->blockSignals(false);

	// resize columns
	m_ui->tableView->resizeColumnsToContents();
	onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex());

	exec();
}


int SVDBScheduleEditDialog::select(unsigned int initialId) {
	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);

	m_ui->tableView->blockSignals(true);
	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)

	// select boundary condition with given schedule Id
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


void SVDBScheduleEditDialog::on_pushButtonSelect_clicked(){
	accept();
}


void SVDBScheduleEditDialog::on_pushButtonCancel_clicked() {
	reject();
}


void SVDBScheduleEditDialog::on_pushButtonClose_clicked() {
	accept();
}


void SVDBScheduleEditDialog::on_toolButtonAdd_clicked() {
	// add new item
	VICUS::Schedule newSchedule;
	newSchedule.m_displayName.setEncodedString("en:<new schedule>");
	if (!SVDBScheduleAddDialog::requestScheduleData(tr("Add new Schedule"), newSchedule ) )
		return;
	QModelIndex sourceIndex = m_dbModel->addNewItem(newSchedule);
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	// resize ID column
	sourceIndex = m_dbModel->index(0,SVDBScheduleTableModel::ColId);
	proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	if (proxyIndex.isValid())
		m_ui->tableView->resizeColumnToContents(proxyIndex.column());
	// finally select the newly added schedule
	m_ui->tableView->selectionModel()->blockSignals(true);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	m_ui->tableView->selectionModel()->blockSignals(false);
	onCurrentIndexChanged(proxyIndex, QModelIndex());
}


void SVDBScheduleEditDialog::on_toolButtonCopy_clicked() {
	// determine current item
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);

	unsigned int id = m_dbModel->data(sourceIndex, Role_Id).toUInt();
	const VICUS::Schedule * sched = SVSettings::instance().m_db.m_schedules[id];

	// add item as copy
	sourceIndex = m_dbModel->addNewItem(*sched);
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->blockSignals(true);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	m_ui->tableView->selectionModel()->blockSignals(false);
	onCurrentIndexChanged(proxyIndex, QModelIndex());
}


void SVDBScheduleEditDialog::on_toolButtonRemove_clicked() {
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->deleteItem(sourceIndex);
	// last boundary condition removed? clear input widget
	if (m_dbModel->rowCount() == 0)
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
}


void SVDBScheduleEditDialog::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/) {
	// if there is no selection, deactivate all buttons that need a selection
	if (!current.isValid()) {
		m_ui->pushButtonSelect->setEnabled(false);
		m_ui->toolButtonRemove->setEnabled(false);
		m_ui->toolButtonCopy->setEnabled(false);
		m_ui->groupBoxScheduleProperties->setEnabled(false);
		m_ui->editWidget->updateInput(-1); // nothing selected
	}
	else {
		m_ui->groupBoxScheduleProperties->setEnabled(true);
		m_ui->pushButtonSelect->setEnabled(true);
		// remove is not allowed for built-ins
		QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
		m_ui->toolButtonRemove->setEnabled(!sourceIndex.data(Role_BuiltIn).toBool());

		m_ui->toolButtonCopy->setEnabled(true);
		m_ui->tableView->selectRow(current.row());
		// retrieve current schedule ID
		int schedId = current.data(Role_Id).toInt();
		m_ui->editWidget->updateInput(schedId);
	}
}


void SVDBScheduleEditDialog::on_pushButtonReloadUserDB_clicked() {
	if (QMessageBox::question(this, QString(), tr("Reloading the user database from file will revert all changes made in this dialog since the program was started. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// tell db to drop all user-defined elements and re-read the DB
		SVSettings::instance().m_db.m_schedules.removeUserElements();
		SVSettings::instance().m_db.readDatabases(SVDatabase::DT_Schedules);
		// tell model to reset completely
		m_dbModel->resetModel();
		onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex());
	}
}


void SVDBScheduleEditDialog::on_tableView_doubleClicked(const QModelIndex &index) {
	if (m_ui->pushButtonSelect->isVisible() && index.isValid())
		accept();
}
