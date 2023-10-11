#include "SVPropAddWindowWidget.h"
#include "ui_SVPropAddWindowWidget.h"

#include <QSpinBox>

#include <SVConversions.h>

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVDatabase.h"
#include "SVSettings.h"
#include "SVGeometryView.h"

#include "Vic3DNewSubSurfaceObject.h"
#include "SVUndoModifySurfaceGeometry.h"


SVPropAddWindowWidget::SVPropAddWindowWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropAddWindowWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditWindowWidth->setup(0, std::numeric_limits<double>::max(), tr("Window width in [m] must be > 0."), false, true);
	m_ui->lineEditWindowHeight->setup(0, std::numeric_limits<double>::max(), tr("Window height in [m] must be > 0."), false, true);
	m_ui->lineEditWindowSillHeight->setup(0, std::numeric_limits<double>::max(), tr("Window-sill height in [m] must be > 0."), false, true);
	m_ui->lineEditWindowDistance->setup(0, std::numeric_limits<double>::max(), tr("Window distance in [m] must be > 0."), false, true);
	m_ui->lineEditWindowOffset->setup(0, std::numeric_limits<double>::max(), tr("Window offset in [m] must be > 0."), false, true);
	m_ui->lineEditWindowPercentage->setup(0, 100, tr("Window area percentage must be between 0 .. 100%."), false, false);

	// TODO  : restore previously used settings
	m_windowInputData.m_width = 1.2;
	m_windowInputData.m_height = 1.5;
	m_windowInputData.m_windowSillHeight = 0.8;
	m_windowInputData.m_distance = 0.5;
	m_windowInputData.m_priorities[0] = 2;		// width
	m_windowInputData.m_priorities[1] = 1;		// height
	m_windowInputData.m_priorities[2] = 3;		// sill height
	m_windowInputData.m_priorities[3] = 4;		// distance
	m_windowInputData.m_byPercentage = true;
	m_windowInputData.m_percentage = 30;
	m_windowInputData.m_baseLineOffset = 0.4;

	m_prioritySpinBoxes[0] = m_ui->spinBoxWindowWidth;
	m_prioritySpinBoxes[1] = m_ui->spinBoxWindowHeight;
	m_prioritySpinBoxes[2] = m_ui->spinBoxWindowSillHeight;
	m_prioritySpinBoxes[3] = m_ui->spinBoxWindowWidthDistance;

	for (unsigned int i=0; i<4; ++i)
		connect(m_prioritySpinBoxes[i], SIGNAL(valueChanged(int)),
				this, SLOT(onSpinBoxValueChanged(int)));

	SVViewStateHandler::instance().m_propAddWindowWidget = this;

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropAddWindowWidget::onModified);

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
			if (isVisible())
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
		case SVProjectHandler::NetworkGeometryChanged:
		case SVProjectHandler::NetworkDataChanged:
		case SVProjectHandler::ClimateLocationAndFileModified:
		case SVProjectHandler::OutputsModified:
		case SVProjectHandler::DrawingModified:
		case SVProjectHandler::LcaLccModified:
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
	bool haveAny = p.selectedSurfaces(m_currentSelection, VICUS::Project::SG_Building);
	if (!haveAny) {
		m_ui->labelSelectSurfaces->show();
		m_ui->groupBoxWindows->setEnabled(false);
		// clear "new subsurfaces" object
		SVViewStateHandler::instance().m_newSubSurfaceObject->clear();
		return;
	}
	else {
		m_ui->labelSelectSurfaces->hide();
		m_ui->groupBoxWindows->setEnabled(true);
	}
	updateGeometryObject();
}


void SVPropAddWindowWidget::updateGeometryObject() {
	QString errorMsg;
	if (!SVViewStateHandler::instance().m_newSubSurfaceObject->generateSubSurfaces(m_currentSelection, m_windowInputData, errorMsg)) {
		QMessageBox::critical(this, QString(), errorMsg);
		return;
	}
	// we need to trigger a redraw here
	SVViewStateHandler::instance().m_geometryView->refreshSceneView();
}


void SVPropAddWindowWidget::setup() {
	// populate input fields with meaningful defaults
	m_ui->lineEditWindowWidth->setValue(m_windowInputData.m_width);
	m_ui->lineEditWindowHeight->setValue(m_windowInputData.m_height);
	m_ui->lineEditWindowDistance->setValue(m_windowInputData.m_distance);
	m_ui->lineEditWindowOffset->setValue(m_windowInputData.m_baseLineOffset);
	m_ui->lineEditWindowPercentage->setValue(m_windowInputData.m_percentage);
	m_ui->lineEditWindowSillHeight->setValue(m_windowInputData.m_windowSillHeight);

	m_ui->spinBoxMaxHoleCount->blockSignals(true);
	m_ui->spinBoxMaxHoleCount->setValue((int)m_windowInputData.m_maxHoleCount);
	m_ui->spinBoxMaxHoleCount->blockSignals(false);
	m_ui->tabWidgetWindow->blockSignals(true);
	if (m_windowInputData.m_byPercentage)
		m_ui->tabWidgetWindow->setCurrentIndex(0);
	else
		m_ui->tabWidgetWindow->setCurrentIndex(1);
	m_ui->tabWidgetWindow->blockSignals(false);

	// TODO : set priorities

	on_radioButtonSubSurfaceTypeWindow_toggled(m_ui->radioButtonSubSurfaceTypeWindow->isChecked());
	updateUi();
}


void SVPropAddWindowWidget::updateSubSurfaceComponentList() {
	on_radioButtonSubSurfaceTypeWindow_toggled(m_ui->radioButtonSubSurfaceTypeWindow->isChecked());
}


void SVPropAddWindowWidget::on_lineEditWindowWidth_editingFinishedSuccessfully() {
	m_windowInputData.m_width = m_ui->lineEditWindowWidth->value();
	updateGeometryObject();
}

void SVPropAddWindowWidget::on_lineEditWindowHeight_editingFinishedSuccessfully() {
	m_windowInputData.m_height = m_ui->lineEditWindowHeight->value();
	updateGeometryObject();
}

void SVPropAddWindowWidget::on_lineEditWindowSillHeight_editingFinishedSuccessfully() {
	m_windowInputData.m_windowSillHeight = m_ui->lineEditWindowSillHeight->value();
	updateGeometryObject();
}

void SVPropAddWindowWidget::on_lineEditWindowDistance_editingFinishedSuccessfully() {
	m_windowInputData.m_distance = m_ui->lineEditWindowDistance->value();
	updateGeometryObject();
}

void SVPropAddWindowWidget::on_lineEditWindowOffset_editingFinishedSuccessfully() {
	m_windowInputData.m_baseLineOffset = m_ui->lineEditWindowOffset->value();
	updateGeometryObject();
}

void SVPropAddWindowWidget::on_lineEditWindowPercentage_editingFinishedSuccessfully() {
	m_windowInputData.m_percentage = m_ui->lineEditWindowPercentage->value();
	updateGeometryObject();
}

void SVPropAddWindowWidget::on_spinBoxMaxHoleCount_valueChanged(int arg1) {
	m_windowInputData.m_maxHoleCount = (unsigned int)arg1;
	updateGeometryObject();
}

void SVPropAddWindowWidget::on_radioButtonSubSurfaceTypeWindow_toggled(bool checked) {
	if (checked) {
		m_ui->comboBoxSubSurfaceComponent->setEnabled(true);
		m_ui->comboBoxSubSurfaceComponent->clear();
		// process all window components and populate box
		const SVDatabase & db = SVSettings::instance().m_db;
		for (const auto & w : db.m_subSurfaceComponents) {
			m_ui->comboBoxSubSurfaceComponent->addItem( QtExt::MultiLangString2QString(w.second.m_displayName),
														w.second.m_id);
		}
		m_ui->comboBoxSubSurfaceComponent->setCurrentIndex(m_ui->comboBoxSubSurfaceComponent->count()-1);
	}
	else {
		m_ui->comboBoxSubSurfaceComponent->setEnabled(false);
		m_ui->comboBoxSubSurfaceComponent->clear();
	}
}


void SVPropAddWindowWidget::on_tabWidgetWindow_currentChanged(int index) {
	m_windowInputData.m_byPercentage = (index == 0);
	updateGeometryObject();
}


void SVPropAddWindowWidget::on_pushButtonCreate_clicked() {
	// retrieve generated geometry from new subsurface object
	// and create undo-action

	// process all selected surface
	std::vector<VICUS::Surface> modSurfaces; // here, we store only the modified surfaces
	std::vector<VICUS::Drawing> modDrawings; // here, we store only the modified drawings

	// in case the original surfaces did have already subsurface components assigned, search and remove them
	std::set<unsigned int> subSurfaceIDsToRemove;
	for (unsigned int i=0; i<m_currentSelection.size(); ++i) {
		const VICUS::Surface* s = m_currentSelection[i];
		for (const VICUS::SubSurface & sub : s->subSurfaces()) {
			if (sub.m_subSurfaceComponentInstance != nullptr)
				subSurfaceIDsToRemove.insert(sub.m_subSurfaceComponentInstance->m_id);
		}
	}

	// populate a vector with existing and remaining subsurface component instances
	std::vector<VICUS::SubSurfaceComponentInstance> subSurfaceComponentInstances;

	for (const VICUS::SubSurfaceComponentInstance & subComp : project().m_subSurfaceComponentInstances) {
		if (subSurfaceIDsToRemove.find(subComp.m_id) == subSurfaceIDsToRemove.end())
			subSurfaceComponentInstances.push_back(subComp);
	}

	const std::vector<VICUS::PlaneGeometry> & geometries = SVViewStateHandler::instance().m_newSubSurfaceObject->generatedSurfaces();
	IBK_ASSERT(geometries.size() == m_currentSelection.size());

	unsigned int lastFreeId = project().nextUnusedID();

	for (unsigned int i=0; i<m_currentSelection.size(); ++i) {
		const VICUS::Surface* s = m_currentSelection[i];

		VICUS::Surface newSurf(*s);
		newSurf.setPolygon3D( geometries[i].polygon3D() ); // update polygon (is this really necessary?)

		// now add subsurface objects for each hole in the polygon
		std::vector<VICUS::SubSurface> subs;
        for (const VICUS::PlaneGeometry::Hole & h : geometries[i].holes()) {
            const VICUS::Polygon2D &p = h.m_holeGeometry;

			VICUS::SubSurface subsurf;
			subsurf.m_id = lastFreeId++;

			subsurf.m_polygon2D = p;
			if (m_ui->radioButtonSubSurfaceTypeWindow->isChecked()) {
				subsurf.m_displayName = tr("Window #%1").arg(subsurf.m_id);
				subsurf.m_color = QColor(96,96,255,64);
			}
			else {
				subsurf.m_displayName = tr("Door #%1").arg(subsurf.m_id);
				subsurf.m_color = QColor(164,164,164,255);
			}

			subs.push_back(subsurf);

			// also create subsurface component instances
			// but only if we have a valid subsurface component selected
			if (s->m_componentInstance != nullptr &&
				m_ui->comboBoxSubSurfaceComponent->count() != 0 &&
				m_ui->comboBoxSubSurfaceComponent->currentIndex() != -1)
			{
				VICUS::SubSurfaceComponentInstance subInstance;
				subInstance.m_id = lastFreeId++;
				subInstance.m_idSubSurfaceComponent = m_ui->comboBoxSubSurfaceComponent->currentData().toUInt();
				subInstance.m_idSideASurface = subsurf.m_id;
				subInstance.m_idSideBSurface = VICUS::INVALID_ID; // currently, all our new windows are outside windows
				subSurfaceComponentInstances.push_back(subInstance);
			}
		}

        std::vector<VICUS::Surface> childs = newSurf.childSurfaces();

        newSurf.setChildAndSubSurfaces(subs, childs);
		modSurfaces.push_back(newSurf);
	}

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Added sub-surfaces/windows"),
																		 modSurfaces,
																		 modDrawings,
																		 &subSurfaceComponentInstances);
	undo->push();

	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
	SVViewStateHandler::instance().setViewState(vs);
}

void SVPropAddWindowWidget::on_pushButtonChangeLocalOrigin_clicked() {
	// process all selected surface
	std::vector<VICUS::Surface> modSurfaces; // here, we store only the modified surfaces
	std::vector<VICUS::Drawing> modDrawings; // here, we store only the modified drawings

	for (unsigned int i=0; i<m_currentSelection.size(); ++i) {
		VICUS::Surface* s = const_cast<VICUS::Surface*>(m_currentSelection[i]);
		if(s->polygon3D().vertexes().size()<3)
			continue;

		s->changeOrigin(1);

		modSurfaces.push_back(*s);
	}
	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("New polygon origin."),
		modSurfaces, modDrawings, nullptr);
	undo->push();
}

