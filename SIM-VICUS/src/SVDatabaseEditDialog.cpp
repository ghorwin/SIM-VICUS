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
#include "SVPreferencesDialog.h"
#include "ui_SVDatabaseEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QGroupBox>
#include <QTimer>
#include <QSplitter>
#include <QScreen>


#include "SVSettings.h"
#include "SVStyle.h"
#include "SVPreferencesPageStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVMainWindow.h"
#include "SVAbstractDatabaseEditWidget.h"
#include "SVDBDialogAddDependentElements.h"

// includes for all the dialogs

#include "SVDBMaterialTableModel.h"
#include "SVDBMaterialEditWidget.h"
#include "SVDBEpdTableModel.h"
#include "SVDBEpdEditWidget.h"
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

#include "SVDBSupplySystemEditWidget.h"
#include "SVDBSupplySystemTableModel.h"

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

	m_ui->tableView->installEventFilter(this);

	connect(SVMainWindow::instance().preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged, this, &SVDatabaseEditDialog::onStyleChanged);

	// modify frames and update colors
	m_ui->frameBuildInDB->setFrameShape(QFrame::NoFrame);
	m_ui->frameUserDB->setFrameShape(QFrame::NoFrame);
	// this update colors of frames but also the table view selection color
	onStyleChanged();


	for (int i=0; i<m_dbModel->columnCount(); ++i){
		QString name = m_dbModel->headerData(i, Qt::Horizontal).toString();
		if (name == "") continue; // Skip valid column
		m_ui->comboBoxColumn->addItem(name, i);
	}
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

	// hide buttons that require a project if we do not have any
	bool haveProject = SVProjectHandler::instance().isValid();
	m_ui->toolButtonStoreInUserDB->setVisible(haveProject);
	m_ui->toolButtonRemoveFromUserDB->setVisible(haveProject);

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


unsigned int SVDatabaseEditDialog::select(unsigned int initialId, bool resetModel,  QString filterText, int filterColumn) {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);
	m_ui->pushButtonRemoveUnusedElements->setEnabled(SVProjectHandler::instance().isValid());

	if(resetModel)
		m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)
	selectItemById(initialId);
	onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex()); // select nothing

	m_ui->tableView->resizeColumnsToContents();

	if(filterColumn > 1 && !filterText.isEmpty()) {
		m_currentFilter = filterText;
		m_proxyModel->setFilterKeyColumn(filterColumn);
		m_proxyModel->setFilterWildcard(filterText);
	}

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

	m_proxyModel->setFilterWildcard("");
	m_currentFilter = ""; // Reset filter

	// nothing selected/dialog aborted
	return initialId;
}

bool SVDatabaseEditDialog::eventFilter(QObject * obj, QEvent * event) {
	if(obj == m_ui->tableView && event->type() == QEvent::Resize) {
		// m_ui->tableView->resizeRowsToContents();
	}
	return QObject::eventFilter(obj, event);
}


void SVDatabaseEditDialog::on_pushButtonSelect_clicked() {
	// TODO: check this, performance problem
//	writeUserDB();
	accept();
}


void SVDatabaseEditDialog::on_pushButtonCancel_clicked() {
	reject();
}


void SVDatabaseEditDialog::on_pushButtonClose_clicked() {
	// TODO: check this, performance problem
//	writeUserDB();
	accept();
}


void SVDatabaseEditDialog::on_toolButtonAdd_clicked() {
	m_ui->toolButtonAdd->setFocus(); // move focus to trigger "leave" events in line edits
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
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
	// in case index has not changed after action: we trigger the slot manually to ensure an update of the editing widget
	if (currentProxyIndex == m_ui->tableView->currentIndex())
		onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex());
}


void SVDatabaseEditDialog::on_toolButtonCopy_clicked() {
	m_ui->toolButtonCopy->setFocus(); // move focus to trigger "leave" events in line edits
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
	// in case index has not changed after action: we trigger the slot manually to ensure an update of the editing widget
	if (currentProxyIndex == m_ui->tableView->currentIndex())
		onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex());
}


void SVDatabaseEditDialog::on_toolButtonRemove_clicked() {
	m_ui->toolButtonRemove->setFocus(); // move focus to trigger "leave" events in line edits
	QModelIndex currentProxyIndex = m_ui->tableView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->deleteItem(sourceIndex);
	// last construction removed? clear input widget
	if (m_dbModel->rowCount() == 0)
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
	// in case index has not changed after action: we trigger the slot manually to ensure an update of the editing widget
	if (currentProxyIndex == m_ui->tableView->currentIndex())
		onCurrentIndexChanged(m_ui->tableView->currentIndex(), QModelIndex());
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
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// tell db to drop all user-defined items and re-read the DB
		switch (m_dbModel->databaseType()) {
			case SVDatabase::DT_Materials:				SVSettings::instance().m_db.m_materials.removeUserElements(); break;
			case SVDatabase::DT_Constructions:			SVSettings::instance().m_db.m_constructions.removeUserElements(); break;
			case SVDatabase::DT_Windows:				SVSettings::instance().m_db.m_windows.removeUserElements(); break;
			case SVDatabase::DT_WindowGlazingSystems:	SVSettings::instance().m_db.m_windowGlazingSystems.removeUserElements(); break;
			case SVDatabase::DT_BoundaryConditions:		SVSettings::instance().m_db.m_boundaryConditions.removeUserElements(); break;
			case SVDatabase::DT_Components:				SVSettings::instance().m_db.m_components.removeUserElements(); break;
			case SVDatabase::DT_EpdDatasets:			SVSettings::instance().m_db.m_epdDatasets.removeUserElements(); break;
			case SVDatabase::DT_SubSurfaceComponents:	SVSettings::instance().m_db.m_subSurfaceComponents.removeUserElements(); break;
			case SVDatabase::DT_SurfaceHeating:			SVSettings::instance().m_db.m_surfaceHeatings.removeUserElements(); break;
			case SVDatabase::DT_Pipes:					SVSettings::instance().m_db.m_pipes.removeUserElements(); break;
			case SVDatabase::DT_Fluids:					SVSettings::instance().m_db.m_fluids.removeUserElements(); break;
			case SVDatabase::DT_NetworkComponents:		SVSettings::instance().m_db.m_networkComponents.removeUserElements(); break;
			case SVDatabase::DT_NetworkControllers:		SVSettings::instance().m_db.m_networkControllers.removeUserElements(); break;
			case SVDatabase::DT_SubNetworks:			SVSettings::instance().m_db.m_subNetworks.removeUserElements(); break;
			case SVDatabase::DT_SupplySystems:			SVSettings::instance().m_db.m_supplySystems.removeUserElements(); break;
			case SVDatabase::DT_Schedules:				SVSettings::instance().m_db.m_schedules.removeUserElements(); break;
			case SVDatabase::DT_InternalLoads:			SVSettings::instance().m_db.m_internalLoads.removeUserElements(); break;
			case SVDatabase::DT_ZoneControlThermostat:	SVSettings::instance().m_db.m_zoneControlThermostat.removeUserElements(); break;
			case SVDatabase::DT_ZoneControlShading:		SVSettings::instance().m_db.m_zoneControlShading.removeUserElements(); break;
			case SVDatabase::DT_ZoneControlNaturalVentilation:			SVSettings::instance().m_db.m_zoneControlVentilationNatural.removeUserElements(); break;
			case SVDatabase::DT_ZoneIdealHeatingCooling:	SVSettings::instance().m_db.m_zoneIdealHeatingCooling.removeUserElements(); break;
			case SVDatabase::DT_VentilationNatural:		SVSettings::instance().m_db.m_ventilationNatural.removeUserElements(); break;
			case SVDatabase::DT_Infiltration:			SVSettings::instance().m_db.m_infiltration.removeUserElements(); break;
			case SVDatabase::DT_ZoneTemplates:			SVSettings::instance().m_db.m_zoneTemplates.removeUserElements(); break;
			case SVDatabase::DT_AcousticTemplates:		break;
			case SVDatabase::NUM_DT:; // just to make compiler happy
		}

		SVSettings::instance().m_db.readDatabases(m_dbModel->databaseType()); // by default the "m_isReferenced" property is off after reading the user DB

		// update "isReferenced" property of all elements
		if (SVProjectHandler::instance().isValid())
			SVSettings::instance().m_db.updateReferencedElements(project());

		// tell model to reset completely
		m_dbModel->resetModel();
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
		m_editWidget->updateInput(-1);
	}
}


void SVDatabaseEditDialog::on_toolButtonStoreInUserDB_clicked() {
	m_ui->toolButtonStoreInUserDB->setFocus(); // move focus to trigger "leave" events in line edits
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
	m_ui->toolButtonRemoveFromUserDB->setFocus(); // move focus to trigger "leave" events in line edits
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


void SVDatabaseEditDialog::onStyleChanged() {
	m_ui->frameBuildInDB->setStyleSheet(QString(".QFrame { background-color: %1; }").arg(SVStyle::instance().m_alternativeBackgroundBright.name()));
	m_ui->frameUserDB->setStyleSheet(QString(".QFrame { background-color: %1; }").arg(SVStyle::instance().m_userDBBackgroundBright.name()));
	m_ui->tableView->setStyleSheet(QString("QTableView {selection-background-color: %1;}").arg(SVStyle::instance().m_DBSelectionColor.name()));
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


void SVDatabaseEditDialog::writeUserDB() {
	// write db if modified
	const SVDatabase &db = SVSettings::instance().m_db;
	if ((m_dbModel->databaseType() == SVDatabase::DT_Materials && db.m_materials.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_Constructions && db.m_constructions.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_Windows && db.m_windows.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_WindowGlazingSystems && db.m_windowGlazingSystems.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_BoundaryConditions && db.m_boundaryConditions.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_Components && db.m_components.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_SubSurfaceComponents && db.m_subSurfaceComponents.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_SurfaceHeating && db.m_surfaceHeatings.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_Pipes && db.m_pipes.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_Fluids && db.m_fluids.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_NetworkComponents && db.m_networkComponents.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_NetworkControllers && db.m_networkControllers.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_SubNetworks && db.m_subNetworks.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_SupplySystems && db.m_supplySystems.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_Schedules && db.m_schedules.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_InternalLoads && db.m_internalLoads.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_ZoneControlThermostat && db.m_zoneControlThermostat.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_ZoneControlShading && db.m_zoneControlShading.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_ZoneControlNaturalVentilation && db.m_zoneControlVentilationNatural.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_ZoneIdealHeatingCooling && db.m_zoneIdealHeatingCooling.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_VentilationNatural && db.m_ventilationNatural.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_Infiltration && db.m_infiltration.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_ZoneTemplates && db.m_zoneTemplates.m_modified) ||
		(m_dbModel->databaseType() == SVDatabase::DT_AcousticTemplates && db.m_acousticTemplates.m_modified) )
	db.writeDatabases(m_dbModel->databaseType());
}



// *** Factory functions ***

SVDatabaseEditDialog * SVDatabaseEditDialog::createMaterialEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBMaterialTableModel(parent, SVSettings::instance().m_db),
		new SVDBMaterialEditWidget(parent),
		tr("Material Database"), tr("Material properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}

SVDatabaseEditDialog * SVDatabaseEditDialog::createEpdEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBEpdTableModel(parent, SVSettings::instance().m_db),
		new SVDBEpdEditWidget(parent),
		tr("EPD Database"), tr("EPD properties"), true
	);
	dlg->resize(1400,600);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createConstructionEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBConstructionTableModel(parent, SVSettings::instance().m_db),
		new SVDBConstructionEditWidget(parent),
		tr("Construction Database"), QString(), true
	);
	resizeDBDialog(dlg);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createComponentEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBComponentTableModel(parent, SVSettings::instance().m_db),
		new SVDBComponentEditWidget(parent),
		tr("Component Database"), tr("Component properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createSubSurfaceComponentEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBSubSurfaceComponentTableModel(parent, SVSettings::instance().m_db),
		new SVDBSubSurfaceComponentEditWidget(parent),
		tr("Sub-Surface Component Database"), tr("Sub-Surface properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createWindowEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBWindowTableModel(parent, SVSettings::instance().m_db),
		new SVDBWindowEditWidget(parent),
		tr("Window Database"), tr("Window properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}

SVDatabaseEditDialog * SVDatabaseEditDialog::createWindowGlazingSystemEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBWindowGlazingSystemTableModel(parent, SVSettings::instance().m_db),
		new SVDBWindowGlazingSystemEditWidget(parent),
		tr("Window glazing system Database"), tr("Window glazing system properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}

SVDatabaseEditDialog * SVDatabaseEditDialog::createBoundaryConditionsEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBBoundaryConditionTableModel(parent, SVSettings::instance().m_db),
		new SVDBBoundaryConditionEditWidget(parent),
		tr("Boundary Condition Database"), tr("Boundary condition properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createScheduleEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBScheduleTableModel(parent, SVSettings::instance().m_db),
		new SVDBScheduleEditWidget(parent),
		tr("Schedule Database"), tr("Schedule properties"), true
	);
	resizeDBDialog(dlg);
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
	resizeDBDialog(dlg);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createZoneControlThermostatEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBZoneControlThermostatTableModel(parent, SVSettings::instance().m_db),
		new SVDBZoneControlThermostatEditWidget(parent),
		tr("Zone Control Thermostat Database"), tr("Zone control thermostat properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createZoneControlVentilationNaturalEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBZoneControlVentilationNaturalTableModel(parent, SVSettings::instance().m_db),
		new SVDBZoneControlVentilationNaturalEditWidget(parent),
		tr("Zone Control Natural Ventilation Database"), tr("Zone Control Natural Ventilation properties"), true
		);
	resizeDBDialog(dlg);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createZoneControlShadingEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBZoneControlShadingTableModel(parent, SVSettings::instance().m_db),
		new SVDBZoneControlShadingEditWidget(parent),
		tr("Zone Control Shading Database"), tr("Zone Control Shading properties"), true
		);
	resizeDBDialog(dlg);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createZoneIdealHeatingCoolingEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
										  new SVDBZoneIdealHeatingCoolingTableModel(parent, SVSettings::instance().m_db),
										  new SVDBZoneIdealHeatingCoolingEditWidget(parent),
										  tr("Zone Ideal Heating/Cooling Database"), tr("Zone Ideal Heating/Cooling properties"), true
										  );
	resizeDBDialog(dlg);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createVentilationNaturalEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBVentilationNaturalTableModel(parent, SVSettings::instance().m_db),
		new SVDBVentilationNaturalEditWidget(parent),
		tr("Natural Ventilation Database"), tr("Natural Ventilation properties"), true
		);
	resizeDBDialog(dlg);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createInfiltrationEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBInfiltrationTableModel(parent, SVSettings::instance().m_db),
		new SVDBInfiltrationEditWidget(parent),
		tr("Infiltration Database"), tr("Infiltration properties"), true
		);
	resizeDBDialog(dlg);
	return dlg;
}

SVDatabaseEditDialog *SVDatabaseEditDialog::createSurfaceHeatingSystemEditDialog(QWidget *parent){
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
										  new SVDBSurfaceHeatingTableModel(parent, SVSettings::instance().m_db),
										  new SVDBSurfaceHeatingEditWidget(parent),
										  tr("Surface Heating/Cooling System Database"),
														  tr("Surface Heating/Cooling System properties"), true
										  );
	resizeDBDialog(dlg);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createSupplySystemsEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBSupplySystemTableModel(parent, SVSettings::instance().m_db),
		new SVDBSupplySystemEditWidget(parent),
		tr("Supply System Database"), tr("Supply system properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createNetworkComponentEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBNetworkComponentTableModel(parent, SVSettings::instance().m_db),
		new SVDBNetworkComponentEditWidget(parent),
		tr("Network Component Database"), tr("Network Component Properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}


SVDatabaseEditDialog * SVDatabaseEditDialog::createPipeEditDialog(QWidget * parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBPipeTableModel(parent, SVSettings::instance().m_db),
		new SVDBPipeEditWidget(parent),
		tr("Network Pipes Database"), tr("Network Pipes Properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}


SVDatabaseEditDialog *SVDatabaseEditDialog::createFluidEditDialog(QWidget *parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBNetworkFluidTableModel(parent, SVSettings::instance().m_db),
		new SVDBNetworkFluidEditWidget(parent),
		tr("Network Fluids Database"), tr("Network Fluids Properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}


SVDatabaseEditDialog *SVDatabaseEditDialog::createNetworkControllerEditDialog(QWidget *parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBNetworkControllerTableModel(parent, SVSettings::instance().m_db),
		new SVDBNetworkControllerEditWidget(parent),
		tr("Network Controllers Database"), tr("Network Controllers Properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}


SVDatabaseEditDialog *SVDatabaseEditDialog::createSubNetworkEditDialog(QWidget *parent) {
	SVDatabaseEditDialog * dlg = new SVDatabaseEditDialog(parent,
		new SVDBSubNetworkTableModel(parent, SVSettings::instance().m_db),
		new SVDBSubNetworkEditWidget(parent),
		tr("Sub Networks Database"), tr("Sub Networks Properties"), true
	);
	resizeDBDialog(dlg);
	return dlg;
}


void SVDatabaseEditDialog::resizeDBDialog(QDialog * dlg) {
	QScreen *screen = QGuiApplication::primaryScreen();
	Q_ASSERT(screen!=nullptr);
	QRect rect = screen->geometry();
	dlg->resize(int(0.8*rect.width()), int(0.8*rect.height()));
}


SVAbstractDatabaseTableModel * SVDatabaseEditDialog::dbModel() const {
	return m_dbModel;
}


void SVDatabaseEditDialog::on_toolButtonApplyFilter_clicked() {
	QString filter = m_ui->lineEditFilter->text();

	// Filter Column
	int filterCol = m_ui->comboBoxColumn->currentData().toInt();

	// Set filter
	m_proxyModel->setFilterWildcard(filter);
	m_proxyModel->setFilterKeyColumn(filterCol);
	m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void SVDatabaseEditDialog::on_comboBoxColumn_currentIndexChanged(int /*index*/) {
	m_proxyModel->setFilterWildcard("");
}


void SVDatabaseEditDialog::on_lineEditFilter_returnPressed() {
	on_toolButtonApplyFilter_clicked();
}

