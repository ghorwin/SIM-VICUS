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

	m_windowInputData.m_width = 1.8;
	m_windowInputData.m_height = 2.2;
	m_windowInputData.m_windowSillHeight = 0.4;
	m_windowInputData.m_distance = 0.5;
	m_windowInputData.m_priorities[0] = 1;
	m_windowInputData.m_priorities[1] = 2;
	m_windowInputData.m_priorities[2] = 3;
	m_windowInputData.m_priorities[3] = 4;

	m_prioritySpinBoxes[0] = m_ui->spinBoxWindowWidth;
	m_prioritySpinBoxes[1] = m_ui->spinBoxWindowHeight;
	m_prioritySpinBoxes[2] = m_ui->spinBoxWindowSillHeight;
	m_prioritySpinBoxes[3] = m_ui->spinBoxWindowWidthDistance;

	for (unsigned int i=0; i<4; ++i)
		connect(m_prioritySpinBoxes[i], SIGNAL(valueChanged(int)),
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
	QSpinBox * senderBox = qobject_cast<QSpinBox*>(sender());
	unsigned int senderIdx = 4;
	unsigned int otherIdx = 4;
	for (unsigned int i=0; i<4; ++i) {
		if (m_windowInputData.m_priorities[i] == val)
			otherIdx = i;
		if (m_prioritySpinBoxes[i] == senderBox)
			senderIdx = i;
	}
	Q_ASSERT(senderIdx != 4);
	Q_ASSERT(otherIdx != 4);
	m_prioritySpinBoxes[otherIdx]->blockSignals(true);
	m_prioritySpinBoxes[otherIdx]->setValue(m_windowInputData.m_priorities[senderIdx]);
	m_prioritySpinBoxes[otherIdx]->blockSignals(false);

	// now swap the stored values
	std::swap(m_windowInputData.m_priorities[senderIdx], m_windowInputData.m_priorities[otherIdx]);

	updateGeometryObject();
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

//	if (m_ui->tabWidgetWindow->currentIndex() == 0) {
//		// percentage
//		m_
//		SVViewStateHandler::instance().m_newSubSurfaceObject->createByPercentage(sel, surfW, surfH, surfWSH, dist, surfPercentage, 0);
//	}
//	else {
//		double surfOff = m_ui->lineEditWindowOffset->value();
//		SVViewStateHandler::instance().m_newSubSurfaceObject->createWithOffset(sel, surfW, surfH, surfWSH, dist, surfOff, 0);
//	}

}


void SVPropAddWindowWidget::setup() {
	updateUi();

	// populate input fields with meaningful defaults

}


void SVPropAddWindowWidget::on_lineEditWindowWidth_editingFinishedSuccessfully() {
	m_windowInputData.m_width = m_ui->lineEditWindowWidth->value();
}

