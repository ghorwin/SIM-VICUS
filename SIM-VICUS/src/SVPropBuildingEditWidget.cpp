#include "SVPropBuildingEditWidget.h"
#include "ui_SVPropBuildingEditWidget.h"

#include <VICUS_Component.h>
#include <VICUS_Conversions.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVConstants.h"
#include "SVStyle.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVDBComponentEditDialog.h"

#include "SVUndoTreeNodeState.h"
#include "SVUndoModifyComponentInstances.h"

SVPropBuildingEditWidget::SVPropBuildingEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);
	m_ui->verticalLayoutComponents->setMargin(0);
	m_ui->verticalLayoutComponentOrientation->setMargin(0);
	m_ui->verticalLayoutBoundaryConditions->setMargin(0);

	// configure tables
	m_ui->tableWidgetComponents->setColumnCount(2);
	m_ui->tableWidgetComponents->setHorizontalHeaderLabels(QStringList() << QString() << tr("Component"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetComponents);
	m_ui->tableWidgetComponents->setSortingEnabled(false);
	m_ui->tableWidgetComponents->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetComponents->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetComponents->horizontalHeader()->setStretchLastSection(true);

	m_ui->tableWidgetBoundaryConditions->setColumnCount(2);
	m_ui->tableWidgetBoundaryConditions->setHorizontalHeaderLabels(QStringList() << QString() << tr("Boundary condition"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetBoundaryConditions);
	m_ui->tableWidgetBoundaryConditions->setSortingEnabled(false);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->setStretchLastSection(true);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropBuildingEditWidget::onModified);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified, nullptr);
}


SVPropBuildingEditWidget::~SVPropBuildingEditWidget() {
	delete m_ui;
}


void SVPropBuildingEditWidget::setPropertyType(int buildingPropertyType) {
	m_ui->stackedWidget->setCurrentIndex(0);
	const SVDatabase & db = SVSettings::instance().m_db;
	switch ((BuildingPropertyTypes)buildingPropertyType) {
		case BT_Components: {
			m_ui->stackedWidget->setCurrentIndex(1);
			// get all visible "building" type objects in the scene
			std::set<const VICUS::Object * > objs;
			project().selectObjects(objs, VICUS::Project::SG_Building, false, true);
			// now build a map of component IDs versus visible surfaces
			m_componentSurfacesMap.clear();
			for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
				// component ID assigned?
				if (ci.m_componentID == VICUS::INVALID_ID)
					continue; // no component, skip
				// lookup component in DB
				const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_componentID];
				if (comp == nullptr) {
					// invalid component ID... should we notify the user about that somehow?
					// for now we keep the nullptr and use this to identify "invalid component" in the table
				}
				// side A
				if (ci.m_sideASurface != nullptr) {
					std::set<const VICUS::Object * >::const_iterator it_A = objs.find(ci.m_sideASurface);
					if (it_A != objs.end())
						m_componentSurfacesMap[comp].push_back(ci.m_sideASurface);
				}
				// side B
				if (ci.m_sideBSurface != nullptr) {
					std::set<const VICUS::Object * >::const_iterator it_B = objs.find(ci.m_sideBSurface);
					if (it_B != objs.end())
						m_componentSurfacesMap[comp].push_back(ci.m_sideBSurface);
				}
			}
			// now put the data of the map into the table
			m_ui->tableWidgetComponents->clearContents();
			m_ui->tableWidgetComponents->setRowCount(m_componentSurfacesMap.size());
			int row=0;
			for (std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> >::const_iterator
				 it = m_componentSurfacesMap.begin(); it != m_componentSurfacesMap.end(); ++it, ++row)
			{
				QTableWidgetItem * item = new QTableWidgetItem();
				// special handling for components with "invalid" component id
				if (it->first == nullptr)
					item->setBackground(QColor(255,128,128));
				else
					item->setBackground(it->first->m_color);
				item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
				m_ui->tableWidgetComponents->setItem(row, 0, item);

				item = new QTableWidgetItem();
				if (it->first == nullptr)
					item->setText(tr("<invalid component id>"));
				else
					item->setText(QString::fromStdString(it->first->m_displayName.string(IBK::MultiLanguageString::m_language, "en")));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				m_ui->tableWidgetComponents->setItem(row, 1, item);
			}
			// update button states
			on_tableWidgetComponents_itemSelectionChanged();
		} break;

		case BT_ComponentOrientation:
			m_ui->stackedWidget->setCurrentIndex(2);
		break;

		case BT_BoundaryConditions: {
			m_ui->stackedWidget->setCurrentIndex(3);
			// get all visible "building" type objects in the scene
			std::set<const VICUS::Object * > objs;
			project().selectObjects(objs, VICUS::Project::SG_Building, false, true);
			// now build a map of component IDs versus visible surfaces
			m_bcSurfacesMap.clear();
			for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
				// component ID assigned?
				if (ci.m_componentID == VICUS::INVALID_ID)
					continue; // no component, skip
				// lookup component in DB
				const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_componentID];
				const VICUS::BoundaryCondition * bcSideA = nullptr;
				const VICUS::BoundaryCondition * bcSideB = nullptr;
				if (comp != nullptr) {
					// lookup boundary condition pointers
					if (comp->m_idSideABoundaryCondition != VICUS::INVALID_ID)
						bcSideA = db.m_boundaryConditions[comp->m_idSideABoundaryCondition];
					if (comp->m_idSideBBoundaryCondition != VICUS::INVALID_ID)
						bcSideB = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition];
				}
				// side A
				if (ci.m_sideASurface != nullptr) {
					std::set<const VICUS::Object * >::const_iterator it_A = objs.find(ci.m_sideASurface);
					if (it_A != objs.end()) {
						m_bcSurfacesMap[bcSideA].push_back(ci.m_sideASurface);
					}
				}
				// side B
				if (ci.m_sideBSurface != nullptr) {
					std::set<const VICUS::Object * >::const_iterator it_B = objs.find(ci.m_sideBSurface);
					if (it_B != objs.end())
						m_bcSurfacesMap[bcSideB].push_back(ci.m_sideBSurface);
				}
			}
			// now put the data of the map into the table
			m_ui->tableWidgetBoundaryConditions->clearContents();
			m_ui->tableWidgetBoundaryConditions->setRowCount(m_bcSurfacesMap.size());
			int row=0;
			for (std::map<const VICUS::BoundaryCondition*, std::vector<const VICUS::Surface *> >::const_iterator
				 it = m_bcSurfacesMap.begin(); it != m_bcSurfacesMap.end(); ++it, ++row)
			{
				QTableWidgetItem * item = new QTableWidgetItem();
				// special handling for surfaces without bc assigned
				if (it->first == nullptr)
					item->setBackground(QColor(64,64,64));
				else
					item->setBackground(it->first->m_color);
				item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
				m_ui->tableWidgetBoundaryConditions->setItem(row, 0, item);

				item = new QTableWidgetItem();
				if (it->first == nullptr)
					item->setText(tr("<no/invalid boundary condition>"));
				else
					item->setText(QString::fromStdString(it->first->m_displayName.string(IBK::MultiLanguageString::m_language, "en")));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				m_ui->tableWidgetBoundaryConditions->setItem(row, 1, item);
			}
		} break;
	}
}


void SVPropBuildingEditWidget::onModified(int modificationType, ModificationInfo * data) {
	// react on selection changes only, then update propertiess
}


void SVPropBuildingEditWidget::on_toolButtonEdit_clicked() {

}


void SVPropBuildingEditWidget::on_pushButtonEditComponents_clicked() {
	SVMainWindow::instance().dbComponentEditDialog()->edit();
	// TODO : signal a "recoloring needed" signal to scene in case any of the colors have changed
	// update table (in case user has deleted some components or changed colors
	setPropertyType(BT_Components);
}


void SVPropBuildingEditWidget::on_pushButtonExchangeComponents_clicked() {
	const VICUS::Component * comp = currentlySelectedComponent();
	Q_ASSERT(comp != nullptr); // if nullptr, the button should be disabled!
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoReplaceComponent",
		tr("Replace component"), tr("This will replace all associations with component '%1 [%2]' with another component.")
			 .arg(VICUS::MultiLangString2QString(comp->m_displayName)).arg(comp->m_id));
	unsigned int oldId = comp->m_id;
	int newId = SVMainWindow::instance().dbComponentEditDialog()->select(oldId);
	if (newId == -1)
		return; // user has aborted the dialog
	// TODO : signal a "recoloring needed" signal to scene in case any of the colors have changed
	// update table (in case user has deleted some components or changed colors
	setPropertyType(BT_Components); // Note: this invalidates "comp"!

	// now compose an undo action and modify the project

	// first, we need to find the component instances which reference the "old" id
	comp = SVSettings::instance().m_db.m_components[oldId];
	if (comp == nullptr) {
		// the user has done something stupid and deleted the component that he wanted to replace. In this
		// case there is no longer a component with this ID and we have a nullptr
		QMessageBox::critical(this, tr("Replace component"), tr("Component with id %1 no longer found in component DB.").arg(oldId));
		return;
	}
	// now process all component instances and modify the component
	std::vector<VICUS::ComponentInstance> modCI = project().m_componentInstances;
	for (VICUS::ComponentInstance & ci : modCI) {
		if (ci.m_componentID == oldId)
			ci.m_componentID = (unsigned int)newId;
	}
	// create the undo action and modify project
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Components exchanged"), modCI);
	undo->push();
}


void SVPropBuildingEditWidget::on_pushButtonSelectObjectsWithComponent_clicked() {
	const VICUS::Component * comp = currentlySelectedComponent();
	Q_ASSERT(comp != nullptr); // if nullptr, the button should be disabled!
	// compose set of objects to be selected
	std::set<unsigned int> objs;
	for (auto s : m_componentSurfacesMap[comp])
		objs.insert(s->uniqueID());

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Select objects with component '%1'")
		.arg(VICUS::MultiLangString2QString(comp->m_displayName)), SVUndoTreeNodeState::SelectedState,
														 objs, true);
	undo->push();
}


void SVPropBuildingEditWidget::on_tableWidgetComponents_itemSelectionChanged() {
	// enable/disable buttons based on selection changed signal
	bool enabled = (currentlySelectedComponent() != nullptr);
	m_ui->pushButtonEditComponents->setEnabled(enabled);
	m_ui->pushButtonExchangeComponents->setEnabled(enabled);
	m_ui->pushButtonSelectObjectsWithComponent->setEnabled(enabled);
}


const VICUS::Component * SVPropBuildingEditWidget::currentlySelectedComponent() const {
	// check if selected "component" is actually a missing component
	int r = m_ui->tableWidgetComponents->currentRow();
	if (r == -1 || m_componentSurfacesMap.empty())
		return nullptr;
	std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> >::const_iterator cit = m_componentSurfacesMap.begin();
	std::advance(cit, r);
	return cit->first;
}

