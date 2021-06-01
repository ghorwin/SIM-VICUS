#include "SVPropAddWindowWidget.h"
#include "ui_SVPropAddWindowWidget.h"

#include <QSpinBox>

#include <VICUS_Project.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"

#include "Vic3DNewSubSurfaceObject.h"

SVPropAddWindowWidget::SVPropAddWindowWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropAddWindowWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditWindowWidth->setup(0, std::numeric_limits<double>::max(), tr("Window width in [m] must be > 0."), false, true);
	m_ui->lineEditWindowHeight->setup(0, std::numeric_limits<double>::max(), tr("Window height in [m] must be > 0."), false, true);
	m_ui->lineEditWindowSillHeight->setup(0, std::numeric_limits<double>::max(), tr("Window-sill height in [m] must be > 0."), false, true);
	m_ui->lineEditWindowWidthDistance->setup(0, std::numeric_limits<double>::max(), tr("Window distance in [m] must be > 0."), false, true);
	m_ui->lineEditWindowOffset->setup(0, std::numeric_limits<double>::max(), tr("Window offset in [m] must be > 0."), false, true);
	m_ui->lineEditWindowPercentage->setup(0, 100, tr("Window area percentage must be between 0 .. 100%."), true, true);

	m_ui->lineEditWindowWidth->setValue(0.6);
	m_ui->lineEditWindowHeight->setValue(1.2);
	m_ui->lineEditWindowSillHeight->setValue(0.8);
	m_ui->lineEditWindowWidthDistance->setValue(1.2);
	m_ui->lineEditWindowOffset->setValue(0.4);
	m_ui->lineEditWindowPercentage->setValue(35);

	connect(m_ui->spinBoxWindowWidth, SIGNAL(valueChanged(int)),
			this, SLOT(onSpinBoxValueChanged(int)));
	connect(m_ui->spinBoxWindowHeight, SIGNAL(valueChanged(int)),
			this, SLOT(onSpinBoxValueChanged(int)));
	connect(m_ui->spinBoxWindowSillHeight, SIGNAL(valueChanged(int)),
			this, SLOT(onSpinBoxValueChanged(int)));
	connect(m_ui->lineEditWindowWidthDistance, SIGNAL(valueChanged(int)),
			this, SLOT(onSpinBoxValueChanged(int)));

	SVViewStateHandler::instance().m_propAddWindowWidget = this;
}


SVPropAddWindowWidget::~SVPropAddWindowWidget() {
	delete m_ui;
}


void SVPropAddWindowWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	// react on selection changes only, then update properties
	switch ((SVProjectHandler::ModificationTypes)modificationType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::NodeStateModified:
		case SVProjectHandler::BuildingGeometryChanged:
			updateUi();
		break;

		// nothing to do for the remaining modification types
		case SVProjectHandler::BuildingTopologyChanged: // used when zone templates are assigned
		case SVProjectHandler::ComponentInstancesModified:
		case SVProjectHandler::SubSurfaceComponentInstancesModified:
		case SVProjectHandler::ObjectRenamed:
		case SVProjectHandler::SolverParametersModified:
		case SVProjectHandler::ClimateLocationModified:
		case SVProjectHandler::GridModified:
		case SVProjectHandler::NetworkModified:
		break;
	}
}


void SVPropAddWindowWidget::onSpinBoxValueChanged(int val) {
	// check sender's value against anyone elses
//	QSpinBox * senderBox = qobject_cast<QSpinBox*>(sender());
//	if (m_ui->spinBoxWindowWidth != senderBox && m_ui->spinBoxWindowWidth->value() == val) {
//		senderBox->blockSignals(true);
//		senderBox->setValue(m_ui->spinBoxWindowWidth->value());
//		senderBox->blockSignals(false);
//	}

}


void SVPropAddWindowWidget::updateUi() {
	// get selected objects - must have at least one surface selected
	const VICUS::Project & p = project();
	std::vector<const VICUS::Surface*> sel;
	bool haveAny = p.selectedSurfaces(sel, VICUS::Project::SG_Building);
	if (!haveAny) {
		m_ui->labelSelectSurfaces->show();
		m_ui->groupBoxWindows->setEnabled(false);
		// clear "new subsurfaces" object
		return;
	}
	else {
		m_ui->labelSelectSurfaces->hide();
		m_ui->groupBoxWindows->setEnabled(true);
	}
	updateGeometryObject();
}


void SVPropAddWindowWidget::updateGeometryObject() {
	// get list of selected surfaces
	const VICUS::Project & p = project();
	std::vector<const VICUS::Surface*> sel;
	p.selectedSurfaces(sel, VICUS::Project::SG_Building);
	Q_ASSERT(!sel.empty());

	// get width, height, window-sill-height, offset, etc.
	double surfW = m_ui->lineEditWindowWidth->value();
	double surfH = m_ui->lineEditWindowHeight->value();
	double surfWSH = m_ui->lineEditWindowSillHeight->value();
	double dist = m_ui->lineEditWindowWidthDistance->value();

	if (m_ui->tabWidgetWindow->currentIndex() == 0) {
		// percentage
		double surfPercentage = m_ui->lineEditWindowOffset->value();
		SVViewStateHandler::instance().m_newSubSurfaceObject->createByPercentage(sel, surfW, surfH, surfWSH, dist, surfPercentage, 0);
	}
	else {
		double surfOff = m_ui->lineEditWindowOffset->value();
		SVViewStateHandler::instance().m_newSubSurfaceObject->createWithOffset(sel, surfW, surfH, surfWSH, dist, surfOff, 0);
	}

}


void SVPropAddWindowWidget::setup() {
	updateUi();

	// populate input fields with meaningful defaults

}


void SVPropAddWindowWidget::on_lineEditWindowWidth_editingFinishedSuccessfully() {


}
