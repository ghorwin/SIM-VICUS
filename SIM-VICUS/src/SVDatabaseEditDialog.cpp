#include "SVDatabaseEditDialog.h"
#include "ui_SVDatabaseEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QGroupBox>

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVMainWindow.h"
#include "SVAbstractDatabaseEditWidget.h"

// includes for all the dialogs

#include "SVDBMaterialTableModel.h"
#include "SVDBMaterialEditWidget.h"



SVDatabaseEditDialog::SVDatabaseEditDialog(QWidget *parent, SVAbstractDatabaseTableModel * tableModel,
										   SVAbstractDatabaseEditWidget * editWidget,
										   const QString & title, const QString & editWidgetTitle,
										   bool horizontalLayout) :
	QDialog(parent),
	m_ui(new Ui::SVDatabaseEditDialog),
	m_dbModel(tableModel),
	m_editWidget(editWidget)
{
	// dialog most only be created by main window
	Q_ASSERT(dynamic_cast<SVMainWindow*>(parent) != nullptr);
	m_ui->setupUi(this);
	m_ui->gridLayoutTableView->setMargin(4);

	setWindowTitle(title);

	SVStyle::formatDatabaseTableView(m_ui->tableView);
	m_ui->tableView->horizontalHeader()->setVisible(true);

	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setSourceModel(dynamic_cast<QAbstractTableModel*>(m_dbModel));
	m_ui->tableView->setModel(m_proxyModel);

	// create groupbox and adjust layout for edit widget
	m_editWidgetGroupBox = new QGroupBox(this);
	m_editWidgetGroupBox->setTitle(editWidgetTitle);
	if (horizontalLayout) {
		m_ui->gridLayoutMaster->addWidget(m_editWidgetGroupBox, 0, 1);
		m_ui->horizontalLayout->setParent(nullptr);
		m_ui->gridLayoutMaster->addLayout(m_ui->horizontalLayout, 1, 0, 1, 2);
	}
	else {
		m_ui->horizontalLayout->setParent(nullptr);
		m_ui->gridLayoutMaster->addLayout(m_ui->horizontalLayout, 2, 0);
		m_ui->gridLayoutMaster->addWidget(m_editWidgetGroupBox, 1, 0);
	}

	QVBoxLayout * verticalLay = new QVBoxLayout(m_editWidgetGroupBox);
	verticalLay->addWidget(editWidget);
	m_editWidgetGroupBox->setLayout(verticalLay);
	verticalLay->setMargin(0);
	editWidget->setup(&SVSettings::instance().m_db, m_dbModel);

	// specific setup for DB table view
	m_dbModel->setColumnResizeModes(m_ui->tableView);

	connect(m_ui->tableView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
			this, SLOT(onCurrentIndexChanged(const QModelIndex &, const QModelIndex &)) );

	// set item delegate for coloring built-ins
	SVDBModelDelegate * dg = new SVDBModelDelegate(this, Role_BuiltIn);
	m_ui->tableView->setItemDelegate(dg);
}


SVDatabaseEditDialog::~SVDatabaseEditDialog() {
	delete m_ui;
}


void SVDatabaseEditDialog::edit() {

	// hide select/cancel buttons, and show "close" button
	m_ui->pushButtonClose->setVisible(true);
	m_ui->pushButtonSelect->setVisible(false);
	m_ui->pushButtonCancel->setVisible(false);

	// ask database model to update its content
	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)

	m_ui->tableView->resizeColumnsToContents();

	exec();
}


unsigned int SVDatabaseEditDialog::select(unsigned int initialId) {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);

	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)

	// select item with given id
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

	m_ui->tableView->resizeColumnsToContents();

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
	return VICUS::INVALID_ID;
}


void SVDatabaseEditDialog::on_pushButtonSelect_clicked() {
	accept();
}


void SVDatabaseEditDialog::on_pushButtonCancel_clicked() {
	reject();
}


void SVDatabaseEditDialog::on_pushButtonClose_clicked() {
	accept();
}


void SVDatabaseEditDialog::on_toolButtonAdd_clicked() {
	// add new item
	QModelIndex sourceIndex = m_dbModel->addNewItem();
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	// resize ID column
	sourceIndex = m_dbModel->index(0, m_dbModel->columnIndexId());
	proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	if (proxyIndex.isValid())
		m_ui->tableView->resizeColumnToContents(proxyIndex.column());
}


void SVDatabaseEditDialog::on_toolButtonCopy_clicked() {
	// determine current item
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	sourceIndex = m_dbModel->copyItem(sourceIndex);
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->tableView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
}


void SVDatabaseEditDialog::on_toolButtonRemove_clicked() {
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->deleteItem(sourceIndex);
	// last construction removed? clear input widget
	if (m_dbModel->rowCount() == 0)
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
}


void SVDatabaseEditDialog::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/) {
	// if there is no selection, deactivate all buttons that need a selection
	if (!current.isValid()) {
		m_ui->pushButtonSelect->setEnabled(false);
		m_ui->toolButtonRemove->setEnabled(false);
		m_ui->toolButtonCopy->setEnabled(false);
		m_editWidgetGroupBox->setEnabled(false);
		m_editWidget->updateInput(-1); // nothing selected
	}
	else {
		m_editWidgetGroupBox->setEnabled(true);
		m_ui->pushButtonSelect->setEnabled(true);
		// remove is not allowed for built-ins
		QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
		m_ui->toolButtonRemove->setEnabled(!sourceIndex.data(Role_BuiltIn).toBool());

		m_ui->toolButtonCopy->setEnabled(true);
		m_ui->tableView->selectRow(current.row());
		// retrieve current ID
		int id = current.data(Role_Id).toInt();
		m_editWidget->updateInput(id);
	}
}


void SVDatabaseEditDialog::on_pushButtonReloadUserDB_clicked() {
	if (QMessageBox::question(this, QString(), tr("Reloading the user database from file will revert all changes "
												  "made in this dialog since the program was started. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// tell db to drop all user-defined items and re-read the DB
		SVSettings::instance().m_db.m_materials.removeUserElements();
		SVSettings::instance().m_db.readDatabases(m_dbModel->databaseType());
		// tell model to reset completely
		m_dbModel->resetModel();
		m_editWidgetGroupBox->setEnabled(false);
		m_editWidget->updateInput(-1);
	}
}


void SVDatabaseEditDialog::on_tableView_doubleClicked(const QModelIndex &index) {
	if (m_ui->pushButtonSelect->isVisible() && index.isValid())
		accept();
}


// *** Factory functions ***

SVDatabaseEditDialog * SVDatabaseEditDialog::createMaterialEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBMaterialTableModel(parent, SVSettings::instance().m_db),
		new SVDBMaterialEditWidget(parent),
		tr("Material Database"), tr("Material properties"), true
	);
	dlg->resize(1400,600);
	return dlg;
}
