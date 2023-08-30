#include "SVPropBuildingComponentOrientationWidget.h"
#include "ui_SVPropBuildingComponentOrientationWidget.h"


#include <SVConversions.h>

#include "SVStyle.h"
#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVUndoModifyComponentInstances.h"
#include "SVUndoTreeNodeState.h"

SVPropBuildingComponentOrientationWidget::SVPropBuildingComponentOrientationWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingComponentOrientationWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayout->setMargin(0);

	m_ui->frameSideA->setStyleSheet(".QFrame { background-color: #2f7dd4; }");
	m_ui->frameSideB->setStyleSheet(".QFrame { background-color: #ffce30; }");

	QPalette p;
	p.setColor(QPalette::Window, QColor(47,125,212));
	m_ui->frameSideA->setPalette(p);
	p.setColor(QPalette::Window, QColor(255, 206, 48));
	m_ui->frameSideB->setPalette(p);

	m_ui->comboBoxComponentSelection->setEnabled(false); // since "All components" is initially checked, we disable the combo box
}


SVPropBuildingComponentOrientationWidget::~SVPropBuildingComponentOrientationWidget() {
	delete m_ui;
}


void SVPropBuildingComponentOrientationWidget::updateUi() {
	m_ui->comboBoxComponentSelection->blockSignals(true);
	m_ui->comboBoxComponentSelection->clear();

	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true); // visible (selected and not selected)

	m_selectedComponentInstances.clear();
	// now build a map of component IDs versus visible surfaces
	std::map<const VICUS::Component *, std::set<const VICUS::Surface *> > componentSurfaceMap;
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// lookup component in DB
		const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_idComponent];

		bool selectedAndVisible = false;

		// side A
		if (ci.m_sideASurface != nullptr) {
			// is this surface visible?
			std::set<const VICUS::Object * >::const_iterator it_A = objs.find(ci.m_sideASurface);
			if (it_A != objs.end()) {
				componentSurfaceMap[comp].insert(ci.m_sideASurface);
				selectedAndVisible = ci.m_sideASurface->m_visible && ci.m_sideASurface->m_selected;
			}
		}
		// side B
		if (ci.m_sideBSurface != nullptr) {
			std::set<const VICUS::Object * >::const_iterator it_B = objs.find(ci.m_sideBSurface);
			if (it_B != objs.end()) {
				componentSurfaceMap[comp].insert(ci.m_sideBSurface);
				if (ci.m_sideBSurface->m_selected)
					selectedAndVisible = ci.m_sideBSurface->m_visible && ci.m_sideBSurface->m_selected;
			}
		}
		if (selectedAndVisible)
			m_selectedComponentInstances.push_back(&ci);
	}

	// populate combo box
	for (std::map<const VICUS::Component*, std::set<const VICUS::Surface *> >::const_iterator
		 it = componentSurfaceMap.begin(); it != componentSurfaceMap.end(); ++it)
	{
		if (it->first == nullptr)
			continue;
		m_ui->comboBoxComponentSelection->addItem(QtExt::MultiLangString2QString(it->first->m_displayName), it->first->m_id);
	}
	m_ui->comboBoxComponentSelection->setCurrentIndex(m_ui->comboBoxComponentSelection->count()-1);
	m_ui->comboBoxComponentSelection->blockSignals(false);

	// selection-related info
	if (m_selectedComponentInstances.empty()) {
		m_ui->labelComponentOrientationInfo->setText(tr("No surfaces with components selected"));
		m_ui->pushButtonAlignComponentToSideA->setEnabled(false);
		m_ui->pushButtonAlignComponentToSideB->setEnabled(false);
	}
	else {
		m_ui->labelComponentOrientationInfo->setText(tr("%1 surfaces with components selected").arg(m_selectedComponentInstances.size()));
		m_ui->pushButtonAlignComponentToSideA->setEnabled(true);
		m_ui->pushButtonAlignComponentToSideB->setEnabled(true);
	}

}



void SVPropBuildingComponentOrientationWidget::on_checkBoxShowAllComponentOrientations_toggled(bool checked) {
	m_ui->labelComponentSelection->setEnabled(!checked);
	m_ui->comboBoxComponentSelection->setEnabled(!checked);
	if (checked) {
		SVViewState vs = SVViewStateHandler::instance().viewState();
		vs.m_colorModePropertyID = VICUS::INVALID_ID; // disable filter
		SVViewStateHandler::instance().setViewState(vs);
	}
	else {
		on_comboBoxComponentSelection_currentIndexChanged(m_ui->comboBoxComponentSelection->currentIndex());
	}
}


void SVPropBuildingComponentOrientationWidget::on_pushButtonAlignComponentToSideA_clicked() {
	alignSelectedComponents(true);
}


void SVPropBuildingComponentOrientationWidget::on_pushButtonAlignComponentToSideB_clicked() {
	alignSelectedComponents(false);
}


void SVPropBuildingComponentOrientationWidget::on_comboBoxComponentSelection_currentIndexChanged(int index) {
	// set index/id of selected component in view manager and update coloring
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_colorModePropertyID = m_ui->comboBoxComponentSelection->currentData().toUInt();
	SVViewStateHandler::instance().setViewState(vs);
	m_ui->comboBoxComponentSelection->blockSignals(true);
	m_ui->comboBoxComponentSelection->setCurrentIndex(index);
	m_ui->comboBoxComponentSelection->blockSignals(false);
}



void SVPropBuildingComponentOrientationWidget::alignSelectedComponents(bool toSideA) {
	// create a copy of the component instances
	std::vector<VICUS::ComponentInstance> compInstances = project().m_componentInstances;

	std::set<unsigned int> surfacesToDDeselect;
	// loop over all components and look for a selected side - if there is more than one side of a component
	// instance selected, show an error message
	for (const VICUS::ComponentInstance * c : m_selectedComponentInstances) {
		// both sides selected?
		bool sideASelected = (c->m_sideASurface != nullptr && c->m_sideASurface->m_selected);
		bool sideBSelected = (c->m_sideBSurface != nullptr && c->m_sideBSurface->m_selected);
		if (sideASelected && sideBSelected) {
			QMessageBox::critical(this, QString(), tr("You must not select both surfaces of the same component!"));
			return;
		}
		// now lookup copied componentInstance by ID and swap sides if:
		// - the selected side is side B and should be switched to side A
		// - the selected side is side A and should be switched to side B
		if ((toSideA && sideBSelected) ||
			(!toSideA && sideASelected))
		{
			std::vector<VICUS::ComponentInstance>::iterator it = std::find(compInstances.begin(), compInstances.end(), c->m_id);
			Q_ASSERT(it != compInstances.end());

			if (sideASelected && it->m_sideASurface != nullptr )
				surfacesToDDeselect.insert(it->m_sideASurface->m_id);

			if (sideBSelected && it->m_sideBSurface != nullptr )
				surfacesToDDeselect.insert(it->m_sideBSurface->m_id);

			std::swap(it->m_idSideASurface, it->m_idSideBSurface);
		}
	}

	// if there was no change, inform the user and abort
	if (surfacesToDDeselect.empty()) {
		QMessageBox::information(this, QString(), tr("All of the selected surfaces are already aligned as requested."));
		return;
	}

	// compose undo action
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Aligning component sides"), compInstances);
	undo->push(); // Note: this invalidates all pointers in the project, calls onModified() and rebuilds our maps

	// now deselect all surfaces that have been touched
	SVUndoTreeNodeState * undoSelect = new SVUndoTreeNodeState(tr("Deselecting modified surfaces"),
															   SVUndoTreeNodeState::SelectedState, surfacesToDDeselect, false);
	undoSelect->push();
}
