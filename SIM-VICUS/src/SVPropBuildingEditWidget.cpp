#include "SVPropBuildingEditWidget.h"
#include "ui_SVPropBuildingEditWidget.h"

#include <VICUS_Component.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVUndoSiteDataChanged.h"
#include "SVConstants.h"
#include "SVStyle.h"

SVPropBuildingEditWidget::SVPropBuildingEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropBuildingEditWidget::onModified);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified, nullptr);
}


SVPropBuildingEditWidget::~SVPropBuildingEditWidget() {
	delete m_ui;
}


void SVPropBuildingEditWidget::setPropertyType(int buildingPropertyType) {
	m_ui->groupBoxObjectList->setVisible(false);
	m_ui->widgetStretch->setVisible(true);
	switch ((BuildingPropertyTypes)buildingPropertyType) {
		case BT_Components: {
			m_ui->groupBoxObjectList->setTitle(tr("Building components"));
			m_ui->groupBoxObjectList->setVisible(true);
			m_ui->widgetStretch->setVisible(false);
			// tell scene to change coloring of visible objects and retrieve map of
			// colored objects
			SVViewState vs = SVViewStateHandler::instance().viewState();
			vs.m_objectColorMode = SVViewState::OCM_Components;
			// set view state - this will re-color the scene based on component association.
			SVViewStateHandler::instance().setViewState(vs);
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
				if (comp != nullptr) {
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
			m_ui->tableWidgetObjects->clear();
			m_ui->tableWidgetObjects->setColumnCount(2);
			m_ui->tableWidgetObjects->setHorizontalHeaderLabels(QStringList() << QString() << tr("Component name"));
			m_ui->tableWidgetObjects->setRowCount(m_componentSurfacesMap.size());
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
				m_ui->tableWidgetObjects->setItem(row, 0, item);

				item = new QTableWidgetItem();
				if (it->first == nullptr)
					item->setText(tr("<invalid component id>"));
				else
					item->setText(QString::fromStdString(it->first->m_displayName.string(IBK::MultiLanguageString::m_language, "en")));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				m_ui->tableWidgetObjects->setItem(row, 1, item);
			}
			SVStyle::formatDatabaseTableView(m_ui->tableWidgetObjects);
			m_ui->tableWidgetObjects->setSortingEnabled(false);
			m_ui->tableWidgetObjects->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
			m_ui->tableWidgetObjects->horizontalHeader()->resizeSection(0,20);
			m_ui->tableWidgetObjects->horizontalHeader()->setStretchLastSection(true);

		} break;
		case BT_ComponentOrientation:
		break;
		case BT_BoundaryConditions:
		break;
	}
}


void SVPropBuildingEditWidget::onModified(int modificationType, ModificationInfo * data) {
	// react on selection changes only, then update properties
}


void SVPropBuildingEditWidget::on_toolButtonEdit_clicked() {

}
