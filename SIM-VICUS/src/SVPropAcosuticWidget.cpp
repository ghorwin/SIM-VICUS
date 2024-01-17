#include "SVPropAcosuticWidget.h"
#include "ui_SVPropAcosuticWidget.h"

#include "SVProjectHandler.h"
#include "SVPropBuildingSoundProtectionTemplatesWidget.h"
#include "SVPropBuildingAcousticTemplatesWidget.h"

#include "SVConstants.h"
#include "SVMainWindow.h"
#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVPreferencesDialog.h"
#include "SVPreferencesPageStyle.h"

SVPropAcosuticWidget::SVPropAcosuticWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropAcosuticWidget)
{
	m_ui->setupUi(this);
	m_ui->toolBox->layout()->setMargin(0);

	// add pages to tool box:
	// Note: NEVER change the order, it must correspond to StructuralUnitPropertyType enum.
	m_ui->toolBox->blockSignals(true);
	m_ui->toolBox->addPage(tr("Sound protection templates"), new SVPropBuildingSoundProtectionTemplatesWidget(this));
	m_ui->toolBox->addPage(tr("Acoustic templates"), new SVPropBuildingAcousticTemplatesWidget(this));


	m_ui->toolBox->blockSignals(false);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropAcosuticWidget::onModified);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::colorRefreshNeeded,
			this, &SVPropAcosuticWidget::onColorRefreshNeeded);

	connect(m_ui->toolBox, &QtExt::ToolBox::indexChanged,
			this, &SVPropAcosuticWidget::onCurrentBuildingPropertyTypeChanged);

	connect(SVMainWindow::instance().preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged,
			this, &SVPropAcosuticWidget::onStyleChanged);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified, nullptr);

	// update color view initially
	onCurrentBuildingPropertyTypeChanged((BuildingPropertyType)m_ui->toolBox->currentIndex());
}

SVPropAcosuticWidget::~SVPropAcosuticWidget() {
	delete m_ui;
}

void SVPropAcosuticWidget::setPropertyType(int acousticPropertyType) {
	m_ui->toolBox->setCurrentIndex((unsigned int)acousticPropertyType);
}

void SVPropAcosuticWidget::onModified(int modificationType, ModificationInfo */*data*/) {
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

void SVPropAcosuticWidget::onColorRefreshNeeded() {
	// for now we just rebuild the widgets... this might be changed in the future, if performance issues arise
	// since updating the table color rows is usually much faster than rebuilding the entire UI
	updateUi();
}

void SVPropAcosuticWidget::onCurrentBuildingPropertyTypeChanged(int propertyType) {
	AcousticPropertyType structuralUnitPropType = AcousticPropertyType(propertyType);
	// set coloring mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	switch (structuralUnitPropType) {
		case AT_AcousticTemplate:
			vs.m_objectColorMode = SVViewState::OCM_AcousticRoomTemplates;
		break;
		case AT_AcousticSoundProtectionTemplate:
			vs.m_objectColorMode = SVViewState::OCM_SoundProtectionRoomTemplates;
		break;
	}
	SVViewStateHandler::instance().setViewState(vs);

}

unsigned int SVPropAcosuticWidget::currentPropertyType() {
	return m_ui->toolBox->currentIndex();
}


void SVPropAcosuticWidget::onStyleChanged() {
	m_ui->toolBox->updatePageBackgroundColorFromStyle();
}

// *** PRIVATE FUNCTIONS ***

void SVPropAcosuticWidget::updateUi() {
	dynamic_cast<SVPropBuildingSoundProtectionTemplatesWidget*>(m_ui->toolBox->widget(AT_AcousticSoundProtectionTemplate))->updateUi();
	dynamic_cast<SVPropBuildingAcousticTemplatesWidget*>(m_ui->toolBox->widget(AT_AcousticTemplate))->updateUi();
}






