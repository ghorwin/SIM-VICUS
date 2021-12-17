/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVDatabaseEditDialog.h"
#include "ui_SVDatabaseEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QGroupBox>
#include <QTimer>
#include <QSplitter>

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVMainWindow.h"
#include "SVAbstractDatabaseEditWidget.h"
#include "SVDBDialogAddDependentElements.h"

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
#include "SVDBZoneIdealHeatingCoolingEditWidget.h"
#include "SVDBZoneIdealHeatingCoolingTableModel.h"
#include "SVDBInfiltrationEditWidget.h"
#include "SVDBInfiltrationTableModel.h"
#include "SVDBVentilationNaturalEditWidget.h"
#include "SVDBVentilationNaturalTableModel.h"

#include "SVDBSurfaceHeatingEditWidget.h"
#include "SVDBSurfaceHeatingTableModel.h"

#include "SVDBNetworkComponentTableModel.h"
#include "SVDBNetworkComponentEditWidget.h"
#include "SVDBPipeTableModel.h"
#include "SVDBPipeEditWidget.h"
#include "SVDBNetworkFluidTableModel.h"
#include "SVDBNetworkFluidEditWidget.h"
#include "SVDBNetworkControllerEditWidget.h"
#include "SVDBNetworkControllerTableModel.h"
#include "SVDBSubNetworkEditWidget.h"
#include "SVDBSubNetworkTableModel.h"

#include "SVViewStateHandler.h"
#include "SVGeometryView.h"

#include "Vic3DSceneView.h"

SVDatabaseEditDialog::SVDatabaseEditDialog(QWidget *parent, SVAbstractDatabaseTableModel * tableModel,
										   SVAbstractDatabaseEditWidget * editWidget,
										   const QString & title, const QString & editWidgetTitle,
										   bool horizontalLayout) :
	QDialog(parent
#ifdef Q_OS_LINUX
			, Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint /*| Qt::WindowSystemMenuHint*/
#endif
			),
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

	// TODO Dirk, i18n fix
	QString newTitle = title;
	if (newTitle.contains(" Database"))
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
	// splitter contains group box and custom edit widget
	QSplitter * split = new QSplitter(this);
	split->setOrientation(horizontalLayout ? Qt::Horizontal : Qt::Vertical);
	split->addWidget(m_ui->groupBoxTableView);
	split->addWidget(m_editWidgetContainerWidget);
	split->setCollapsible(0, false);
	split->setCollapsible(1, false);
	m_ui->verticalLayout->insertWidget(0, split);

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
	SVDBModelDelegate * dg = new SVDBModelDelegate(this, Role_BuiltIn, Role_Local, Role_Referenced);
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
	m_ui->pushButtonRemoveUnusedElements->setEnabled(SVProjectHandler::instance().isValid());

	// ask database model to update its content
	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)
	selectItemById(initialId);
	onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex()); // select nothing

	m_ui->tableView->resizeColumnsToContents();

	// update "isRferenced" property of all elements
	if (SVProjectHandler::instance().isValid()){
		SVSettings::instance().m_db.updateReferencedElements(project());
	}

	exec();
	QTimer::singleShot(0, &SVViewStateHandler::instance(), &SVViewStateHandler::refreshColors);
}


unsigned int SVDatabaseEditDialog::select(unsigned int initialId) {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);
	m_ui->pushButtonRemoveUnusedElements->setEnabled(SVProjectHandler::instance().isValid());

	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)
	selectItemById(initialId);
	onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex()); // select nothing

	m_ui->tableView->resizeColumnsToContents();

	// update "isRferenced" property of all elements
	if (SVProjectHandler::instance().isValid()){
		SVSettings::instance().m_db.updateReferencedElements(project());
	}

	int res = exec();
	QTimer::singleShot(0, &SVViewStateHandler::instance(), &SVViewStateHandler::refreshColors);
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
	// if we have a project loaded, keep this item as "local", otherwise make it a user-db element directly
	if (!SVProjectHandler::instance().isValid())
		m_dbModel->setItemLocal(sourceIndex, false);
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
	// if we have a project loaded, keep this item as "local", otherwise make it a user-db element directly
	if (!SVProjectHandler::instance().isValid())
		m_dbModel->setItemLocal(sourceIndex, false);
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
		m_ui->toolButtonStoreInUserDB->setEnabled(false);
		m_ui->toolButtonRemoveFromUserDB->setEnabled(false);
		m_editWidgetContainerWidget->setEnabled(false);
		m_editWidget->updateInput(-1); // nothing selected
	}
	else {
		m_editWidgetContainerWidget->setEnabled(true);
		m_ui->pushButtonSelect->setEnabled(true);

		// remove is not allowed for built-ins
		QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
		bool builtIn = sourceIndex.data(Role_BuiltIn).toBool();
		m_ui->toolButtonRemove->setEnabled(!builtIn);

		// only elements which are local and not built-in can be stored to user DB
		bool local = sourceIndex.data(Role_Local).toBool();
		m_ui->toolButtonStoreInUserDB->setEnabled(local && !builtIn);
		m_ui->toolButtonRemoveFromUserDB->setEnabled(!local && !builtIn);

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
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
		// tell db to drop all user-defined items and re-read the DB
		SVSettings::instance().m_db.m_materials.removeUserElements();
		SVSettings::instance().m_db.readDatabases(m_dbModel->databaseType());
		// update "isRferenced" property of all elements
		if (SVProjectHandler::instance().isValid()){
			SVSettings::instance().m_db.updateReferencedElements(project());
		}
		// tell model to reset completely
		m_dbModel->resetModel();
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
		m_editWidget->updateInput(-1);
	}
}


void SVDatabaseEditDialog::on_toolButtonStoreInUserDB_clicked() {
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->setItemLocal(sourceIndex, false);
	onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex());

	// find local children
	unsigned int id = sourceIndex.data(Role_Id).toUInt();
	std::set<VICUS::AbstractDBElement *> localChildren;
	SVSettings::instance().m_db.findLocalChildren(m_dbModel->databaseType(), id, localChildren);

	// ask user if child elements should be added to user DB as well
	if (localChildren.size() > 0) {
		SVDBDialogAddDependentElements diag(this);
		diag.setup(localChildren);
		int res = diag.exec();
		if (res == QDialog::Accepted) {
			for (VICUS::AbstractDBElement *el: localChildren)
				el->m_local = false;
		}
	}
}


void SVDatabaseEditDialog::on_toolButtonRemoveFromUserDB_clicked() {
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->setItemLocal(sourceIndex, true);
	onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex());
}


void SVDatabaseEditDialog::on_tableView_doubleClicked(const QModelIndex &index) {
	if (m_ui->pushButtonSelect->isVisible() && index.isValid())
		accept();
}


void SVDatabaseEditDialog::on_pushButtonRemoveUnusedElements_clicked() {
	if (QMessageBox::question(this, QString(), tr("All elements that are currently not used in the project will be deleted. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
		// tell db to drop all user-defined items and re-read the DB
		if (SVProjectHandler::instance().isValid())
			SVSettings::instance().m_db.removeNotReferencedLocalElements(m_dbModel->databaseType(), project());
		// tell model to reset completely
		m_dbModel->resetModel();
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
		m_editWidget->updateInput(-1);
	}
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

SVDatabaseEditDialog *SVDatabaseEditDialog::createZoneIdealHeatingCoolingEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
										  new SVDBZoneIdealHeatingCoolingTableModel(parent, SVSettings::instance().m_db),
										  new SVDBZoneIdealHeatingCoolingEditWidget(parent),
										  tr("Zone Ideal Heating/Cooling Database"), tr("Zone Ideal Heating/Cooling properties"), true
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

SVDatabaseEditDialog *SVDatabaseEditDialog::createSurfaceHeatingSystemEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
										  new SVDBSurfaceHeatingTableModel(parent, SVSettings::instance().m_db),
										  new SVDBSurfaceHeatingEditWidget(parent),
										  tr("Surface Heating/Cooling System Database"),
														  tr("Surface Heating/Cooling System properties"), true
										  );
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createNetworkComponentEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBNetworkComponentTableModel(parent, SVSettings::instance().m_db),
		new SVDBNetworkComponentEditWidget(parent),
		tr("Network Component Database"), tr("Network Component Properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createPipeEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBPipeTableModel(parent, SVSettings::instance().m_db),
		new SVDBPipeEditWidget(parent),
		tr("Network Pipes Database"), tr("Network Pipes Properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog *SVDatabaseEditDialog::createFluidEditDialog(QWidget *parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBNetworkFluidTableModel(parent, SVSettings::instance().m_db),
		new SVDBNetworkFluidEditWidget(parent),
		tr("Network Fluids Database"), tr("Network Fluids Properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog *SVDatabaseEditDialog::createNetworkControllerEditDialog(QWidget *parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBNetworkControllerTableModel(parent, SVSettings::instance().m_db),
		new SVDBNetworkControllerEditWidget(parent),
		tr("Network Controllers Database"), tr("Network Controllers Properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}


SVDatabaseEditDialog *SVDatabaseEditDialog::createSubNetworkEditDialog(QWidget *parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBSubNetworkTableModel(parent, SVSettings::instance().m_db),
		new SVDBSubNetworkEditWidget(parent),
		tr("Sub Networks Database"), tr("Sub Networks Properties"), true
	);
	dlg->resize(1400,800);
	return dlg;
}
