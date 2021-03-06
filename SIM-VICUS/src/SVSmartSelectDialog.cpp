/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVSmartSelectDialog.h"
#include "ui_SVSmartSelectDialog.h"

#include <QPushButton>
#include <QDialogButtonBox>

#include <VICUS_KeywordList.h>
#include <VICUS_NetworkNode.h>
#include <VICUS_Project.h>

#include <QtExt_Conversions.h>

#include "SVProjectHandler.h"
#include "SVUndoTreeNodeState.h"
#include "SVDatabase.h"
#include "SVSettings.h"

SVSmartSelectDialog::SVSmartSelectDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSmartSelectDialog)
{
	m_ui->setupUi(this);

	m_ui->verticalLayoutNetwork->setContentsMargins(0,6,0,0);
	m_ui->gridLayoutBuildings->setContentsMargins(0,6,0,0);

	QPushButton * btn = new QPushButton(tr("Select"));
	m_ui->buttonBox->addButton(btn, QDialogButtonBox::NoRole);
	connect(btn, &QPushButton::clicked, this, &SVSmartSelectDialog::onSelectClicked);

	// populate combo boxes

	m_ui->comboBoxNetwork->clear();
	m_ui->comboBoxNetwork->addItem(tr("<all networks>"), -1);
	for (const VICUS::Network & nw : project().m_geometricNetworks)
		m_ui->comboBoxNetwork->addItem(nw.m_displayName, nw.m_id);
	m_ui->comboBoxNetwork->setCurrentIndex(0);

	m_ui->comboBoxNodeType->blockSignals(true);
	m_ui->comboBoxNodeType->clear();
	m_ui->comboBoxNodeType->addItem(tr("<all types>"));
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Mixer),
									VICUS::NetworkNode::NT_Mixer);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Source),
									VICUS::NetworkNode::NT_Source);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Building),
									VICUS::NetworkNode::NT_Building);
	m_ui->comboBoxNodeType->blockSignals(false);
	m_ui->comboBoxNodeType->setCurrentIndex(0);

	m_ui->checkBoxMaxHeatingDemandBelow->setChecked(false);
	m_ui->checkBoxMaxHeatingDemandAbove->setChecked(false);
	m_ui->checkBoxLengthBelow->setChecked(false);
	m_ui->checkBoxLengthAbove->setChecked(false);

	m_ui->lineEditMaxHeatingDemandAbove->setEnabled(false);
	m_ui->lineEditMaxHeatingDemandBelow->setEnabled(false);
	m_ui->lineEditLengthAbove->setEnabled(false);
	m_ui->lineEditLengthBelow->setEnabled(false);

	// populate static options
	m_options.m_options.resize(2);

	FilterOption & components = m_options.m_options[0];
	components.m_name = tr("Components");
	components.m_options.resize(3);

	components.m_options[0].m_name = tr("Outside walls");
	components.m_options[1].m_name = tr("Inside walls");
	components.m_options[2].m_name = tr("Windows");

	FilterOption & thermalElements = m_options.m_options[1];
	thermalElements.m_name = tr("Thermal elements");

	delete m_ui->pushButtonDummy;
}


SVSmartSelectDialog::~SVSmartSelectDialog() {
	qDeleteAll(m_selectionButtons);
	qDeleteAll(m_optionButtons);
	delete m_ui;
}


void SVSmartSelectDialog::select() {

	// populate dynamic elements
	FilterOption & outsideWalls = m_options.m_options[0].m_options[0];
	outsideWalls.m_options.clear();
	FilterOption & insideWalls = m_options.m_options[0].m_options[1];
	insideWalls.m_options.clear();
	FilterOption & windows = m_options.m_options[0].m_options[2];
	windows.m_options.clear();

	// process data structure and populate options
	const SVDatabase & db = SVSettings::instance().m_db;

	std::set<const VICUS::Component *> components;
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances)
		components.insert(db.m_components[ci.m_componentID]);
	for (const VICUS::Component * comp : components) {
		if (comp == nullptr) continue;
		switch (comp->m_type) {
			case VICUS::Component::CT_OutsideWall:
				outsideWalls.m_options.push_back(
							FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_OutsideWallToGround:
				outsideWalls.m_options.push_back(
							FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_InsideWall:
				insideWalls.m_options.push_back(
							FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_FloorToCellar:
			break;
			case VICUS::Component::CT_FloorToAir:
			break;
			case VICUS::Component::CT_FloorToGround:
			break;
			case VICUS::Component::CT_Ceiling:
			break;
			case VICUS::Component::CT_SlopedRoof:
			break;
			case VICUS::Component::CT_FlatRoof:
			break;
			case VICUS::Component::CT_ColdRoof:
			break;
			case VICUS::Component::CT_WarmRoof:
			break;
			case VICUS::Component::CT_Miscellaneous:
			break;
			case VICUS::Component::NUM_CT: break;
		}
	}

	std::set<const VICUS::SubSurfaceComponent *> subSurfaceComponents;
	for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances)
		subSurfaceComponents.insert(db.m_subSurfaceComponents[ci.m_subSurfaceComponentID]);
	for (const VICUS::SubSurfaceComponent * comp : subSurfaceComponents) {
		if (comp == nullptr) continue;
		switch (comp->m_type) {
			case VICUS::SubSurfaceComponent::CT_Window:
				windows.m_options.push_back(
							FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::SubSurfaceComponent::CT_Door:
			break;
			case VICUS::SubSurfaceComponent::CT_Miscellaneous:
			break;
			case VICUS::SubSurfaceComponent::NUM_CT:
			break;
		}
	}

	updateButtonGrid();

	// everything else is done inside the dialog
	exec();
}


void SVSmartSelectDialog::onSelectClicked() {
	// evaluate selection and create undo action for the selection
	if (m_ui->tabWidgetGroup->currentIndex() == 0) {
		// do we have a component selected?
		FilterOption * option = &m_options;
		for (unsigned int i=0; i<m_selections.size(); ++i) {
			unsigned int selectedIndex = m_selections[i];
			if (selectedIndex >= option->m_options.size())
				break; // not available, skip
			option = &option->m_options[selectedIndex];
		}
		if (option->m_dbElement == nullptr) {
			QMessageBox::critical(this, QString(), tr("Not a valid filter selection!"));
			return;
		}
		// now determine all surfaces that are associated with this component
		// TODO :
	}
	else {

		// Network selections
		int filterNetworkId = m_ui->comboBoxNetwork->currentData().toInt();

		// node selections
		if (m_ui->tabWidgetNetworkSelections->currentIndex()==0) {

			VICUS::NetworkNode::NodeType nodeTypeFilter = VICUS::NetworkNode::NUM_NT; // no filter on node type
			if (m_ui->comboBoxNodeType->currentIndex() != 0)
				nodeTypeFilter = (VICUS::NetworkNode::NodeType)m_ui->comboBoxNodeType->currentData().toInt();

			// heating demand must be below value
			double maxHeatingDemandBelowFilter = std::numeric_limits<double>::max();
			if (m_ui->checkBoxMaxHeatingDemandBelow->isChecked() && m_ui->lineEditMaxHeatingDemandBelow->isValid())
				maxHeatingDemandBelowFilter = m_ui->lineEditMaxHeatingDemandBelow->value();

			// heating demand must be above value
			double maxHeatingDemandAboveFilter = std::numeric_limits<double>::lowest();
			if (m_ui->checkBoxMaxHeatingDemandAbove->isChecked() && m_ui->lineEditMaxHeatingDemandAbove->isValid())
				maxHeatingDemandAboveFilter = m_ui->lineEditMaxHeatingDemandAbove->value();

			// now process all network objects and store IDs of selected nodes
			std::set<unsigned int> nodeIDs;

			for (const VICUS::Network & nw : project().m_geometricNetworks) {

				// filter networks
				if (filterNetworkId != -1 && filterNetworkId != (int)nw.m_id)
					continue;

				for (const VICUS::NetworkNode & n : nw.m_nodes) {
				// apply filter rules

					// filter based on type
					if (nodeTypeFilter != VICUS::NetworkNode::NUM_NT && n.m_type != nodeTypeFilter)
						continue;

					if (n.m_maxHeatingDemand.value > maxHeatingDemandBelowFilter)
						continue;

					if (n.m_maxHeatingDemand.value < maxHeatingDemandAboveFilter)
						continue;

					// all filters match, store unique ID of this node
					nodeIDs.insert(n.uniqueID());
				}
			}

			SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selecting nodes"), SVUndoTreeNodeState::SelectedState, nodeIDs, true);
			undo->push();
		}
		else {

			// length must be below value
			double lengthBelowFilter = std::numeric_limits<double>::max();
			if (m_ui->checkBoxLengthBelow->isChecked() && m_ui->lineEditLengthBelow->isValid())
				lengthBelowFilter = m_ui->lineEditLengthBelow->value();

			// length must be above value
			double lengthAboveFilter = std::numeric_limits<double>::lowest();
			if (m_ui->checkBoxLengthAbove->isChecked() && m_ui->lineEditLengthAbove->isValid())
				lengthAboveFilter = m_ui->lineEditLengthAbove->value();

			std::set<unsigned int> edgeIDs;

			for (const VICUS::Network & nw : project().m_geometricNetworks) {

				// filter networks
				if (filterNetworkId != -1 && filterNetworkId != (int)nw.m_id)
					continue;

				for (const VICUS::NetworkEdge & edge : nw.m_edges) {

					if (edge.length() > lengthBelowFilter)
						continue;

					if (edge.length() < lengthAboveFilter)
						continue;

					// all filters match, store unique ID of this node
					edgeIDs.insert(edge.uniqueID());
				}
			}

			SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selecting edges"), SVUndoTreeNodeState::SelectedState, edgeIDs, true);
			undo->push();
		}
	}

	// when this even is done, close dialog automatically
	close();
}


void SVSmartSelectDialog::on_comboBoxNodeType_currentIndexChanged(int index)
{
	bool noBuilding = index==1 || index==2;
	m_ui->checkBoxMaxHeatingDemandAbove->setEnabled(!noBuilding);
	m_ui->checkBoxMaxHeatingDemandBelow->setEnabled(!noBuilding);
	if (noBuilding){
		m_ui->checkBoxMaxHeatingDemandBelow->setChecked(false);
		m_ui->checkBoxMaxHeatingDemandAbove->setChecked(false);
	}
}


void SVSmartSelectDialog::on_checkBoxMaxHeatingDemandBelow_stateChanged(int arg1) {
	m_ui->lineEditMaxHeatingDemandBelow->setEnabled(arg1);
}


void SVSmartSelectDialog::on_checkBoxMaxHeatingDemandAbove_stateChanged(int arg1) {
	m_ui->lineEditMaxHeatingDemandAbove->setEnabled(arg1);
}


void SVSmartSelectDialog::on_checkBoxLengthBelow_stateChanged(int arg1) {
	m_ui->lineEditLengthBelow->setEnabled(arg1);
}


void SVSmartSelectDialog::on_checkBoxLengthAbove_stateChanged(int arg1) {
	m_ui->lineEditLengthAbove->setEnabled(arg1);
}


void SVSmartSelectDialog::on_pushButtonReset_clicked() {
	// reset current hierarchy level to 0
	m_selections.clear();
	updateButtonGrid();
}


void SVSmartSelectDialog::onSelectionButtonPressed() {
	QPushButton * btn = qobject_cast<QPushButton *>(sender());
	Q_ASSERT(btn != nullptr);

	// determine level and reduce selection accordingly
	unsigned int buttonLevel = 0;
	for (;buttonLevel < m_selectionButtons.size(); ++buttonLevel)
		if (m_selectionButtons[buttonLevel] == btn)
			break;

	if (buttonLevel+1 == m_selectionButtons.size()) {
		m_selectionButtons.back()->setDown(true);
		return;
	}
	m_selections.resize(buttonLevel+1);
	updateButtonGrid();
}


void SVSmartSelectDialog::onOptionButtonPressed() {
	QPushButton * btn = qobject_cast<QPushButton *>(sender());
	Q_ASSERT(btn != nullptr);

	// determine level and reduce selection accordingly
	unsigned int buttonLevel = 0;
	for (;buttonLevel < m_optionButtons.size(); ++buttonLevel)
		if (m_optionButtons[buttonLevel] == btn)
			break;

	// append selection
	m_selections.push_back(buttonLevel);
	updateButtonGrid();
}


void SVSmartSelectDialog::updateButtonGrid() {
	// Mind: it may be the case that we cannot follow all selections through, if
	//       dynamic options have disappeared sind last call

	qDeleteAll(m_selectionButtons);
	m_selectionButtons.clear();

	FilterOption * option = &m_options;
	std::vector<unsigned int> validSelections;
	for (unsigned int i=0; i<m_selections.size(); ++i) {
		unsigned int selectedIndex = m_selections[i];
		if (selectedIndex >= option->m_options.size())
			break; // not available, skip
		validSelections.push_back(selectedIndex);
		option = &option->m_options[selectedIndex];
		// create push button
		QPushButton * button = new QPushButton(option->m_name);
		// connect clicked signal
		connect(button, &QPushButton::clicked, this, &SVSmartSelectDialog::onSelectionButtonPressed);
		m_ui->horizontalLayoutBreadCrums->insertWidget(m_ui->horizontalLayoutBreadCrums->count()-1, button);
		m_selectionButtons.push_back(button);
	}
	m_selections.swap(validSelections);

	if (!m_selectionButtons.empty()) {
		m_selectionButtons.back()->setDown(true);
	}

	// now update button grid
	qDeleteAll(m_optionButtons);
	m_optionButtons.clear();

	// based on the currently selected hierarchy level 'option', populate buttons
	for (FilterOption & o : option->m_options) {
		QPushButton * button = new QPushButton(o.m_name);
		connect(button, &QPushButton::clicked, this, &SVSmartSelectDialog::onOptionButtonPressed);
		m_ui->verticalLayoutOptions->insertWidget(m_ui->verticalLayoutOptions->count()-1, button);
		m_optionButtons.push_back(button);
	}

}
