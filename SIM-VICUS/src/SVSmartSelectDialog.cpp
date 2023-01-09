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
#include <QListWidgetItem>

#include <VICUS_KeywordList.h>
#include <VICUS_NetworkNode.h>
#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include "SVConversions.h"
#include "SVProjectHandler.h"
#include "SVUndoTreeNodeState.h"
#include "SVDatabase.h"
#include "SVSettings.h"


SVSmartSelectDialog::SVSmartSelectDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSmartSelectDialog)
{
	m_ui->setupUi(this);

	m_arrowRight = QPixmap(":/gfx/actions/24x24/arrow-right-grey.png");

	m_ui->verticalLayoutNetwork->setContentsMargins(0,6,0,0);
	m_ui->gridLayoutBuildings->setContentsMargins(0,6,0,0);
	m_ui->labelFilterInfo->setText("");

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
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_SubStation),
									VICUS::NetworkNode::NT_SubStation);
	m_ui->comboBoxNodeType->blockSignals(false);
	m_ui->comboBoxNodeType->setCurrentIndex(0);

	m_ui->checkBoxMaxHeatingDemandBelow->setChecked(false);
	m_ui->checkBoxMaxHeatingDemandAbove->setChecked(false);
	m_ui->checkBoxLengthBelow->setChecked(false);
	m_ui->checkBoxLengthAbove->setChecked(false);
	m_ui->checkBoxNodeId->setChecked(false);
	m_ui->checkBoxNodeDisplayName->setChecked(false);
	m_ui->checkBoxMaxHeatingDemandEqualTo->setChecked(false);


	// populate static options
	m_allOptions = FilterOption();
	m_allOptions.m_options.resize(NUM_ET);

	// Components
	FilterOption & components = m_allOptions.m_options[ET_Component];
	components.m_name = tr("Components");
	for (int i=0; i<VICUS::Component::NUM_CT; ++i) {
		std::string name = VICUS::KeywordList::Keyword("Component::ComponentType", i);
		components.m_options.push_back(FilterOption(VICUS::camelCase2ReadableString(name), nullptr));
	}
	components.m_options.push_back(FilterOption("Invalid Component", nullptr));

	// Subsurfaces
	FilterOption & surf = m_allOptions.m_options[ET_SubSurface];
	surf.m_name = tr("Sub Surfaces");
	for (int i=0; i<VICUS::SubSurfaceComponent::NUM_CT; ++i) {
		std::string name = VICUS::KeywordList::Keyword("SubSurfaceComponent::SubSurfaceComponentType", i);
		surf.m_options.push_back(FilterOption(VICUS::camelCase2ReadableString(name), nullptr));
	}
	surf.m_options.push_back(FilterOption("Invalid sub surface", nullptr));

	// Geometry
	FilterOption & geometry = m_allOptions.m_options[ET_Geometry];
	geometry.m_name = tr("Geometry");
	geometry.m_options.push_back(FilterOption(tr("Surfaces"), nullptr));
	geometry.m_options.push_back(FilterOption(tr("Sub-Surfaces"), nullptr));


	// configure list widget
	m_ui->listWidgetOptions->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_ui->listWidgetOptions->setAlternatingRowColors(true);

	// check box & combobox boundary condition
	m_ui->checkBoxFilterBoundaryCondition->setChecked(false);
	m_ui->comboBoxBoundaryCondition->setEnabled(false);
}


SVSmartSelectDialog::~SVSmartSelectDialog() {
	qDeleteAll(m_selectionButtons);
	qDeleteAll(m_optionButtons);
	delete m_ui;
}


void SVSmartSelectDialog::select() {

	// process data structure and populate options

	const SVDatabase & db = SVSettings::instance().m_db;

	// populate boundary condition combobox
	std::map<unsigned int, QString> boundaryConditionNames;
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		const VICUS::Component *c = db.m_components[ci.m_idComponent];
		if (c != nullptr) {
			const VICUS::BoundaryCondition *bcA =  db.m_boundaryConditions[c->m_idSideABoundaryCondition];
			if (bcA != nullptr) {
				boundaryConditionNames[bcA->m_id] = QtExt::MultiLangString2QString(bcA->m_displayName);
			}
			const VICUS::BoundaryCondition *bcB =  db.m_boundaryConditions[c->m_idSideBBoundaryCondition];
			if (bcB != nullptr) {
				boundaryConditionNames[bcB->m_id] = QtExt::MultiLangString2QString(bcB->m_displayName);
			}
		}
	}
	m_ui->comboBoxBoundaryCondition->clear();
	for (auto it=boundaryConditionNames.begin(); it!=boundaryConditionNames.end(); ++it)
		m_ui->comboBoxBoundaryCondition->addItem(it->second, it->first);

	// clear existing options
	for (int i=0; i<=VICUS::Component::NUM_CT; ++i) // NUM_CT also exists!
		m_allOptions.m_options[ET_Component].m_options[VICUS::Component::ComponentType(i)].m_options.clear();
	for (int i=0; i<=VICUS::SubSurfaceComponent::NUM_CT; ++i) // NUM_CT also exists!
		m_allOptions.m_options[ET_SubSurface].m_options[VICUS::SubSurfaceComponent::SubSurfaceComponentType(i)].m_options.clear();


	// populate components

	std::set<const VICUS::Component *> components;
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances)
		components.insert(db.m_components[ci.m_idComponent]);

	for (const VICUS::Component * comp : components) {

		if (comp == nullptr) {
			m_allOptions.m_options[ET_Component].m_options[VICUS::Component::NUM_CT].m_options
					.push_back(FilterOption("Invalid Component", nullptr) );
		continue;
		}

		switch (comp->m_type) {
			case VICUS::Component::CT_OutsideWall:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_OutsideWall].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_OutsideWallToGround:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_OutsideWallToGround].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_InsideWall:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_InsideWall].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_FloorToCellar:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_FloorToCellar].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_FloorToAir:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_FloorToAir].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_FloorToGround:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_FloorToGround].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_Ceiling:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_Ceiling].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_SlopedRoof:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_SlopedRoof].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_FlatRoof:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_FlatRoof].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_ColdRoof:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_ColdRoof].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_WarmRoof:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_WarmRoof].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::CT_Miscellaneous:
				m_allOptions.m_options[ET_Component].m_options[VICUS::Component::CT_Miscellaneous].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::Component::NUM_CT:
			break;
		}
	}

	// populate subsurfaces

	std::set<const VICUS::SubSurfaceComponent *> subSurfaceComponents;
	for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances)
		subSurfaceComponents.insert(db.m_subSurfaceComponents[ci.m_idSubSurfaceComponent]);

	for (const VICUS::SubSurfaceComponent * comp : subSurfaceComponents) {
		if (comp == nullptr) {
			m_allOptions.m_options[ET_SubSurface].m_options[VICUS::SubSurfaceComponent::NUM_CT].m_options
					.push_back(FilterOption("Invalid sub surface", nullptr) );
		continue;
		}
		switch (comp->m_type) {
			case VICUS::SubSurfaceComponent::CT_Window:
				m_allOptions.m_options[ET_SubSurface].m_options[VICUS::SubSurfaceComponent::CT_Window].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::SubSurfaceComponent::CT_Door:
				m_allOptions.m_options[ET_SubSurface].m_options[VICUS::SubSurfaceComponent::CT_Door].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::SubSurfaceComponent::CT_Miscellaneous:
				m_allOptions.m_options[ET_SubSurface].m_options[VICUS::SubSurfaceComponent::CT_Miscellaneous].m_options
						.push_back(FilterOption(QtExt::MultiLangString2QString(comp->m_displayName), comp) );
			break;
			case VICUS::SubSurfaceComponent::NUM_CT:
			break;
		}
	}

	updateButtonsAndListWidget();

	// everything else is done inside the dialog
	exec();
}



void SVSmartSelectDialog::onSelectClicked() {

	// evaluate selection and create undo action for the selection
	if (m_ui->tabWidgetGroup->currentIndex() == 0)
		selectBuildingComponents();
	else
		selectNetworkComponents();

	// when this even is done, close dialog automatically
	close();
}


void SVSmartSelectDialog::selectBuildingComponents() {

	// do we have a component selected?
	FilterOption * option = &m_allOptions;
	for (unsigned int i=0; i<m_selectionIndex.size(); ++i) {
		unsigned int selectedIndex = m_selectionIndex[i];
		if (selectedIndex >= option->m_options.size())
			break; // not available, skip
		option = &option->m_options[selectedIndex];
	}

	// filter for selection (an empty filter means no filtering)
	std::set<QString> filterList;
	const auto &selectedItems = m_ui->listWidgetOptions->selectedItems();
	for (const QListWidgetItem *item: selectedItems) {
		filterList.insert(item->text());
	}

	// filter boundary condition
	int boundaryConditionFilterId = -1;
	if (m_ui->checkBoxFilterBoundaryCondition->isChecked())
		boundaryConditionFilterId = m_ui->comboBoxBoundaryCondition->currentData().toInt();

	std::set<const VICUS::Object*> selectedObjects;
	for (FilterOption &option: option->m_options) {
		if (filterList.empty() || filterList.find(option.m_name) != filterList.end())
			collectSelectedObjects(&option, selectedObjects, boundaryConditionFilterId);
	}

	if (selectedObjects.empty())
		QMessageBox::information(this, QString(), tr("No objects found with given properties."));

	std::set<unsigned int> nodeIDs;
	for (const VICUS::Object* o : selectedObjects)
		nodeIDs.insert(o->m_id);

	// create an undo-action with the selected
	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selecting objects"), SVUndoTreeNodeState::SelectedState, nodeIDs, true);
	undo->push();
}



void SVSmartSelectDialog::collectSelectedObjects(FilterOption * option, std::set<const VICUS::Object*> & objs, int boundaryConditionFilterId)  {

	// if we have child leaves, process these
	if (!option->m_options.empty()) {
		for (FilterOption & o : option->m_options) {
			collectSelectedObjects(&o, objs, boundaryConditionFilterId);
		}
	}
	else {

		// we have a child node, now determine the type
		const VICUS::Component * componentToLookFor = dynamic_cast<const VICUS::Component *>(option->m_dbElement);
		// Note: it is intended that componentToLookFor can be a nullptr
		// now lookup all component instances that make use of this component
		for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
			VICUS::Component *comp = SVSettings::instance().m_db.m_components[ci.m_idComponent];

			if (comp == componentToLookFor) { // could be that both are nullptr

				// filter boundary condition
				if (comp !=nullptr) {
					if ( boundaryConditionFilterId > 0 &&
						comp->m_idSideABoundaryCondition != (unsigned int)boundaryConditionFilterId &&
						comp->m_idSideBBoundaryCondition != (unsigned int)boundaryConditionFilterId )
						continue;
				}

				// look up referenced surfaces
				if (ci.m_sideASurface != nullptr && ci.m_sideASurface->m_visible)
					objs.insert(ci.m_sideASurface);
				if (ci.m_sideBSurface != nullptr && ci.m_sideBSurface->m_visible)
					objs.insert(ci.m_sideBSurface);
			}
		}


		const VICUS::SubSurfaceComponent * sscToLookFor = dynamic_cast<const VICUS::SubSurfaceComponent *>(option->m_dbElement);
		// Note: it is intended that sscToLookFor can be a nullptr
		// now lookup all component instances that make use of this component
		for (const VICUS::SubSurfaceComponentInstance & ssci : project().m_subSurfaceComponentInstances) {
			VICUS::SubSurfaceComponent *ssc = SVSettings::instance().m_db.m_subSurfaceComponents[ssci.m_idSubSurfaceComponent];

			if (ssc == sscToLookFor) { // could be that both are nullptr

				// filter boundary condition
				if (ssc !=nullptr) {
					if ( boundaryConditionFilterId > 0 &&
						ssc->m_idSideABoundaryCondition != (unsigned int)boundaryConditionFilterId &&
						ssc->m_idSideBBoundaryCondition != (unsigned int)boundaryConditionFilterId )
						continue;
				}

				// look up referenced surfaces
				if (ssci.m_sideASubSurface != nullptr && ssci.m_sideASubSurface->m_visible)
					objs.insert(ssci.m_sideASubSurface);
				if (ssci.m_sideBSubSurface != nullptr && ssci.m_sideBSubSurface->m_visible)
					objs.insert(ssci.m_sideBSubSurface);
			}
		}

		// group select, compare pointers

		// surfaces
		if (option == &m_allOptions.m_options[ET_Geometry].m_options[0]) {
			// get list of all surfaces
			for (const VICUS::Building & b : project().m_buildings)
				for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
					for (const VICUS::Room & r : bl.m_rooms)
						for (const VICUS::Surface & s : r.m_surfaces)
							objs.insert(&s);
		}
		// sub-surfaces
		if (option == &m_allOptions.m_options[ET_Geometry].m_options[1]) {
			// get list of all subsurfaces
			for (const VICUS::Building & b : project().m_buildings)
				for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
					for (const VICUS::Room & r : bl.m_rooms)
						for (const VICUS::Surface & s : r.m_surfaces)
							for (const VICUS::SubSurface & sub : s.subSurfaces() )
								objs.insert(&sub);
		}
	}
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
	m_selectionIndex.resize(buttonLevel+1);
	updateButtonsAndListWidget();
}


void SVSmartSelectDialog::onListWidgetItemClicked() {

	// determine level and reduce selection accordingly
	unsigned int optionLevel = 0;
	QString name = m_ui->listWidgetOptions->currentItem()->text();
	for (;optionLevel < m_optionNames.size(); ++optionLevel)
		if (m_optionNames[optionLevel] == name)
			break;

	// append selection
	m_selectionIndex.push_back(optionLevel);
	updateButtonsAndListWidget();
}


void SVSmartSelectDialog::updateButtonsAndListWidget() {
	// Mind: it may be the case that we cannot follow all selections through, if
	//       dynamic options have disappeared sind last call

	qDeleteAll(m_selectionButtons);
	qDeleteAll(m_arrows);
	m_selectionButtons.clear();
	m_arrows.clear();

	FilterOption * options = &m_allOptions;
	std::vector<unsigned int> validSelections;
	for (unsigned int i=0; i<m_selectionIndex.size(); ++i) {
		unsigned int selectedIndex = m_selectionIndex[i];
		if (selectedIndex >= options->m_options.size())
			break; // not available, skip
		validSelections.push_back(selectedIndex);
		options = &options->m_options[selectedIndex];
		// create push button
		QPushButton * button = new QPushButton(options->m_name);
		// connect clicked signal
		connect(button, &QPushButton::clicked, this, &SVSmartSelectDialog::onSelectionButtonPressed);
		m_selectionButtons.push_back(button);
		if (m_selectionButtons.size() > 1) {
			QLabel *labelIcon = new QLabel;
			labelIcon->setPixmap(m_arrowRight);
			m_arrows.push_back(labelIcon);
			m_ui->horizontalLayoutBreadCrums->insertWidget(m_ui->horizontalLayoutBreadCrums->count()-1, labelIcon);
		}
		m_ui->horizontalLayoutBreadCrums->insertWidget(m_ui->horizontalLayoutBreadCrums->count()-1, button);

	}
	m_selectionIndex.swap(validSelections);

	if (!m_selectionButtons.empty()) {
		m_selectionButtons.back()->setDown(true);
	}

	// now update list widget

	m_ui->listWidgetOptions->clear();
	m_optionNames.clear();

	// based on the currently selected hierarchy level 'option', populate buttons
	for (FilterOption & o : options->m_options) {

		QListWidgetItem *item = new QListWidgetItem(o.m_name);
		item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
		item->setData(Qt::UserRole, (int)o.m_options.size());
		QFont fnt;
		if (o.m_options.size() > 0)
			fnt.setWeight(QFont::Bold);
		else
			fnt.setWeight(QFont::Normal);
		item->setFont(fnt);
		m_ui->listWidgetOptions->addItem(item);
		m_optionNames.push_back(&o.m_name);

	}
}


void SVSmartSelectDialog::selectNetworkComponents() {
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

		// heating demand equal to value
		double equalToHeatingDemandFilter = -1;
		if (m_ui->checkBoxMaxHeatingDemandEqualTo->isChecked() && m_ui->lineEditMaxHeatingDemandEqualTo->isValid())
			equalToHeatingDemandFilter = m_ui->lineEditMaxHeatingDemandEqualTo->value();

		unsigned int idFilter = VICUS::INVALID_ID;
		if (m_ui->checkBoxNodeId->isChecked() && m_ui->lineEditNodeId->isValid())
			idFilter = (unsigned int)m_ui->lineEditNodeId->value();

		QString nameFilter = "";
		if (m_ui->checkBoxNodeDisplayName->isChecked())
			nameFilter = m_ui->lineEditNodeDisplayName->text();


		// now process all network objects and store IDs of selected nodes
		std::set<unsigned int> nodeIDs;

		for (const VICUS::Network & nw : project().m_geometricNetworks) {

			// filter networks
			if (filterNetworkId != -1 && filterNetworkId != (int)nw.m_id)
				continue;

			for (const VICUS::NetworkNode & n : nw.m_nodes) {
			// apply filter rules

				// name filter
				if (!nameFilter.isEmpty() && n.m_displayName != nameFilter)
					continue;

				// id filter
				if (idFilter != VICUS::INVALID_ID && n.m_id != idFilter)
					continue;

				// filter based on type
				if (nodeTypeFilter != VICUS::NetworkNode::NUM_NT && n.m_type != nodeTypeFilter)
					continue;

				// heating demand filter
				if (n.m_maxHeatingDemand.value > maxHeatingDemandBelowFilter)
					continue;
				if (n.m_maxHeatingDemand.value < maxHeatingDemandAboveFilter)
					continue;
				if ( equalToHeatingDemandFilter > 0 &&
					 abs(n.m_maxHeatingDemand.value - equalToHeatingDemandFilter) > 0.1)
					continue;

				// all filters match, store unique ID of this node
				nodeIDs.insert(n.m_id);
			}
		}

		if (nodeIDs.empty())
			QMessageBox::information(this, QString(), tr("No objects found with given properties."));

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
				edgeIDs.insert(edge.m_id);
			}
		}

		SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selecting edges"), SVUndoTreeNodeState::SelectedState, edgeIDs, true);
		undo->push();
	}
}


void SVSmartSelectDialog::on_comboBoxNodeType_currentIndexChanged(int index) {
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

void SVSmartSelectDialog::on_checkBoxMaxHeatingDemandEqualTo_stateChanged(int arg1)
{
	bool checked = arg1;
	m_ui->checkBoxMaxHeatingDemandAbove->setEnabled(!checked);
	m_ui->checkBoxMaxHeatingDemandBelow->setEnabled(!checked);
	m_ui->lineEditMaxHeatingDemandEqualTo->setEnabled(checked);
}

void SVSmartSelectDialog::on_checkBoxNodeDisplayName_stateChanged(int arg1)
{
	m_ui->lineEditNodeDisplayName->setEnabled(arg1);
}


void SVSmartSelectDialog::on_listWidgetOptions_itemDoubleClicked(QListWidgetItem *item) {
	// only items that contain children can be clicked
	if (item->data(Qt::UserRole).toUInt() > 0)
		onListWidgetItemClicked();
}


void SVSmartSelectDialog::on_checkBoxFilterBoundaryCondition_stateChanged(int arg1) {
	m_ui->comboBoxBoundaryCondition->setEnabled(arg1);
}


void SVSmartSelectDialog::on_toolButtonReset_clicked() {
	// reset current hierarchy level to 0
	m_selectionIndex.clear();
	updateButtonsAndListWidget();
}

