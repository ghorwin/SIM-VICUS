#include "SVPropSoundProtectionEditWidget.h"
#include "ui_SVPropSoundProtectionEditWidget.h"

#include "SVProjectHandler.h"
#include "SVPropBuildingSoundProtectionTemplatesWidget.h"

#include "SVMainWindow.h"
#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVPreferencesDialog.h"
#include "SVPreferencesPageStyle.h"

SVPropSoundProtectionWidget::SVPropSoundProtectionWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropSoundProtectionWidget)
{
	m_ui->setupUi(this);
	m_ui->toolBox->layout()->setMargin(0);

	// add pages to tool box:
	// Note: NEVER change the order, it must correspond to StructuralUnitPropertyType enum.
	m_ui->toolBox->blockSignals(true);
	m_ui->toolBox->addPage(tr("Acoustic Templates"), new SVPropBuildingSoundProtectionTemplatesWidget(this));


	m_ui->toolBox->blockSignals(false);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropSoundProtectionWidget::onModified);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::colorRefreshNeeded,
			this, &SVPropSoundProtectionWidget::onColorRefreshNeeded);

	connect(m_ui->toolBox, &QtExt::ToolBox::indexChanged,
			this, &SVPropSoundProtectionWidget::onCurrentBuildingPropertyTypeChanged);

	connect(SVMainWindow::instance().preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged,
			this, &SVPropSoundProtectionWidget::onStyleChanged);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified, nullptr);

	// update color view initially
	onCurrentBuildingPropertyTypeChanged((BuildingPropertyType)m_ui->toolBox->currentIndex());
}

SVPropSoundProtectionWidget::~SVPropSoundProtectionWidget() {
	delete m_ui;
}

void SVPropSoundProtectionWidget::setPropertyType(int acousticPropertyType) {
	m_ui->toolBox->setCurrentIndex((unsigned int)acousticPropertyType);
}

void SVPropSoundProtectionWidget::onModified(int modificationType, ModificationInfo */*data*/) {
	// react on selection changes only, then update properties
	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {
	// This is meant to update all widgets when e.g. a new project has been loaded
	case SVProjectHandler::AllModified:
	case SVProjectHandler::BuildingGeometryChanged:
	case SVProjectHandler::BuildingTopologyChanged: // used when zone templates are assigned
	case SVProjectHandler::ComponentInstancesModified:
	case SVProjectHandler::SubSurfaceComponentInstancesModified:
	case SVProjectHandler::NodeStateModified:
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
	case SVProjectHandler::ClimateLocationAndFileModified:
	case SVProjectHandler::OutputsModified:
	case SVProjectHandler::DrawingModified:
	case SVProjectHandler::LcaLccModified:
		break;
	}
}

void SVPropSoundProtectionWidget::onColorRefreshNeeded() {
	// for now we just rebuild the widgets... this might be changed in the future, if performance issues arise
	// since updating the table color rows is usually much faster than rebuilding the entire UI
	updateUi();
}

void SVPropSoundProtectionWidget::onCurrentBuildingPropertyTypeChanged(int propertyType) {
	AcousticPropertyType structuralUnitPropType = AcousticPropertyType(propertyType);
	// set coloring mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	switch (structuralUnitPropType) {
		case AT_AcousticTemplate:
			vs.m_objectColorMode = SVViewState::OCM_AcousticRoomType;
		break;
	}
	SVViewStateHandler::instance().setViewState(vs);

}

unsigned int SVPropSoundProtectionWidget::currentPropertyType() {
	return m_ui->toolBox->currentIndex();
}


void SVPropSoundProtectionWidget::onStyleChanged() {
	m_ui->toolBox->updatePageBackgroundColorFromStyle();
}

// *** PRIVATE FUNCTIONS ***

void SVPropSoundProtectionWidget::updateUi() {
	// TODO Andreas : this function currently updates all widgets in the stacked widget, regardless of which
	//                is currently visible. This makes switching property modes very fast, but whenever the project
	//                data changes, it takes a bit more time. If this becomes a performance issue at some point,
	//                modify the update logic.
	// Note: It is not meaningful to update the widgets based on their visibility.
	// It could be that project data changes and then the user switches to a different widget, which has then not be updated yet.
	dynamic_cast<SVPropBuildingSoundProtectionTemplatesWidget*>(m_ui->toolBox->widget(AT_AcousticTemplate))->updateUi();

	// SVPropFloorManagerWidget has its own onModified() slot, no need to handle that here
}






