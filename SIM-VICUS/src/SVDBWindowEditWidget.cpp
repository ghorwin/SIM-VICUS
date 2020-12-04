#include "SVDBWindowEditWidget.h"
#include "ui_SVDBWindowEditWidget.h"

#include <QSortFilterProxyModel>

#include "SVSettings.h"

SVDBWindowEditWidget::SVDBWindowEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBWindowEditWidget)
{
	m_ui->setupUi(this);

	// style the table widget

	QStringList captions;
	captions << tr("Name") << tr("Category"); // TODO other captions

	// create sort/filter proxy model
	m_sortFilterModel = new QSortFilterProxyModel(this);
	m_ui->tableWidget->setSortingEnabled(true);
	m_ui->splitter->setCollapsible(0,false);
}


SVDBWindowEditWidget::~SVDBWindowEditWidget() {
	delete m_ui;
}


void SVDBWindowEditWidget::edit() {
	updateUi();
	on_tableWidget_itemSelectionChanged();
	show();
}


void SVDBWindowEditWidget::on_toolButtonAdd_clicked() {
	// add a new window definition
	const VICUS::Window * w = SVSettings::instance().addDatabaseElement(SVSettings::instance().m_dbWindows);
	unsigned int newID = w->m_id;
	// update table

	updateUi();

	QTableWidgetItem * newItem = nullptr;
	do {
		// and select it (mind the sorting of the table!)
		for (int i=0; i<m_ui->tableWidget->rowCount(); ++i) {
			QTableWidgetItem * it = m_ui->tableWidget->item(i,0);
			if (it->data(Qt::UserRole).toUInt() == newID)
				newItem = it;
		}
		if (newItem == nullptr) {
			// item is now shown? Remove filter and try again
			// TODO : Andreas, add filter
		}
	} while (newItem == nullptr);

	// now select row of the item
	m_ui->tableWidget->selectRow(newItem->row());
}


void SVDBWindowEditWidget::on_toolButtonCopy_clicked() {

}


void SVDBWindowEditWidget::on_toolButtonRemove_clicked() {

}


void SVDBWindowEditWidget::updateUi() {
	m_ui->tableWidget->clearContents();
	m_ui->tableWidget->setRowCount(SVSettings::instance().m_dbWindows.size());

	// add all database elements
	int row = 0;
	for (const auto & d : SVSettings::instance().m_dbWindows) {
		QTableWidgetItem * item = new QTableWidgetItem( d.second.m_displayName );
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		// store ID
		item->setData(Qt::UserRole, d.first);
		// store built-in role
		item->setData(Qt::UserRole+5, d.second.m_builtIn);
		m_ui->tableWidget->setItem(row, 0, item);
		++row;
	}
}


void SVDBWindowEditWidget::updateEditWidget(unsigned int id) {
	if (id == VICUS::INVALID_ID) {
		// TODO : hide edit widget, or place all widets
		m_ui->groupBoxEditProperties->hide();
		return;
	}
	else {
		m_ui->groupBoxEditProperties->show();

		// get selected database element

	}
}


void SVDBWindowEditWidget::on_tableWidget_itemSelectionChanged() {
	// enable/disable buttons based on selection state
	if (m_ui->tableWidget->selectedItems().isEmpty()) {
		m_ui->toolButtonCopy->setEnabled(false);
		m_ui->toolButtonRemove->setEnabled(false);
	}
	else {
		// copying is always possible
		m_ui->toolButtonCopy->setEnabled(true);
		// removing not allowed for built-in database elements
		QTableWidgetItem * item = m_ui->tableWidget->selectedItems().first();
		if (item->data(Qt::UserRole+5).toBool())
			m_ui->toolButtonRemove->setEnabled(false); // built-ins cannot be removed
		else
			m_ui->toolButtonRemove->setEnabled(true);

		// update edit widget
		unsigned int itemID = item->data(Qt::UserRole).toUInt();
		updateEditWidget(itemID);
	}
}
