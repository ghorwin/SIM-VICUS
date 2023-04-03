#include "SVPropStructuralUnitEditWidget.h"
#include "ui_SVPropStructuralUnitEditWidget.h"

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVConstants.h"
#include "SVPropBuildingStructuralUnitWidget.h"
#include "SVPropBuildingZoneProperty.h"
#include "SVPropFloorManagerWidget.h"
#include "SVMainWindow.h"
#include "SVPropSupplySystemsWidget.h"
#include "SVPreferencesDialog.h"
#include "SVPreferencesPageStyle.h"


SVPropStructuralUnitEditWidget::SVPropStructuralUnitEditWidget(QWidget *parent) :
    QWidget(parent),
	m_ui(new Ui::SVPropStructuralUnitEditWidget)
{
	m_ui->setupUi(this);
	m_ui->toolBox->layout()->setMargin(0);

	// add pages to tool box:
	// Note: NEVER change the order, it must correspond to StructuralUnitPropertyType enum.
	m_ui->toolBox->blockSignals(true);
	m_ui->toolBox->addPage(tr("Structural unit"), new SVPropBuildingStructuralUnitWidget(this));
	m_ui->toolBox->addPage(tr("Room properties"), new SVPropBuildingZoneProperty(this));
	m_ui->toolBox->addPage(tr("Building levels"), new SVPropFloorManagerWidget(this));

	m_ui->toolBox->blockSignals(false);
	m_ui->toolBox->setCurrentIndex(ST_StructuralUnit);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropStructuralUnitEditWidget::onModified);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::colorRefreshNeeded,
			this, &SVPropStructuralUnitEditWidget::onColorRefreshNeeded);

	connect(m_ui->toolBox, &QtExt::ToolBox::indexChanged,
			this, &SVPropStructuralUnitEditWidget::onCurrentBuildingPropertyTypeChanged);

	connect(SVMainWindow::instance().preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged,
			this, &SVPropStructuralUnitEditWidget::onStyleChanged);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified, nullptr);

	// update color view initially
	onCurrentBuildingPropertyTypeChanged((BuildingPropertyType)m_ui->toolBox->currentIndex());
}

SVPropStructuralUnitEditWidget::~SVPropStructuralUnitEditWidget()
{
	delete m_ui;
}

void SVPropStructuralUnitEditWidget::setPropertyType(int structuralUnitPropertyType) {
	m_ui->toolBox->setCurrentIndex((unsigned int)structuralUnitPropertyType);
}



void SVPropStructuralUnitEditWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	// react on selection changes only, then update properties
	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {
		// This is meant to update all widgets when e.g. a new project has been loaded
		case SVProjectHandler::AllModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::BuildingTopologyChanged: // used when zone templates are assigned
		case SVProjectHandler::ComponentInstancesModified:
		case SVProjectHandler::SubSurfaceComponentInstancesModified:
		case SVProjectHandler::StructuralUnitsModified:
			updateUi();
		break;

// nothing to do for the remaining modification types
		case SVProjectHandler::ObjectRenamed:
		case SVProjectHandler::SolverParametersModified:
		case SVProjectHandler::ClimateLocationModified:
		case SVProjectHandler::GridModified:
		case SVProjectHandler::NetworkGeometryChanged:
		case SVProjectHandler::NetworkDataChanged:
		case SVProjectHandler::NodeStateModified:
		break;
	}
}

void SVPropStructuralUnitEditWidget::onColorRefreshNeeded() {
	// for now we just rebuild the widgets... this might be changed in the future, if performance issues arise
	// since updating the table color rows is usually much faster than rebuilding the entire UI
	updateUi();
}

void SVPropStructuralUnitEditWidget::onCurrentBuildingPropertyTypeChanged(int propertyType) {
	StructuralUnitPropertyType structuralUnitPropType = StructuralUnitPropertyType(propertyType);
	// set coloring mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	switch (structuralUnitPropType) {
		case ST_StructuralUnit				: vs.m_objectColorMode = SVViewState::OCM_StructuralUnit ; break;
		case ST_ZoneProperty			: vs.m_objectColorMode = SVViewState::OCM_ZoneTemplates; break;
		case ST_FloorManager			: vs.m_objectColorMode = SVViewState::OCM_None; break;
		}
		SVViewStateHandler::instance().setViewState(vs);

	}


unsigned int SVPropStructuralUnitEditWidget::currentPropertyType() {
	return m_ui->toolBox->currentIndex();
}


void SVPropStructuralUnitEditWidget::onStyleChanged() {
	m_ui->toolBox->updatePageBackgroundColorFromStyle();
}


// *** PRIVATE FUNCTIONS ***

void SVPropStructuralUnitEditWidget::updateUi() {
	// TODO Andreas : this function currently updates all widgets in the stacked widget, regardless of which
	//                is currently visible. This makes switching property modes very fast, but whenever the project
	//                data changes, it takes a bit more time. If this becomes a performance issue at some point,
	//                modify the update logic.
	// Note: It is not meaningful to update the widgets based on their visibility.
	// It could be that project data changes and then the user switches to a different widget, which has then not be updated yet.
	dynamic_cast<SVPropBuildingStructuralUnitWidget*>(m_ui->toolBox->widget(ST_StructuralUnit))->updateUi();
	dynamic_cast<SVPropBuildingZoneProperty*>(m_ui->toolBox->widget(ST_ZoneProperty))->updateUi();

	// SVPropFloorManagerWidget has its own onModified() slot, no need to handle that here
}





