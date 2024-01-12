#include "SVPropBuildingSurfaceConnectionWidget.h"
#include "ui_SVPropBuildingSurfaceConnectionWidget.h"

#include <SVConversions.h>

#include <QComboBox>
#include <QItemSelectionRange>

#include <VICUS_utilities.h>

#include "SVStyle.h"
#include "SVUndoTreeNodeState.h"
#include "SVUndoModifyComponentInstances.h"
#include "SVUndoModifyBuildingTopology.h"
#include "SVProjectHandler.h"
#include "SVSmartIntersectionDialog.h"
#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "Vic3DSceneView.h"


SVPropBuildingSurfaceConnectionWidget::SVPropBuildingSurfaceConnectionWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingSurfaceConnectionWidget)
{
	m_ui->setupUi(this);
	m_ui->mainGridLayout->setMargin(0);

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

	m_ui->comboBoxHighlightingMode->addItem(tr("Transparent surfaces"), Vic3D::Scene::HM_TransparentWithBoxes);
	m_ui->comboBoxHighlightingMode->addItem(tr("Colored surfaces"), Vic3D::Scene::HM_ColoredSurfaces);

	const Vic3D::SceneView *sc = SVViewStateHandler::instance().m_geometryView->sceneView();
	connect(this, &SVPropBuildingSurfaceConnectionWidget::updatedHighlightingMode, sc, &Vic3D::SceneView::onTransparentBuildingModeChanged);

	// set default mode
	m_ui->comboBoxHighlightingMode->setCurrentIndex(Vic3D::Scene::HM_TransparentWithBoxes);
	emit updatedHighlightingMode(Vic3D::Scene::HM_TransparentWithBoxes);
}


SVPropBuildingSurfaceConnectionWidget::~SVPropBuildingSurfaceConnectionWidget() {
	delete m_ui;
}


void SVPropBuildingSurfaceConnectionWidget::updateUi(bool onlySelectionModified) {
	if (onlySelectionModified && !m_ui->tableWidgetInterlinkedSurfaces->isVisible())
		return;

	// get all visible "building" type objects in the scene
	std::vector<const VICUS::Surface *> selectedSurfaces; // note: we use a set, because we want to quickly remove items later
	project().selectedSurfaces(selectedSurfaces, VICUS::Project::SG_Building); // visible and selected surfaces

	// update set with selected surfaces
	m_selectedSurfaces.clear();
	for(const VICUS::Surface *surf : selectedSurfaces)
		m_selectedSurfaces[surf->m_id] = surf;

	m_ui->tableWidgetInterlinkedSurfaces->blockSignals(true);
	m_ui->tableWidgetInterlinkedSurfaces->selectionModel()->blockSignals(true);

	const SVDatabase & db = SVSettings::instance().m_db;

	// process all component instances
	std::set<int> selectedRows;
	std::set<const VICUS::Surface*> referencedSurfaces;
	std::set<unsigned int>	handledIds;

	if(onlySelectionModified) {
		for(unsigned int rowIdx = 0; rowIdx < (unsigned int)m_ui->tableWidgetInterlinkedSurfaces->rowCount(); ++rowIdx) {
			QTableWidgetItem *item0 = m_ui->tableWidgetInterlinkedSurfaces->item(rowIdx, 2); // Surf A
			QTableWidgetItem *item1 = m_ui->tableWidgetInterlinkedSurfaces->item(rowIdx, 3); // Surf B

			unsigned int idSurfA = item0->data(Qt::UserRole).toUInt();
			unsigned int idSurfB = item1->data(Qt::UserRole).toUInt();

			if (m_selectedSurfaces.find(idSurfA) != m_selectedSurfaces.end() ||
					m_selectedSurfaces.find(idSurfB) != m_selectedSurfaces.end() )
					selectedRows.insert(rowIdx);
		}
	}
	else {
		m_ui->tableWidgetInterlinkedSurfaces->setRowCount(0);
		for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
			unsigned int counter = 1;
			for (const VICUS::ComponentInstance & ciTest : project().m_componentInstances) {
				if(handledIds.find(ci.m_id) != handledIds.end())
					break; // skip already handled ci ids with doublings
				// Not very smart comparission of component instance
				// TODO add == operator

				if(ciTest.m_idSideASurface == ci.m_idSideASurface &&
						ci.m_idSideBSurface == ciTest.m_idSideBSurface)
					continue;

				// Give user feedback, when several component instances exist.
				if(ci.m_id == ciTest.m_id &&
						(ciTest.m_idSideASurface != ci.m_idSideASurface ||
						 ci.m_idSideBSurface != ciTest.m_idSideBSurface)) {
					++counter;
				}
			}

			if(counter > 1)
				IBK::IBK_Message(IBK::FormatString("Unfortunately the project contains %2 component instances with the same id #%1 "
												   "but different connected surfaces.").arg(ci.m_id).arg(counter), IBK::MSG_ERROR);

			handledIds.insert(ci.m_id);

			// skip all without two surfaces
			if (ci.m_idSideASurface == VICUS::INVALID_ID || ci.m_idSideBSurface == VICUS::INVALID_ID)
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
			if (m_selectedSurfaces.find(ci.m_idSideASurface) != m_selectedSurfaces.end())
				selectedRows.insert(row);
			m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 2, item);

			// column 3 - surface name B

			item = new QTableWidgetItem(ci.m_sideBSurface->m_displayName);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(Qt::UserRole, ci.m_sideBSurface->m_id); // uniqueID is the user role
			if (m_selectedSurfaces.find(ci.m_idSideBSurface) != m_selectedSurfaces.end())
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
	}


	m_ui->tableWidgetInterlinkedSurfaces->selectionModel()->blockSignals(true);


	// once table is complete (and won't be modified anylonger), select items
	if (selectedRows.empty())
		m_ui->tableWidgetInterlinkedSurfaces->clearSelection();
	else {
		QItemSelectionModel *selection = m_ui->tableWidgetInterlinkedSurfaces->selectionModel();
		QItemSelection itemSelection;

		for (int r : selectedRows) {
			// Create a selection for the first two rows
			QModelIndex topLeft = m_ui->tableWidgetInterlinkedSurfaces->model()->index(r, 0);
			QModelIndex bottomRight = m_ui->tableWidgetInterlinkedSurfaces->model()->index(r, 4);
			QItemSelection selection(topLeft, bottomRight);

			itemSelection.select(topLeft, bottomRight);
		}

		selection->select(itemSelection, QItemSelectionModel::Select);

		int row = *selectedRows.begin();
		// m_ui->tableWidgetInterlinkedSurfaces->setCurrentCell(row, 0);
		QTableWidgetItem *item = m_ui->tableWidgetInterlinkedSurfaces->item(row, 0);
		m_ui->tableWidgetInterlinkedSurfaces->scrollToItem(item);
	}


	SVStyle::resizeTableColumnToContents(m_ui->tableWidgetInterlinkedSurfaces, 0, true);
	SVStyle::resizeTableColumnToContents(m_ui->tableWidgetInterlinkedSurfaces, 1, true);
	SVStyle::resizeTableColumnToContents(m_ui->tableWidgetInterlinkedSurfaces, 2, true);

	m_ui->tableWidgetInterlinkedSurfaces->selectionModel()->blockSignals(false);
	m_ui->tableWidgetInterlinkedSurfaces->blockSignals(false);

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
			ci1.m_sideBSurface = nullptr;
			VICUS::ComponentInstance ci2(ci);
			ci2.m_id = newID++;
			ci2.m_idSideASurface = VICUS::INVALID_ID;
			ci2.m_sideASurface = nullptr;
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
		const std::vector<VICUS::SubSurfaceComponentInstance> *subCis = nullptr;
		if(!m_smartClippingDialog->componentInstances().empty())
			cis = &m_smartClippingDialog->componentInstances();
		if(!m_smartClippingDialog->subSurfaceComponentInstances().empty())
			subCis = &m_smartClippingDialog->subSurfaceComponentInstances();
		SVUndoModifyBuildingTopology *undo = new SVUndoModifyBuildingTopology("Smart-clipping applied to project.", m_smartClippingDialog->buildings(), cis, subCis);
		undo->push();
	}
	else if (res == SVSmartIntersectionDialog::CancelledClipping) {
		return;
	}
}


void SVPropBuildingSurfaceConnectionWidget::on_comboBoxHighlightingMode_currentIndexChanged(int /*index*/) {
	Vic3D::Scene::HighlightingMode m = static_cast<Vic3D::Scene::HighlightingMode>(m_ui->comboBoxHighlightingMode->currentData(Qt::UserRole).toInt());
	emit updatedHighlightingMode(m);
}

