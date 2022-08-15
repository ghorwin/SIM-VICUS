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

#include "SVDBZoneTemplateEditDialog.h"
#include "ui_SVDBZoneTemplateEditDialog.h"

#include <QItemSelectionModel>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QTimer>
#include <QGroupBox>

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVDBDialogAddDependentElements.h"

#include "SVDBZoneTemplateEditWidget.h"
#include "SVDBZoneTemplateTreeModel.h"


SVDBZoneTemplateEditDialog::SVDBZoneTemplateEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBZoneTemplateEditDialog)
{
	// dialog most only be created by main window
	m_ui->setupUi(this);
	m_ui->groupBox->layout()->setMargin(4);

	m_dbModel = new SVDBZoneTemplateTreeModel(this, SVSettings::instance().m_db);
	m_editWidget = new SVDBZoneTemplateEditWidget(this);

	SVStyle::formatDatabaseTreeView(m_ui->treeView);

	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setSourceModel(m_dbModel);
	m_ui->treeView->setModel(m_proxyModel);

	// create groupbox and adjust layout for edit widget
	m_groupBox = new QGroupBox(this);
	m_groupBox->setTitle(tr("Zone template properties"));
	m_ui->gridLayoutMaster->addWidget(m_groupBox, 0, 1);
	m_ui->horizontalLayout->setParent(nullptr);
	m_ui->gridLayoutMaster->addLayout(m_ui->horizontalLayout, 1, 0, 1, 2);

	QVBoxLayout * verticalLay = new QVBoxLayout(m_groupBox);
	verticalLay->addWidget(m_editWidget);
	m_groupBox->setLayout(verticalLay);
	verticalLay->setMargin(0);
	m_editWidget->setup(&SVSettings::instance().m_db, m_dbModel);

	// specific setup for DB table view
	m_ui->treeView->header()->setSectionResizeMode(SVDBZoneTemplateTreeModel::ColCheck, QHeaderView::Fixed);
	m_ui->treeView->header()->setSectionResizeMode(SVDBZoneTemplateTreeModel::ColColor, QHeaderView::Fixed);
	m_ui->treeView->setSortingEnabled(true);

	connect(m_editWidget, &SVDBZoneTemplateEditWidget::selectSubTemplate,
			this, &SVDBZoneTemplateEditDialog::onSelectSubTemplate);

	connect(m_ui->treeView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
			this, SLOT(onCurrentIndexChanged(const QModelIndex &, const QModelIndex &)) );

	// set item delegate for coloring built-ins
	SVDBModelDelegate * dg = new SVDBModelDelegate(this, Role_BuiltIn, Role_Local, Role_Referenced);
	m_ui->treeView->setItemDelegate(dg);

	resize(1400,600);
}


SVDBZoneTemplateEditDialog::~SVDBZoneTemplateEditDialog() {
	delete m_ui;
}


void SVDBZoneTemplateEditDialog::edit(unsigned int initialId) {

	// hide select/cancel buttons, and show "close" button
	m_ui->pushButtonClose->setVisible(true);
	m_ui->pushButtonSelect->setVisible(false);
	m_ui->pushButtonCancel->setVisible(false);
	m_ui->pushButtonRemoveUnusedElements->setEnabled(SVProjectHandler::instance().isValid());

	// update "isRferenced" property of all elements
	if (SVProjectHandler::instance().isValid()){
		SVSettings::instance().m_db.updateReferencedElements(project());
	}

	// ask database model to update its content
	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)
	selectItemById(initialId);
	onCurrentIndexChanged(m_ui->treeView->currentIndex(), QModelIndex()); // select nothing

	//m_ui->treeView->expandAll();
	m_ui->treeView->resizeColumnToContents(0);
	m_ui->treeView->resizeColumnToContents(1);
	m_ui->treeView->resizeColumnToContents(2);
	m_ui->treeView->resizeColumnToContents(3);

	exec();
	QTimer::singleShot(0, &SVViewStateHandler::instance(), &SVViewStateHandler::refreshColors);
}


unsigned int SVDBZoneTemplateEditDialog::select(unsigned int initialId) {

	m_ui->pushButtonClose->setVisible(false);
	m_ui->pushButtonSelect->setVisible(true);
	m_ui->pushButtonCancel->setVisible(true);
	m_ui->pushButtonRemoveUnusedElements->setEnabled(SVProjectHandler::instance().isValid());

	// update "isRferenced" property of all elements
	if (SVProjectHandler::instance().isValid()){
		SVSettings::instance().m_db.updateReferencedElements(project());
	}

	m_dbModel->resetModel(); // ensure we use up-to-date data (in case the database data has changed elsewhere)
	selectItemById(initialId);
	onCurrentIndexChanged(m_ui->treeView->currentIndex(), QModelIndex()); // select nothing

	//m_ui->treeView->expandAll();
	m_ui->treeView->resizeColumnToContents(0);
	m_ui->treeView->resizeColumnToContents(1);
	m_ui->treeView->resizeColumnToContents(2);

	int res = exec();
	QTimer::singleShot(0, &SVViewStateHandler::instance(), &SVViewStateHandler::refreshColors);
	if (res == QDialog::Accepted) {
		// determine current item
		QModelIndex currentProxyIndex = m_ui->treeView->currentIndex();
		Q_ASSERT(currentProxyIndex.isValid());
		QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);

		// if this is a child index, get the index of the parent
		if (sourceIndex.internalPointer() != nullptr)
			sourceIndex = sourceIndex.parent();

		// return ID
		return sourceIndex.data(Role_Id).toUInt();
	}

	// nothing selected/dialog aborted
	return VICUS::INVALID_ID;
}


void SVDBZoneTemplateEditDialog::on_pushButtonSelect_clicked() {
	accept();
}


void SVDBZoneTemplateEditDialog::on_pushButtonCancel_clicked() {
	reject();
}


void SVDBZoneTemplateEditDialog::on_pushButtonClose_clicked() {
	accept();
}


void SVDBZoneTemplateEditDialog::on_toolButtonAdd_clicked() {
	// add new item
	QModelIndex sourceIndex = m_dbModel->addNewItem();
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->treeView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
	// resize ID column
	sourceIndex = m_dbModel->index(0, SVDBZoneTemplateTreeModel::ColId, QModelIndex());
	proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	if (proxyIndex.isValid())
		m_ui->treeView->resizeColumnToContents(proxyIndex.column());
}


void SVDBZoneTemplateEditDialog::on_toolButtonCopy_clicked() {
	// determine current item
	QModelIndex currentProxyIndex = m_ui->treeView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	// if this is a child index, get the index of the parent
	if (sourceIndex.internalPointer() != nullptr)
		sourceIndex = sourceIndex.parent();
	sourceIndex = m_dbModel->copyItem(sourceIndex);
	QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
	m_ui->treeView->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
}


void SVDBZoneTemplateEditDialog::on_toolButtonRemove_clicked() {
	QModelIndex currentProxyIndex = m_ui->treeView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	// if this is a child index, get the index of the parent
	if (sourceIndex.internalPointer() != nullptr) {
		m_editWidget->on_toolButtonRemoveSubComponent_clicked();
		return;
	}
	m_dbModel->deleteItem(sourceIndex);
	// last construction removed? clear input widget
	if (m_dbModel->rowCount() == 0)
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
}


void SVDBZoneTemplateEditDialog::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex &/*previous*/) {
	// if there is no selection, deactivate all buttons that need a selection
	if (!current.isValid()) {
		m_ui->pushButtonSelect->setEnabled(false);
		m_ui->toolButtonRemove->setEnabled(false);
		m_ui->toolButtonCopy->setEnabled(false);
		m_ui->toolButtonStoreInUserDB->setEnabled(false);
		m_ui->toolButtonRemoveFromUserDB->setEnabled(false);
		m_groupBox->setEnabled(false);
		m_editWidget->updateInput(-1, -1, VICUS::ZoneTemplate::NUM_ST); // nothing selected
	}
	else {
		m_groupBox->setEnabled(true);
		m_ui->pushButtonSelect->setEnabled(true);
		// remove is not allowed for built-ins
		QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
		bool builtIn = sourceIndex.data(Role_BuiltIn).toBool();
		bool isZoneTemplate = sourceIndex.internalPointer() == nullptr;
		m_ui->toolButtonRemove->setEnabled(!builtIn && isZoneTemplate);

		// only elements which are local and not built-in can be stored to user DB
		bool local = sourceIndex.data(Role_Local).toBool();
		m_ui->toolButtonStoreInUserDB->setEnabled(local && !builtIn && isZoneTemplate);
		m_ui->toolButtonRemoveFromUserDB->setEnabled(!local && !builtIn && isZoneTemplate);

		m_ui->toolButtonCopy->setEnabled(isZoneTemplate);
		m_ui->treeView->setCurrentIndex(current);
		// retrieve current ID
		int id = sourceIndex.data(Role_Id).toInt();
		int subTemplateID = -1;
		VICUS::ZoneTemplate::SubTemplateType subTemplateType = VICUS::ZoneTemplate::NUM_ST;
		// sub-template selected?
		if (!isZoneTemplate) {
			subTemplateID = id;
			QModelIndex parentIndex = sourceIndex.parent();
			id = parentIndex.data(Role_Id).toInt();
			subTemplateType = (VICUS::ZoneTemplate::SubTemplateType)sourceIndex.data(Role_SubTemplateType).toInt();
		}
		m_editWidget->updateInput(id, subTemplateID, subTemplateType);
	}
}


void SVDBZoneTemplateEditDialog::on_pushButtonReloadUserDB_clicked() {
	if (QMessageBox::question(this, QString(), tr("Reloading the user database from file will revert all changes "
												  "made in this dialog since the program was started. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// tell db to drop all user-defined items and re-read the DB
		SVSettings::instance().m_db.m_zoneTemplates.removeUserElements();
		SVSettings::instance().m_db.readDatabases(SVDatabase::DT_ZoneTemplates);

		// update "isRferenced" property of all elements
		if (SVProjectHandler::instance().isValid()){
			SVSettings::instance().m_db.updateReferencedElements(project());
		}

		// tell model to reset completely
		m_dbModel->resetModel();
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
		m_ui->treeView->expandAll();
		m_editWidget->updateInput(-1, -1, VICUS::ZoneTemplate::NUM_ST);
	}
}


void SVDBZoneTemplateEditDialog::selectItemById(unsigned int id) {
	// select top-level item with given id
	for (int i=0, count = m_dbModel->rowCount(); i<count; ++i) {
		QModelIndex sourceIndex = m_dbModel->index(i,0, QModelIndex());
		if (m_dbModel->data(sourceIndex, Role_Id).toUInt() == id) {
			// get proxy index
			QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
			if (proxyIndex.isValid()) {
				m_ui->treeView->blockSignals(true);
				m_ui->treeView->setCurrentIndex(proxyIndex);
				m_ui->treeView->blockSignals(false);
			}
			break;
		}
	}
}


void SVDBZoneTemplateEditDialog::on_treeView_doubleClicked(const QModelIndex &index) {
	if (m_ui->pushButtonSelect->isVisible() && index.isValid()) {
		QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
		if (sourceIndex.internalPointer() == nullptr)
			accept();
	}
}


void SVDBZoneTemplateEditDialog::onSelectSubTemplate(unsigned int zoneTemplateID,
													 VICUS::ZoneTemplate::SubTemplateType subTemplateType) {
	// if subTemplateType is NUM_ST, select only the top item
	if (subTemplateType == VICUS::ZoneTemplate::NUM_ST) {
		selectItemById((unsigned int)zoneTemplateID);
		m_editWidget->updateInput((int)zoneTemplateID, -1, subTemplateType);
	}
	else {
		// first find the zone template index, then search its children for matching subTemplateType
		for (int i=0, count = m_dbModel->rowCount(); i<count; ++i) {
			QModelIndex sourceIndex = m_dbModel->index(i,0, QModelIndex());
			if (sourceIndex.data(Role_Id).toUInt() == zoneTemplateID) {
				// now loop over all children and pick the one with the correct subTemplateType
				for (int j=0, count = m_dbModel->rowCount(sourceIndex); j<count; ++j) {
					QModelIndex subTemplateSourceIndex = m_dbModel->index(j,0,sourceIndex);
					if (subTemplateSourceIndex.data(Qt::UserRole + 20).toInt() == (int)subTemplateType) {
						QModelIndex proxyIndex = m_proxyModel->mapFromSource(subTemplateSourceIndex);
						if (proxyIndex.isValid()) {
							m_ui->treeView->blockSignals(true);
							m_ui->treeView->setCurrentIndex(proxyIndex);
							m_ui->treeView->blockSignals(false);
						}
						m_editWidget->updateInput((int)zoneTemplateID, subTemplateSourceIndex.data(Role_Id).toInt(), subTemplateType);
						return;
					}
				}
			}
		}
	}
}

void SVDBZoneTemplateEditDialog::on_treeView_expanded(const QModelIndex &index) {
	m_ui->treeView->resizeColumnToContents(0);
	m_ui->treeView->resizeColumnToContents(1);
	// we add a bit more width since automatic resizing not fully working
	m_ui->treeView->setColumnWidth(1, m_ui->treeView->columnWidth(1) + 100);
}


void SVDBZoneTemplateEditDialog::on_treeView_collapsed(const QModelIndex &index) {
	m_ui->treeView->resizeColumnToContents(0);
	m_ui->treeView->resizeColumnToContents(1);
}


void SVDBZoneTemplateEditDialog::on_toolButtonStoreInUserDB_clicked() {
	QModelIndex currentProxyIndex = m_ui->treeView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->setItemLocal(sourceIndex, false);
	onCurrentIndexChanged(m_ui->treeView->currentIndex(), QModelIndex());

	// find local children
	unsigned int id = sourceIndex.data(Role_Id).toUInt();
	std::set<VICUS::AbstractDBElement *> localChildren;
	SVSettings::instance().m_db.findLocalChildren(SVDatabase::DT_ZoneTemplates, id, localChildren);

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


void SVDBZoneTemplateEditDialog::on_toolButtonRemoveFromUserDB_clicked() {
	QModelIndex currentProxyIndex = m_ui->treeView->currentIndex();
	Q_ASSERT(currentProxyIndex.isValid());
	QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxyIndex);
	m_dbModel->setItemLocal(sourceIndex, true);
	onCurrentIndexChanged(m_ui->treeView->currentIndex(), QModelIndex());
}


void SVDBZoneTemplateEditDialog::on_pushButtonRemoveUnusedElements_clicked() {
	if (QMessageBox::question(this, QString(), tr("All elements that are currently not used in the project will be deleted. Continue?"),
							  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
		// tell db to drop all user-defined items and re-read the DB
		if (SVProjectHandler::instance().isValid())
			SVSettings::instance().m_db.removeNotReferencedLocalElements(SVDatabase::DT_ZoneTemplates, project());
		// tell model to reset completely
		m_dbModel->resetModel();
		onCurrentIndexChanged(QModelIndex(), QModelIndex());
	}
}

