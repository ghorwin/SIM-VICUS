#include "SVPropBuildingSurfaceConnectionWidget.h"
#include "ui_SVPropBuildingSurfaceConnectionWidget.h"

#include <SV_Conversions.h>

#include <VICUS_utilities.h>

#include "SVStyle.h"
#include "SVUndoTreeNodeState.h"
#include "SVUndoModifyComponentInstances.h"
#include "SVUndoModifyBuildingTopology.h"
#include "SVProjectHandler.h"
#include "SVSmartIntersectionDialog.h"


SVPropBuildingSurfaceConnectionWidget::SVPropBuildingSurfaceConnectionWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingSurfaceConnectionWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayoutMain->setMargin(0);

	m_ui->tableWidgetInterlinkedSurfaces->setColumnCount(5);
	m_ui->tableWidgetInterlinkedSurfaces->setHorizontalHeaderLabels(QStringList() << tr("") << tr("CI id") << tr("Surface Side A") << tr("Surface Side B") << tr("Component") );
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetInterlinkedSurfaces);
	m_ui->tableWidgetInterlinkedSurfaces->setSortingEnabled(false);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->resizeSection(0,10);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->resizeSection(1,40);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->resizeSection(2,100);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->resizeSection(3,100);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->resizeSection(4,100);
//	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
//	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
//	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetInterlinkedSurfaces->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_ui->tableWidgetInterlinkedSurfaces->setSelectionBehavior(QAbstractItemView::SelectRows);


}


SVPropBuildingSurfaceConnectionWidget::~SVPropBuildingSurfaceConnectionWidget() {
	delete m_ui;
}


void SVPropBuildingSurfaceConnectionWidget::updateUi(bool /*onlySelectionModified*/) {
	// get all visible "building" type objects in the scene
	std::vector<const VICUS::Surface *> selectedSurfaces; // note: we use a set, because we want to quickly remove items later
	project().selectedSurfaces(selectedSurfaces, VICUS::Project::SG_Building); // visible and selected surfaces

	// update set with selected surfaces
	m_selectedSurfaces = std::set<const VICUS::Surface *>(selectedSurfaces.begin(), selectedSurfaces.end());

	m_ui->tableWidgetInterlinkedSurfaces->blockSignals(true);
	m_ui->tableWidgetInterlinkedSurfaces->selectionModel()->blockSignals(true);
	m_ui->tableWidgetInterlinkedSurfaces->setRowCount(0);

	const SVDatabase & db = SVSettings::instance().m_db;

	// process all component instances
	std::set<int> selectedRows;
	std::set<const VICUS::Surface*> referencedSurfaces;
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// skip all without two surfaces
		if (ci.m_sideASurface == nullptr || ci.m_sideBSurface == nullptr)
			continue;

		// add new row
		int row = m_ui->tableWidgetInterlinkedSurfaces->rowCount();
		m_ui->tableWidgetInterlinkedSurfaces->setRowCount(row + 1);

		// column 1 - ID of this component instance

		QTableWidgetItem * item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setData(Qt::UserRole, ci.m_id);
		item->setText(QString("%1").arg(ci.m_id));
		item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 1, item);

		// column 2 - surface name A

		item = new QTableWidgetItem(ci.m_sideASurface->m_displayName);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setData(Qt::UserRole, ci.m_sideASurface->m_id); // uniqueID is the user role
		if (m_selectedSurfaces.find(ci.m_sideASurface) != m_selectedSurfaces.end())
			selectedRows.insert(row);
		m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 2, item);

		// column 3 - surface name B

		item = new QTableWidgetItem(ci.m_sideBSurface->m_displayName);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setData(Qt::UserRole, ci.m_sideBSurface->m_id); // uniqueID is the user role
		if (m_selectedSurfaces.find(ci.m_sideBSurface) != m_selectedSurfaces.end())
			selectedRows.insert(row);
		m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 3, item);

		// column 4 - component
		const VICUS::Component * comp = db.m_components[ci.m_idComponent];
		QString compName;
		if (comp == nullptr)
			compName = "---";
		else
			compName = QtExt::MultiLangString2QString(comp->m_displayName);

		item = new QTableWidgetItem(compName);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 4, item);

		// we also have to check all components, so that we have not only
		// connected surfaces but also boundary conditions that fits to them

		// we already now that both sides are connected

		//QIcon(":/gfx/actions/16x16/error.png");
		item = new QTableWidgetItem();
		item->setIcon(QIcon(":/gfx/actions/16x16/ok.png"));

		// we collect all relevent information as text in the tooltip
		QString toolTip;

		// check that neither of the two surfaces was previously used in another component instance
		if (referencedSurfaces.find(ci.m_sideASurface) != referencedSurfaces.end()) {
			item->setIcon(QIcon(":/gfx/actions/16x16/error.png"));
			toolTip += "Component references surface at side A that was already previously referenced somewhere else.";
			m_ui->tableWidgetInterlinkedSurfaces->item(row, 2)->setForeground(Qt::red);
		}
		if (referencedSurfaces.find(ci.m_sideBSurface) != referencedSurfaces.end()) {
			item->setIcon(QIcon(":/gfx/actions/16x16/error.png"));
			toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
			toolTip += "Component references surface at side B that was already previously referenced somewhere else.";
			m_ui->tableWidgetInterlinkedSurfaces->item(row, 3)->setForeground(Qt::red);
		}
		// remember surfaces are "referenced"
		referencedSurfaces.insert(ci.m_sideASurface);
		referencedSurfaces.insert(ci.m_sideBSurface);

		// accces DB

		if (comp != nullptr) {
			const VICUS::BoundaryCondition * bcLeft = db.m_boundaryConditions[comp->m_idSideABoundaryCondition];
			if (bcLeft == nullptr) {
				item->setIcon(QIcon(":/gfx/actions/16x16/error.png"));
				toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
				toolTip += "Component has no valid boundary condition at surface side A.";

				m_ui->tableWidgetInterlinkedSurfaces->item(row, 2)->setForeground(Qt::red);
			}
			else {
				// check that bc does not reference constant zone
				if (bcLeft->m_heatConduction.m_otherZoneType != VICUS::InterfaceHeatConduction::OZ_Standard) {
					item->setIcon(QIcon(":/gfx/actions/16x16/error.png"));
					toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
					toolTip += "Boundary condition at surface side A is associated with constant/scheduled zone.";

					m_ui->tableWidgetInterlinkedSurfaces->item(row, 2)->setForeground(Qt::red);
				}
			}
		}
		else {
			item->setIcon(QIcon(":/gfx/actions/16x16/error.png"));
			toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
			toolTip += "Invalid/unassigned component";
			m_ui->tableWidgetInterlinkedSurfaces->item(row, 2)->setForeground(Qt::red);
		}


		if (comp != nullptr) {
			const VICUS::BoundaryCondition * bcRight = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition];
			if (bcRight == nullptr) {
				item->setIcon(QIcon(":/gfx/actions/16x16/error.png"));
				toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
				toolTip += "Component has no valid boundary condition at surface side B.";

				m_ui->tableWidgetInterlinkedSurfaces->item(row, 3)->setForeground(Qt::red);
			}
			else {
				// check that bc does not reference constant zone
				if (bcRight->m_heatConduction.m_otherZoneType != VICUS::InterfaceHeatConduction::OZ_Standard) {
					item->setIcon(QIcon(":/gfx/actions/16x16/error.png"));
					toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
					toolTip += "Boundary condition at surface side B is associated with constant/scheduled zone.";

					m_ui->tableWidgetInterlinkedSurfaces->item(row, 3)->setForeground(Qt::red);
				}
			}
		}

		// must not reference the same surface on both sides
		if (ci.m_sideASurface == ci.m_sideBSurface) {
			item->setIcon(QIcon(":/gfx/actions/16x16/error.png"));
			toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
			toolTip += "Same surface referenced on both sides.";
			m_ui->tableWidgetInterlinkedSurfaces->item(row, 2)->setForeground(Qt::red);
			m_ui->tableWidgetInterlinkedSurfaces->item(row, 3)->setForeground(Qt::red);
		}

		item->setToolTip(toolTip);
		m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 0, item);

	}
	m_ui->tableWidgetInterlinkedSurfaces->blockSignals(false);

	// once table is complete (and won't be modified anylonger), select items
	if (m_ui->tableWidgetInterlinkedSurfaces->isVisible()) {
		for (int r : selectedRows) {
			m_ui->tableWidgetInterlinkedSurfaces->selectRow(r);
		}
	}


	SVStyle::resizeTableColumnToContents(m_ui->tableWidgetInterlinkedSurfaces, 0, true);
	SVStyle::resizeTableColumnToContents(m_ui->tableWidgetInterlinkedSurfaces, 1, true);
	SVStyle::resizeTableColumnToContents(m_ui->tableWidgetInterlinkedSurfaces, 2, true);

	m_ui->tableWidgetInterlinkedSurfaces->selectionModel()->blockSignals(false);

	// enable/disable button based on available selections
	m_ui->pushButtonRemoveComponentInstance->setEnabled(!selectedRows.empty());
    //m_ui->groupBoxConnectSurfaces->setEnabled(!m_selectedSurfaces.empty());
}



void SVPropBuildingSurfaceConnectionWidget::on_tableWidgetInterlinkedSurfaces_itemSelectionChanged() {
	// get selected items, compose undo-action and fire
	// compose set of objects to be selected
	std::set<unsigned int> selectedObjs;

	for (QTableWidgetItem * item : m_ui->tableWidgetInterlinkedSurfaces->selectedItems()) {
		if (item->column() == 2 || item->column() == 3)
			selectedObjs.insert(item->data(Qt::UserRole).toUInt());
	}

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selected connected surfaces"),
														 SVUndoTreeNodeState::SelectedState, selectedObjs,
														 true, true /* exclusive */);
	undo->push();
}



void SVPropBuildingSurfaceConnectionWidget::on_pushButtonRemoveComponentInstance_clicked() {

	// create set with selected and connected surfaces
	std::set<unsigned int> connectedSurfacesIDs;
	for (QTableWidgetItem * item : m_ui->tableWidgetInterlinkedSurfaces->selectedItems())
		connectedSurfacesIDs.insert(item->data(Qt::UserRole).toUInt());

	// now process all ComponentInstances and handle those with selected and connected surfaces
	std::vector<VICUS::ComponentInstance> newInstances;
	unsigned int newID = VICUS::largestUniqueId(project().m_componentInstances);
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// must have both sides connected
		if (ci.m_sideASurface == nullptr || ci.m_sideBSurface == nullptr) {
			newInstances.push_back(ci); // just keep unmodified
			continue;
		}

		if ( (ci.m_sideASurface != nullptr &&
				connectedSurfacesIDs.find(ci.m_sideASurface->m_id) != connectedSurfacesIDs.end()) ||
			(ci.m_sideBSurface != nullptr &&
							connectedSurfacesIDs.find(ci.m_sideBSurface->m_id) != connectedSurfacesIDs.end()) )
		{
			// create two copies of the ComponentInstance and remove sideA and sideB in either one
			VICUS::ComponentInstance ci1(ci);
			ci1.m_id = newID++;
			ci1.m_idSideBSurface = VICUS::INVALID_ID;
			VICUS::ComponentInstance ci2(ci);
			ci2.m_id = newID++;
			ci2.m_idSideASurface = VICUS::INVALID_ID;
			newInstances.push_back(ci1);
			newInstances.push_back(ci2);
		}
		else {
			// not currently selected
			newInstances.push_back(ci); // just keep unmodified
			continue;
		}
	}

	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Removed surface-surface connection"),
																			   newInstances);
	undo->push();
}

void SVPropBuildingSurfaceConnectionWidget::on_pushButtonSmartClipping_clicked() {

    if(m_smartClippingDialog == nullptr)
        m_smartClippingDialog = new SVSmartIntersectionDialog(this);

    SVSmartIntersectionDialog::ClippingResults res = m_smartClippingDialog->clipProject();
    if(res == SVSmartIntersectionDialog::AcceptClipping) {
		const std::vector<VICUS::ComponentInstance> *cis = nullptr;
		if(!m_smartClippingDialog->componentInstances().empty())
			cis = &m_smartClippingDialog->componentInstances();
		SVUndoModifyBuildingTopology *undo = new SVUndoModifyBuildingTopology("Smart-clipping applied to project.", m_smartClippingDialog->buildings(), cis);
        undo->push();
    }
    else if (res == SVSmartIntersectionDialog::CancelledClipping) {
        return;
    }
}

