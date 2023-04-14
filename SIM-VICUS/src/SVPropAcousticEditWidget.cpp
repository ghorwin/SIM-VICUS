#include "SVPropAcousticEditWidget.h"
#include "ui_SVPropAcousticEditWidget.h"

#include "SVProjectHandler.h"
#include "SVPropBuildingAcousticComponentWidget.h"
#include "SVPropBuildingAcousticTemplatesWidget.h"



#include "SVMainWindow.h"
#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVPreferencesDialog.h"
#include "SVPreferencesPageStyle.h"

SVPropAcousticEditWidget::SVPropAcousticEditWidget(QWidget *parent) :
    QWidget(parent),
	m_ui(new Ui::SVPropAcousticEditWidget)
{
	m_ui->setupUi(this);
	m_ui->toolBox->layout()->setMargin(0);

	// add pages to tool box:
	// Note: NEVER change the order, it must correspond to StructuralUnitPropertyType enum.
	m_ui->toolBox->blockSignals(true);
	m_ui->toolBox->addPage(tr("Acoustic Templates"), new SVPropBuildingAcousticTemplatesWidget(this));
	m_ui->toolBox->addPage(tr("Acoustic Components"), new SVPropBuildingAcousticComponentWidget(this));


	m_ui->toolBox->blockSignals(false);
	m_ui->toolBox->setCurrentIndex(ST_StructuralUnit);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropAcousticEditWidget::onModified);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::colorRefreshNeeded,
			this, &SVPropAcousticEditWidget::onColorRefreshNeeded);

	connect(m_ui->toolBox, &QtExt::ToolBox::indexChanged,
			this, &SVPropAcousticEditWidget::onCurrentBuildingPropertyTypeChanged);

	connect(SVMainWindow::instance().preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged,
			this, &SVPropAcousticEditWidget::onStyleChanged);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified, nullptr);

	// update color view initially
	onCurrentBuildingPropertyTypeChanged((BuildingPropertyType)m_ui->toolBox->currentIndex());
}

SVPropAcousticEditWidget::~SVPropAcousticEditWidget()
{
	delete m_ui;
}

void SVPropAcousticEditWidget::setPropertyType(int acousticPropertyType) {
	m_ui->toolBox->setCurrentIndex((unsigned int)acousticPropertyType);
}

void SVPropAcousticEditWidget::onModified(int modificationType, ModificationInfo */*data*/) {
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
		break;
	}
}

void SVPropAcousticEditWidget::onColorRefreshNeeded() {
	// for now we just rebuild the widgets... this might be changed in the future, if performance issues arise
	// since updating the table color rows is usually much faster than rebuilding the entire UI
	updateUi();
}

void SVPropAcousticEditWidget::onCurrentBuildingPropertyTypeChanged(int propertyType) {
	AcousticPropertyType structuralUnitPropType = AcousticPropertyType(propertyType);
	// set coloring mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	switch (structuralUnitPropType) {
	// TODO Anton change accordingly
		case AT_AcousticComponent			: vs.m_objectColorMode = SVViewState::OCM_AcousticComponent ; break;
		case AT_AcousticTemplate			: vs.m_objectColorMode = SVViewState::OCM_AcousticRoomType; break;
		}
		SVViewStateHandler::instance().setViewState(vs);

}

unsigned int SVPropAcousticEditWidget::currentPropertyType() {
	return m_ui->toolBox->currentIndex();
}


void SVPropAcousticEditWidget::onStyleChanged() {
	m_ui->toolBox->updatePageBackgroundColorFromStyle();
}

// *** PRIVATE FUNCTIONS ***

void SVPropAcousticEditWidget::updateUi() {
	// TODO Andreas : this function currently updates all widgets in the stacked widget, regardless of which
	//                is currently visible. This makes switching property modes very fast, but whenever the project
	//                data changes, it takes a bit more time. If this becomes a performance issue at some point,
	//                modify the update logic.
	// Note: It is not meaningful to update the widgets based on their visibility.
	// It could be that project data changes and then the user switches to a different widget, which has then not be updated yet.
	dynamic_cast<SVPropBuildingAcousticTemplatesWidget*>(m_ui->toolBox->widget(AT_AcousticTemplate))->updateUi();
	dynamic_cast<SVPropBuildingAcousticComponentWidget*>(m_ui->toolBox->widget(AT_AcousticComponent))->updateUi();

	// SVPropFloorManagerWidget has its own onModified() slot, no need to handle that here
}






