#include "SVPropSupplySystemsWidget.h"

#include <SV_Conversions.h>

#include "SVStyle.h"
#include "SVProjectHandler.h"
#include "SVDatabase.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVUndoTreeNodeState.h"

SVPropSupplySystemsWidget::SVPropSupplySystemsWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropSupplySystemsWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->tableWidgetSupplySystems->setColumnCount(2);
	m_ui->tableWidgetSupplySystems->setHorizontalHeaderLabels(QStringList() << QString() << tr("Supply systems"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSupplySystems);
	m_ui->tableWidgetSupplySystems->setSortingEnabled(false);
	m_ui->tableWidgetSupplySystems->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetSupplySystems->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetSupplySystems->horizontalHeader()->setStretchLastSection(true);
}


SVPropSupplySystemsWidget::~SVPropSupplySystemsWidget() {
	delete m_ui;
}


void SVPropSupplySystemsWidget::updateUi() {
	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true); // visible (selected and not selected)

	const SVDatabase & db = SVSettings::instance().m_db;

	// now build a map of referenced boundary conditions versus visible surfaces
	m_supplySysSurfacesMap.clear();
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// supply system ID assigned?
		if (ci.m_idSupplySystem == VICUS::INVALID_ID)
			continue; // no supply system, skip
		// lookup supply system in DB
		const VICUS::SupplySystem * supplySystem = db.m_supplySystems[ci.m_idSupplySystem];
		// invalid heating ID?
		if (supplySystem == nullptr)
			continue; // no heating, skip

		// side A
		if (ci.m_sideASurface != nullptr) {
			// is this surface visible?
			std::set<const VICUS::Object * >::const_iterator it_A = objs.find(ci.m_sideASurface);
			if (it_A != objs.end()) {
				m_supplySysSurfacesMap[supplySystem].insert(ci.m_sideASurface);
			}
		}
		// side B
		if (ci.m_sideBSurface != nullptr) {
			std::set<const VICUS::Object * >::const_iterator it_B = objs.find(ci.m_sideBSurface);
			if (it_B != objs.end())
				m_supplySysSurfacesMap[supplySystem].insert(ci.m_sideBSurface);
		}
	}
	// now put the data of the map into the table
	int currentRow = m_ui->tableWidgetSupplySystems->currentRow();
	m_ui->tableWidgetSupplySystems->blockSignals(true);
	m_ui->tableWidgetSupplySystems->clearContents();
	m_ui->tableWidgetSupplySystems->setRowCount(m_supplySysSurfacesMap.size());
	int row=0;
	for (std::map<const VICUS::SupplySystem*, std::set<const VICUS::Surface *> >::const_iterator
		 it = m_supplySysSurfacesMap.begin(); it != m_supplySysSurfacesMap.end(); ++it, ++row)
	{
		QTableWidgetItem * item = new QTableWidgetItem();
		// special handling for surfaces without bc assigned
		if (it->first == nullptr) {
			item->setBackground(QColor(64,64,64)); // gray = invalid
			item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
			m_ui->tableWidgetSupplySystems->setItem(row, 0, item);

			item = new QTableWidgetItem();
			item->setText(tr("Invalid/missing boundary condition"));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetSupplySystems->setItem(row, 1, item);
		}
		else {
			item->setBackground(it->first->m_color);
			item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
			m_ui->tableWidgetSupplySystems->setItem(row, 0, item);

			item = new QTableWidgetItem();
			item->setText(QtExt::MultiLangString2QString(it->first->m_displayName) );
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetSupplySystems->setItem(row, 1, item);
		}
	}
	// reselect row
	m_ui->tableWidgetSupplySystems->blockSignals(false);
	m_ui->tableWidgetSupplySystems->selectRow(std::min(currentRow, m_ui->tableWidgetSupplySystems->rowCount()-1));

	on_tableWidgetSupplySystems_itemSelectionChanged();

	m_ui->pushButtonEditSupplySystem->setEnabled(m_ui->tableWidgetSupplySystems->currentRow() != -1);
	m_ui->pushButtonSelectSupplySystem->setEnabled(m_ui->tableWidgetSupplySystems->currentRow() != -1);
}


void SVPropSupplySystemsWidget::on_pushButtonEditSupplySystem_clicked() {
	int currentRow = m_ui->tableWidgetSupplySystems->currentRow();
	std::map<const VICUS::SupplySystem*, std::set<const VICUS::Surface *> >::const_iterator it = m_supplySysSurfacesMap.begin();
	std::advance(it, currentRow);
	const VICUS::SupplySystem* supplySys = it->first;
	// start DB editor for selected boundary condition
	SVMainWindow::instance().dbSupplySystemEditDialog()->edit(supplySys->m_id);
	// Note: project data isn't modified, since only user DB data was changed.
	// reselect row, because closing the edit dialog recreates the table (due to possible color change)
	if (m_ui->tableWidgetSupplySystems->rowCount() > currentRow)
		m_ui->tableWidgetSupplySystems->selectRow(currentRow);
}


void SVPropSupplySystemsWidget::on_tableWidgetSupplySystems_itemSelectionChanged() {
	bool enabled = m_ui->tableWidgetSupplySystems->currentRow() != -1;
	if (enabled) {
		// if selected item is an invalid/missing BC, disable buttons
		std::map<const VICUS::SupplySystem*, std::set<const VICUS::Surface *> >::const_iterator it = m_supplySysSurfacesMap.begin();
		std::advance(it, m_ui->tableWidgetSupplySystems->currentRow());
		if (it->first == nullptr)
			enabled = false;
	}


	m_ui->pushButtonEditSupplySystem->setEnabled(enabled);
	m_ui->pushButtonSelectSupplySystem->setEnabled(enabled);
}


void SVPropSupplySystemsWidget::on_pushButtonSelectSupplySystem_clicked() {
	int r = m_ui->tableWidgetSupplySystems->currentRow();
	Q_ASSERT(r != -1);
	std::map<const VICUS::SupplySystem*, std::set<const VICUS::Surface *> >::const_iterator sit = m_supplySysSurfacesMap.begin();
	std::advance(sit, r);
	const VICUS::SupplySystem * supplySys = sit->first;
	Q_ASSERT(m_supplySysSurfacesMap.find(supplySys) != m_supplySysSurfacesMap.end());

	std::set<unsigned int> objs;
	for (const VICUS::Surface * s : sit->second)
		objs.insert(s->m_id);

	QString undoText;
	if (supplySys != nullptr)
		undoText = tr("Select objects with supply system '%1'").arg(QtExt::MultiLangString2QString(supplySys->m_displayName));
	else
		undoText = tr("Select objects with invalid supply system");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, objs, true);
	undo->push();
}
