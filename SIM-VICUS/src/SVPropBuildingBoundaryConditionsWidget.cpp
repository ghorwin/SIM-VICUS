#include "SVPropBuildingBoundaryConditionsWidget.h"
#include "ui_SVPropBuildingBoundaryConditionsWidget.h"

#include <SV_Conversions.h>

#include "SVStyle.h"
#include "SVProjectHandler.h"
#include "SVDatabase.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVUndoTreeNodeState.h"

SVPropBuildingBoundaryConditionsWidget::SVPropBuildingBoundaryConditionsWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingBoundaryConditionsWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->tableWidgetBoundaryConditions->setColumnCount(2);
	m_ui->tableWidgetBoundaryConditions->setHorizontalHeaderLabels(QStringList() << QString() << tr("Boundary condition"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetBoundaryConditions);
	m_ui->tableWidgetBoundaryConditions->setSortingEnabled(false);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->setStretchLastSection(true);
}


SVPropBuildingBoundaryConditionsWidget::~SVPropBuildingBoundaryConditionsWidget() {
	delete m_ui;
}


void SVPropBuildingBoundaryConditionsWidget::updateUi() {
	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true); // visible (selected and not selected)

	const SVDatabase & db = SVSettings::instance().m_db;

	// now build a map of referenced boundary conditions versus visible surfaces
	m_bcSurfacesMap.clear();
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// component ID assigned?
		if (ci.m_idComponent == VICUS::INVALID_ID)
			continue; // no component, skip
		// lookup component in DB
		const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_idComponent];
		// invalid component ID?
		if (comp == nullptr)
			continue; // no component, skip
		const VICUS::BoundaryCondition * bcSideA = nullptr;
		const VICUS::BoundaryCondition * bcSideB = nullptr;
		// lookup boundary condition pointers
		if (comp->m_idSideABoundaryCondition != VICUS::INVALID_ID)
			bcSideA = db.m_boundaryConditions[comp->m_idSideABoundaryCondition];
		if (comp->m_idSideBBoundaryCondition != VICUS::INVALID_ID)
			bcSideB = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition];

		// side A
		if (ci.m_sideASurface != nullptr) {
			// is this surface visible?
			std::set<const VICUS::Object * >::const_iterator it_A = objs.find(ci.m_sideASurface);
			if (it_A != objs.end()) {
				m_bcSurfacesMap[bcSideA].insert(ci.m_sideASurface);
			}
		}
		// side B
		if (ci.m_sideBSurface != nullptr) {
			std::set<const VICUS::Object * >::const_iterator it_B = objs.find(ci.m_sideBSurface);
			if (it_B != objs.end())
				m_bcSurfacesMap[bcSideB].insert(ci.m_sideBSurface);
		}
	}
	// now put the data of the map into the table
	int currentRow = m_ui->tableWidgetBoundaryConditions->currentRow();
	m_ui->tableWidgetBoundaryConditions->blockSignals(true);
	m_ui->tableWidgetBoundaryConditions->clearContents();
	m_ui->tableWidgetBoundaryConditions->setRowCount(m_bcSurfacesMap.size());
	int row=0;
	for (std::map<const VICUS::BoundaryCondition*, std::set<const VICUS::Surface *> >::const_iterator
		 it = m_bcSurfacesMap.begin(); it != m_bcSurfacesMap.end(); ++it, ++row)
	{
		QTableWidgetItem * item = new QTableWidgetItem();
		// special handling for surfaces without bc assigned
		if (it->first == nullptr) {
			item->setBackground(QColor(64,64,64)); // gray = invalid
			item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
			m_ui->tableWidgetBoundaryConditions->setItem(row, 0, item);

			item = new QTableWidgetItem();
			item->setText(tr("Invalid/missing boundary condition"));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetBoundaryConditions->setItem(row, 1, item);
		}
		else {
			item->setBackground(it->first->m_color);
			item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
			m_ui->tableWidgetBoundaryConditions->setItem(row, 0, item);

			item = new QTableWidgetItem();
			item->setText(QtExt::MultiLangString2QString(it->first->m_displayName) );
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetBoundaryConditions->setItem(row, 1, item);
		}
	}
	// reselect row
	m_ui->tableWidgetBoundaryConditions->blockSignals(false);
	m_ui->tableWidgetBoundaryConditions->selectRow(std::min(currentRow, m_ui->tableWidgetBoundaryConditions->rowCount()-1));

	// enable/disable edit/select button based on current scene selection and selection in table view
	on_tableWidgetBoundaryConditions_itemSelectionChanged();
}


void SVPropBuildingBoundaryConditionsWidget::on_pushButtonEditBoundaryConditions_clicked() {
	int currentRow = m_ui->tableWidgetBoundaryConditions->currentRow();
	std::map<const VICUS::BoundaryCondition*, std::set<const VICUS::Surface *> >::const_iterator it = m_bcSurfacesMap.begin();
	std::advance(it, currentRow);
	const VICUS::BoundaryCondition* bc = it->first;
	// start DB editor for selected boundary condition
	SVMainWindow::instance().dbBoundaryConditionEditDialog()->edit(bc->m_id);
	// Note: project data isn't modified, since only user DB data was changed.
	// reselect row, because closing the edit dialog recreates the table (due to possible color change)
	if (m_ui->tableWidgetBoundaryConditions->rowCount() > currentRow)
		m_ui->tableWidgetBoundaryConditions->selectRow(currentRow);
}


void SVPropBuildingBoundaryConditionsWidget::on_tableWidgetBoundaryConditions_itemSelectionChanged() {
	bool enabled = m_ui->tableWidgetBoundaryConditions->currentRow() != -1;
	if (enabled) {
		// if selected item is an invalid/missing BC, disable buttons
		std::map<const VICUS::BoundaryCondition*, std::set<const VICUS::Surface *> >::const_iterator it = m_bcSurfacesMap.begin();
		std::advance(it, m_ui->tableWidgetBoundaryConditions->currentRow());
		if (it->first == nullptr)
			enabled = false;
	}
	m_ui->pushButtonEditBoundaryConditions->setEnabled(enabled);
	m_ui->pushButtonSelectBoundaryConditions->setEnabled(enabled);
}


void SVPropBuildingBoundaryConditionsWidget::on_pushButtonSelectBoundaryConditions_clicked() {
	int r = m_ui->tableWidgetBoundaryConditions->currentRow();
	Q_ASSERT(r != -1);
	std::map<const VICUS::BoundaryCondition*, std::set<const VICUS::Surface *> >::const_iterator cit = m_bcSurfacesMap.begin();
	std::advance(cit, r);
	const VICUS::BoundaryCondition * bc = cit->first;
	Q_ASSERT(m_bcSurfacesMap.find(bc) != m_bcSurfacesMap.end());

	std::set<unsigned int> objs;
	for (const VICUS::Surface * s : cit->second)
		objs.insert(s->m_id);

	QString undoText;
	if (bc != nullptr)
		undoText = tr("Select objects with boundary condition '%1'").arg(QtExt::MultiLangString2QString(bc->m_displayName));
	else
		undoText = tr("Select objects with invalid boundary condition");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, objs, true);
	undo->push();
}
