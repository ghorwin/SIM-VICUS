#include "SVPropModeSelectionWidget.h"
#include "ui_SVPropModeSelectionWidget.h"

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVUndoSiteDataChanged.h"
#include "SVConstants.h"

SVPropModeSelectionWidget::SVPropModeSelectionWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropModeSelectionWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->comboBoxBuildingProperties->blockSignals(true);
	m_ui->comboBoxBuildingProperties->addItem(tr("Component"), BT_Components);
	m_ui->comboBoxBuildingProperties->addItem(tr("Construction orientation"), BT_ComponentOrientation);
	m_ui->comboBoxBuildingProperties->addItem(tr("Boundary conditions"), BT_BoundaryConditions);
	m_ui->comboBoxBuildingProperties->blockSignals(false);

	// we are in "Site" mode initially, so the rest is hidden
	updateWidgetVisibility();
}


SVPropModeSelectionWidget::~SVPropModeSelectionWidget() {
	delete m_ui;
}


void SVPropModeSelectionWidget::selectionChanged() {
	if (m_ui->pushButtonNetwork->isChecked()) {
		// now check the selected objects and if we have only nodes - go to node edit more,
		// if we have only edges - go to edge edit mode

		std::set<const VICUS::Object*> objs;
		project().selectObjects(objs, VICUS::Project::SG_Network, true, true);
		bool haveNode = false;
		bool haveEdge = false;
		for (const VICUS::Object* o : objs) {
			if (dynamic_cast<const VICUS::NetworkNode*>(o) != nullptr)
				haveNode = true;
			else if (dynamic_cast<const VICUS::NetworkEdge*>(o) != nullptr) {
				haveEdge = true;
			}
			if (haveNode && haveEdge)
				return; // both selected, cannot do anything
		}
		// only nodes selected?
		if (haveNode && !haveEdge) {
			if (m_ui->comboBoxNetworkProperties->currentIndex() == 1)
				emit networkPropertiesSelected(1);
			else
				m_ui->comboBoxNetworkProperties->setCurrentIndex(1);	// sends a signal to change property widget
		}
		if (!haveNode && haveEdge) {
			if (m_ui->comboBoxNetworkProperties->currentIndex() == 2)
				emit networkPropertiesSelected(2);
			else
				m_ui->comboBoxNetworkProperties->setCurrentIndex(2);	// sends a signal to change property widget

		}

	}

	// TODO : what about building editing default mode?

}


SVViewState::PropertyWidgetMode SVPropModeSelectionWidget::currentPropertyWidgetMode() const {
	if (m_ui->pushButtonBuilding->isChecked())
		return SVViewState::PM_BuildingProperties;
	if (m_ui->pushButtonNetwork->isChecked())
		return SVViewState::PM_NetworkProperties;
	return SVViewState::PM_SiteProperties;
}


void SVPropModeSelectionWidget::on_pushButtonBuilding_toggled(bool on) {
	if (!on) return;
	updateWidgetVisibility();
	blockSignals(true);
	selectionChanged();
	blockSignals(false);
	// emit a signal with the information about the changed input
	emit buildingPropertiesSelected(m_ui->comboBoxBuildingProperties->currentData().toInt());
}


void SVPropModeSelectionWidget::on_pushButtonNetwork_toggled(bool on) {
	if (!on) return;
	updateWidgetVisibility();
	blockSignals(true);
	selectionChanged();
	blockSignals(false);
	// emit a signal with the information about the changed input
	emit networkPropertiesSelected(m_ui->comboBoxNetworkProperties->currentIndex());
}


void SVPropModeSelectionWidget::on_pushButtonSite_toggled(bool on) {
	if (!on) return;
	updateWidgetVisibility();
	blockSignals(true);
	selectionChanged();
	blockSignals(false);
	// emit a signal with the information about the changed input
	emit sitePropertiesSelected();
}


void SVPropModeSelectionWidget::updateWidgetVisibility() {
	bool showBuildingProps = m_ui->pushButtonBuilding->isChecked();
	bool showNetworkProps = m_ui->pushButtonNetwork->isChecked();
	m_ui->labelNetworkProperties->setVisible(showNetworkProps);
	m_ui->comboBoxNetworkProperties->setVisible(showNetworkProps);

	m_ui->labelBuildingProperties->setVisible(showBuildingProps);
	m_ui->comboBoxBuildingProperties->setVisible(showBuildingProps);
}


void SVPropModeSelectionWidget::on_comboBoxNetworkProperties_currentIndexChanged(int) {
	emit networkPropertiesSelected(m_ui->comboBoxNetworkProperties->currentData().toInt());
}


void SVPropModeSelectionWidget::on_comboBoxBuildingProperties_currentIndexChanged(int index) {
	emit buildingPropertiesSelected(index);
}
