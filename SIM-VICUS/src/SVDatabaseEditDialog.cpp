#include "SVDatabaseEditDialog.h"
#include "ui_SVDatabaseEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QGroupBox>
#include <QTimer>

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVMainWindow.h"
#include "SVAbstractDatabaseEditWidget.h"

// includes for all the dialogs

#include "SVDBMaterialTableModel.h"
#include "SVDBMaterialEditWidget.h"
#include "SVDBConstructionTableModel.h"
#include "SVDBConstructionEditWidget.h"
#include "SVDBComponentTableModel.h"
#include "SVDBComponentEditWidget.h"
#include "SVDBSubSurfaceComponentTableModel.h"
#include "SVDBSubSurfaceComponentEditWidget.h"
#include "SVDBWindowTableModel.h"
#include "SVDBWindowEditWidget.h"
#include "SVDBWindowGlazingSystemTableModel.h"
#include "SVDBWindowGlazingSystemEditWidget.h"
#include "SVDBBoundaryConditionTableModel.h"
#include "SVDBBoundaryConditionEditWidget.h"
#include "SVDBScheduleTableModel.h"
#include "SVDBScheduleEditWidget.h"
#include "SVDBInternalLoadsTableModel.h"
#include "SVDBInternalLoadsPersonEditWidget.h"
#include "SVDBInternalLoadsElectricEquipmentEditWidget.h"
#include "SVDBInternalLoadsLightsEditWidget.h"
#include "SVDBInternalLoadsOtherEditWidget.h"

#include "SVDBZoneControlThermostatEditWidget.h"
#include "SVDBZoneControlThermostatTableModel.h"
#include "SVDBZoneControlShadingEditWidget.h"
#include "SVDBZoneControlShadingTableModel.h"
#include "SVDBZoneControlVentilationNaturalEditWidget.h"
#include "SVDBZoneControlVentilationNaturalTableModel.h"
#include "SVDBInfiltrationEditWidget.h"
#include "SVDBInfiltrationTableModel.h"
#include "SVDBVentilationNaturalEditWidget.h"
#include "SVDBVentilationNaturalTableModel.h"

#include "SVDBNetworkComponentTableModel.h"
#include "SVDBNetworkComponentEditWidget.h"
#include "SVDBPipeTableModel.h"
#include "SVDBPipeEditWidget.h"
#include "SVDBNetworkFluidTableModel.h"
#include "SVDBNetworkFluidEditWidget.h"
#include "SVViewStateHandler.h"
#include "SVGeometryView.h"

#include "Vic3DSceneView.h"

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
	QString newTitle = title;
	if(newTitle.contains(" Database"))
		newTitle = newTitle.mid(0, newTitle.length()-9);

	m_ui->groupBoxTableView->setTitle(newTitle);

	// create groupbox and adjust layout for edit widget
	if (!editWidgetTitle.isEmpty()) {
		QGroupBox * groupBox = new QGroupBox(this);
		groupBox->setTitle(editWidgetTitle);
		m_editWidgetContainerWidget = groupBox;
	}
	else {
		QWidget * wg = new QWidget(this);
		m_editWidgetContainerWidget = wg;
	}
	if (horizontalLayout) {
		m_ui->gridLayoutMaster->addWidget(m_editWidgetContainerWidget, 0, 1);
		m_ui->horizontalLayout->setParent(nullptr);
		m_ui->gridLayoutMaster->addLayout(m_ui->horizontalLayout, 1, 0, 1, 2);
		QSize ewSize = editWidget->sizeHint();
		editWidget->setMinimumWidth(ewSize.width());
		m_ui->gridLayoutMaster->setColumnStretch(0,2);
		m_ui->gridLayoutMaster->setColumnStretch(1,1);
	}
	else {
		m_ui->horizontalLayout->setParent(nullptr);
		m_ui->gridLayoutMaster->addLayout(m_ui->horizontalLayout, 2, 0);
		m_ui->gridLayoutMaster->addWidget(m_editWidgetContainerWidget, 1, 0);
	}

	QVBoxLayout * verticalLay = new QVBoxLayout(m_editWidgetContainerWidget);
	verticalLay->addWidget(editWidget);
	m_editWidgetContainerWidget->setLayout(verticalLay);
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


void SVDatabaseEditDialog::edit(unsigned int initialId) {

	// hide select/cancel buttons, and show "close" button
	m_ui->pushButtonClose->setVisible(true);
	m_ui->pushButtonSelect->setVisible(false);
	m_ui->pushButtonCancel->setVisible(false);

	// ask database model to update its content
	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)
	selectItemById(initialId);
	onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex()); // select nothing

	m_ui->tableView->resizeColumnsToContents();

	exec();
	QTimer::singleShot(0, &SVViewStateHandler::instance(), &SVViewStateHandler::colorRefreshNeeded);
}


unsigned int SVDatabaseEditDialog::select(unsigned int initialId) {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);

	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)
	selectItemById(initialId);
	onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex()); // select nothing

	m_ui->tableView->resizeColumnsToContents();

	int res = exec();
	QTimer::singleShot(0, &SVViewStateHandler::instance(), &SVViewStateHandler::colorRefreshNeeded);
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
		m_editWidgetContainerWidget->setEnabled(false);
		m_editWidget->updateInput(-1); // nothing selected
	}
	else {
		m_editWidgetContainerWidget->setEnabled(true);
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
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
		m_editWidget->updateInput(-1);
	}
}


void SVDatabaseEditDialog::on_tableView_doubleClicked(const QModelIndex &index) {
	if (m_ui->pushButtonSelect->isVisible() && index.isValid())
		accept();
}


void SVDatabaseEditDialog::selectItemById(unsigned int id) {
	// select item with given id
	for (int i=0, count = m_dbModel->rowCount(); i<count; ++i) {
		QModelIndex sourceIndex = m_dbModel->index(i,0);
		if (m_dbModel->data(sourceIndex, Role_Id).toUInt() == id) {
			// get proxy index
			QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
			if (proxyIndex.isValid()) {
				m_ui->tableView->blockSignals(true);
				m_ui->tableView->setCurrentIndex(proxyIndex);
				m_ui->tableView->blockSignals(false);
			}
			break;
		}
	}
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


SVDatabaseEditDialog * SVDatabaseEditDialog::createConstructionEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBConstructionTableModel(parent, SVSettings::instance().m_db),
		new SVDBConstructionEditWidget(parent),
		tr("Construction Database"), QString(), false
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createComponentEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBComponentTableModel(parent, SVSettings::instance().m_db),
		new SVDBComponentEditWidget(parent),
		tr("Component Database"), tr("Component properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createSubSurfaceComponentEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBSubSurfaceComponentTableModel(parent, SVSettings::instance().m_db),
		new SVDBSubSurfaceComponentEditWidget(parent),
		tr("Sub-Surface Component Database"), tr("Sub-Surface properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createWindowEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBWindowTableModel(parent, SVSettings::instance().m_db),
		new SVDBWindowEditWidget(parent),
		tr("Window Database"), tr("Window properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}

SVDatabaseEditDialog * SVDatabaseEditDialog::createWindowGlazingSystemEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBWindowGlazingSystemTableModel(parent, SVSettings::instance().m_db),
		new SVDBWindowGlazingSystemEditWidget(parent),
		tr("Window glazing system Database"), tr("Window glazing system properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}

SVDatabaseEditDialog * SVDatabaseEditDialog::createBoundaryConditionsEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBBoundaryConditionTableModel(parent, SVSettings::instance().m_db),
		new SVDBBoundaryConditionEditWidget(parent),
		tr("Boundary Condition Database"), tr("Boundary condition properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createScheduleEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBScheduleTableModel(parent, SVSettings::instance().m_db),
		new SVDBScheduleEditWidget(parent),
		tr("Schedule Database"), tr("Schedule properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createInternalLoadsEditDialog(QWidget * parent, VICUS::InternalLoad::Category category) {
	SVDatabaseEditDialog * dlg = nullptr;
	switch (category) {
		case VICUS::InternalLoad::IC_Person :
			dlg = new SVDatabaseEditDialog(parent,
				new SVDBInternalLoadsTableModel(parent, SVSettings::instance().m_db, category),
				new SVDBInternalLoadsPersonEditWidget(parent),
				tr("Person Loads Database"), tr("Person load properties"), true);
			break;
		case VICUS::InternalLoad::IC_ElectricEquiment :
			dlg = new SVDatabaseEditDialog(parent,
				new SVDBInternalLoadsTableModel(parent, SVSettings::instance().m_db, category),
				new SVDBInternalLoadsElectricEquipmentEditWidget(parent),
				tr("Electric Equipment Loads Database"), tr("Electric equipment load properties"), true);
			break;
		case VICUS::InternalLoad::IC_Lighting :
			dlg = new SVDatabaseEditDialog(parent,
				new SVDBInternalLoadsTableModel(parent, SVSettings::instance().m_db, category),
				new SVDBInternalLoadsLightsEditWidget(parent),
				tr("Lights Loads Database"), tr("Lights load properties"), true);
			 break;
		case VICUS::InternalLoad::IC_Other :
			dlg = new SVDatabaseEditDialog(parent,
				new SVDBInternalLoadsTableModel(parent, SVSettings::instance().m_db, category),
				new SVDBInternalLoadsOtherEditWidget(parent),
				tr("Other Loads Database"), tr("Other load properties"), true);
			break;
		default:
			Q_ASSERT(false);
	}
	dlg->resize(1400,800);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createZoneControlThermostatEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBZoneControlThermostatTableModel(parent, SVSettings::instance().m_db),
		new SVDBZoneControlThermostatEditWidget(parent),
		tr("Zone Control Thermostat Database"), tr("Zone control thermostat properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createZoneControlVentilationNaturalEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBZoneControlVentilationNaturalTableModel(parent, SVSettings::instance().m_db),
		new SVDBZoneControlVentilationNaturalEditWidget(parent),
		tr("Zone Control Natural Ventilation Database"), tr("Zone Control Natural Ventilation properties"), true
		);
	dlg->resize(1400,800);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createZoneControlShadingEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBZoneControlShadingTableModel(parent, SVSettings::instance().m_db),
		new SVDBZoneControlShadingEditWidget(parent),
		tr("Zone Control Shading Database"), tr("Zone Control Shading properties"), true
		);
	dlg->resize(1400,800);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createVentilationNaturalEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBVentilationNaturalTableModel(parent, SVSettings::instance().m_db),
		new SVDBVentilationNaturalEditWidget(parent),
		tr("Natural Ventilation Database"), tr("Natural Ventilation properties"), true
		);
	dlg->resize(1400,800);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createInfiltrationEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBInfiltrationTableModel(parent, SVSettings::instance().m_db),
		new SVDBInfiltrationEditWidget(parent),
		tr("Infiltration Database"), tr("Infiltration properties"), true
		);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createNetworkComponentEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBNetworkComponentTableModel(parent, SVSettings::instance().m_db),
		new SVDBNetworkComponentEditWidget(parent),
		tr("Network Component Database"), QString(), true ///TODO Hauke muss hier der QString() auch ersetzt werden?
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createPipeEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBPipeTableModel(parent, SVSettings::instance().m_db),
		new SVDBPipeEditWidget(parent),
		tr("Network Pipes Database"), QString(), true///TODO Hauke muss hier der QString() auch ersetzt werden?
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog *SVDatabaseEditDialog::createFluidEditDialog(QWidget *parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBNetworkFluidTableModel(parent, SVSettings::instance().m_db),
		new SVDBNetworkFluidEditWidget(parent),
		tr("Network Fluids Database"), QString(), true///TODO Hauke muss hier der QString() auch ersetzt werden?
	);
	dlg->resize(1400,800);
	return dlg;
}
