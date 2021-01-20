#include "SVPropModeSelectionWidget.h"
#include "ui_SVPropModeSelectionWidget.h"

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVUndoSiteDataChanged.h"

SVPropModeSelectionWidget::SVPropModeSelectionWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropModeSelectionWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);
	updateProperties();
}


SVPropModeSelectionWidget::~SVPropModeSelectionWidget() {
	delete m_ui;
}


void SVPropModeSelectionWidget::updateUI() {
	// transfer data to user interface elements
	m_ui->comboBoxNetwork->blockSignals(true);
	// store network names
	int currentIdx = m_ui->comboBoxNetwork->currentIndex();
	m_ui->comboBoxNetwork->clear();
	for (const VICUS::Network & n : project().m_geometricNetworks) {
		m_ui->comboBoxNetwork->addItem(QString::fromStdString(n.m_name));
	}
	if (currentIdx == -1)
		currentIdx = 0;
	if (m_ui->comboBoxNetwork->count() > currentIdx)
		currentIdx = m_ui->comboBoxNetwork->count()-1;
	m_ui->comboBoxNetwork->setCurrentIndex(currentIdx);
	m_ui->comboBoxNetwork->blockSignals(false);
}


void SVPropModeSelectionWidget::on_pushButtonBuilding_toggled(bool) {
	updateProperties();
	// emit a signal with the information about the changed input
	emit buildingPropertiesSelected(m_ui->comboBoxBuildingProperties->currentIndex());
}


void SVPropModeSelectionWidget::on_pushButtonNetwork_toggled(bool) {
	updateProperties();
	// emit a signal with the information about the changed input
	emit networkPropertiesSelected(m_ui->comboBoxNetwork->currentIndex(), m_ui->comboBoxNetworkProperties->currentIndex());
}


void SVPropModeSelectionWidget::on_pushButtonSite_toggled(bool) {
	updateProperties();
	// emit a signal with the information about the changed input
	emit sitePropertiesSelected();
}


void SVPropModeSelectionWidget::updateProperties() {
	bool showBuildingProps = m_ui->pushButtonBuilding->isChecked();
	bool showNetworkProps = m_ui->pushButtonNetwork->isChecked();
	m_ui->labelNetwork->setVisible(showNetworkProps);
	m_ui->labelNetworkProperties->setVisible(showNetworkProps);
	m_ui->comboBoxNetworkProperties->setVisible(showNetworkProps);
	m_ui->comboBoxNetwork->setVisible(showNetworkProps);

	m_ui->labelBuildingProperties->setVisible(showBuildingProps);
	m_ui->comboBoxBuildingProperties->setVisible(showBuildingProps);
}

