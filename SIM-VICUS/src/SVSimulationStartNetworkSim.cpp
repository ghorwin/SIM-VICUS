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

#include "SVSimulationStartNetworkSim.h"
#include "ui_SVSimulationStartNetworkSim.h"

#include <QtExt_Directories.h>

#include <QFileInfo>
#include <QProcess>

#include "SVSettings.h"
#include "SVProjectHandler.h"
#include "SVUndoModifyNetwork.h"

#include <NANDRAD_Project.h>

#include <VICUS_Project.h>
#include <VICUS_KeywordList.h>

SVSimulationStartNetworkSim::SVSimulationStartNetworkSim(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSimulationStartNetworkSim)
{
	m_ui->setupUi(this);

	connect(m_ui->pushButtonClose, &QPushButton::clicked,
			this, &SVSimulationStartNetworkSim::close);
}


SVSimulationStartNetworkSim::~SVSimulationStartNetworkSim() {
	delete m_ui;
}


void SVSimulationStartNetworkSim::edit() {
	// transfer network names to ui and select the first

	// populate networks combobox
	m_networksMap.clear();
	for (const VICUS::Network & n : project().m_geometricNetworks)
		m_networksMap.insert(n.m_displayName, n.m_id);
	m_ui->comboBoxNetwork->clear();
	m_ui->comboBoxNetwork->addItems(m_networksMap.keys());
	m_ui->comboBoxNetwork->setCurrentIndex(0);
	m_ui->lineEditEndTime->setValue(30);

	// populate model type combobox
	m_ui->comboBoxModelType->clear();
	m_ui->comboBoxModelType->addItem(QString("%1 [%2]").arg(NANDRAD::KeywordList::Description("HydraulicNetwork::ModelType",
																		NANDRAD::HydraulicNetwork::MT_HydraulicNetwork))
														.arg(NANDRAD::KeywordList::Keyword("HydraulicNetwork::ModelType",
																		NANDRAD::HydraulicNetwork::MT_HydraulicNetwork)),
																		NANDRAD::HydraulicNetwork::MT_HydraulicNetwork);
	m_ui->comboBoxModelType->addItem(QString("%1 [%2]").arg(NANDRAD::KeywordList::Description("HydraulicNetwork::ModelType",
																		NANDRAD::HydraulicNetwork::MT_ThermalHydraulicNetwork))
														.arg(NANDRAD::KeywordList::Keyword("HydraulicNetwork::ModelType",
																		NANDRAD::HydraulicNetwork::MT_ThermalHydraulicNetwork)),
																		NANDRAD::HydraulicNetwork::MT_ThermalHydraulicNetwork);
	m_ui->comboBoxModelType->setCurrentIndex(
				m_ui->comboBoxModelType->findData(NANDRAD::HydraulicNetwork::MT_ThermalHydraulicNetwork));
	updateLineEdits();
	toggleRunButton();
	updateCmdLine();
	exec();
}


void SVSimulationStartNetworkSim::on_checkBoxCloseConsoleWindow_toggled(bool /*checked*/) {
	updateCmdLine();
}


void SVSimulationStartNetworkSim::updateCmdLine() {
	m_cmdLine.clear();
	m_solverExecutable = QFileInfo(SVSettings::instance().m_installDir + "/NandradSolver").filePath();
#ifdef WIN32
	m_solverExecutable += ".exe";
#endif // WIN32
	if (m_ui->checkBoxCloseConsoleWindow->isChecked())
		m_cmdLine << "-x";

	QString targetFile = QFileInfo(SVProjectHandler::instance().projectFile()).completeBaseName();

	targetFile += "-network.nandrad";
	m_targetProjectFile = QFileInfo(SVProjectHandler::instance().projectFile()).dir().filePath(targetFile);

	m_ui->lineEditCmdLine->setText("\"" + m_solverExecutable + "\" " + m_cmdLine.join(" ") + "\"" + m_targetProjectFile + "\"");
	m_ui->lineEditCmdLine->setCursorPosition( m_ui->lineEditCmdLine->text().length() );
}


void SVSimulationStartNetworkSim::on_pushButtonRun_clicked() {

	modifyParameters();

	// generate NANDRAD project
	NANDRAD::Project p;

	try {
		generateNandradProject(p);
	} catch (IBK::Exception &e) {
		QMessageBox msgBox(QMessageBox::Critical, "Error", e.what(), QMessageBox::Ok, this);
		msgBox.exec();
	}


	// save project
	p.writeXML(IBK::Path(m_targetProjectFile.toStdString()));

	// launch solver
	bool success = SVSettings::startProcess(m_solverExecutable, m_cmdLine, m_targetProjectFile);
	if (!success) {
		QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(m_solverExecutable));
		return;
	}

	close(); // finally close dialog
}


bool SVSimulationStartNetworkSim::generateNandradProject(NANDRAD::Project & p) const {
	FUNCID(SVSimulationStartNetworkSim::generateNandradProject);


	// get selected Vicus Network
	VICUS::Project proj = project();
	unsigned int networkId = m_networksMap.value(m_ui->comboBoxNetwork->currentText());
	const VICUS::Network vicusNetwork = *proj.element(proj.m_geometricNetworks, networkId);

	// node must have subNetworkId
	for (const VICUS::NetworkNode &n: vicusNetwork.m_nodes){
		if (n.m_subNetworkId != VICUS::INVALID_ID)
			throw IBK::Exception(IBK::FormatString("Missing subNetworkId in node with id #%1 ('%2')")
								 .arg(n.m_id).arg(n.m_displayName.toStdString()), FUNC_ID);
	}

	// buildings can only have one connected edge
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Building && node.m_edges.size()>1 )
			throw IBK::Exception(IBK::FormatString("Node with id #%1 has more than one edge connected, but is a building.")
								 .arg(node.m_id), FUNC_ID);
	}

	// check network type
	if (vicusNetwork.m_type != VICUS::Network::NET_DoublePipe)
		throw IBK::Exception("This NetworkType is not yet implemented. Use networkType 'DoublePipe'", FUNC_ID);


	// create dummy zone
	NANDRAD::Zone z;
	z.m_id = 1;
	z.m_displayName = "dummy";
	z.m_type = NANDRAD::Zone::ZT_Active;
	NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Volume, 100);
	NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Area, 10);
	p.m_zones.push_back(z);

	// create dummy location/climate data
	p.m_location.m_climateFilePath = (QtExt::Directories::databasesDir() + "/DB_climate/Konstantopol_20C.c6b").toStdString();
	NANDRAD::KeywordList::setParameter(p.m_location.m_para, "Location::para_t", NANDRAD::Location::P_Albedo, 20); // %
	NANDRAD::KeywordList::setParameter(p.m_location.m_para, "Location::para_t", NANDRAD::Location::P_Latitude, 53); // Deg
	NANDRAD::KeywordList::setParameter(p.m_location.m_para, "Location::para_t", NANDRAD::Location::P_Longitude, 13); // Deg

	// set simulation duration and solver parameters
	if(!m_ui->lineEditEndTime->isValid())
		throw IBK::Exception("Missing end time", FUNC_ID);
	double endTime = m_ui->lineEditEndTime->value();
	NANDRAD::KeywordList::setParameter(p.m_simulationParameter.m_para, "SimulationParameter::para_t",
									   NANDRAD::SimulationParameter::P_InitialTemperature, 20); // C
	NANDRAD::KeywordList::setParameter(p.m_simulationParameter.m_interval.m_para,
									   "Interval::para_t", NANDRAD::Interval::P_End, endTime); // d

	NANDRAD::Interval inter;
	NANDRAD::KeywordList::setParameter(inter.m_para, "Interval::para_t", NANDRAD::Interval::P_Start, 0); // d
	NANDRAD::KeywordList::setParameter(inter.m_para, "Interval::para_t", NANDRAD::Interval::P_End, endTime); // d
	NANDRAD::KeywordList::setParameter(inter.m_para, "Interval::para_t", NANDRAD::Interval::P_StepSize, 1); // h

	NANDRAD::OutputGrid grid;
	grid.m_name = "hourly";
	grid.m_intervals.push_back(inter);
	NANDRAD::IDGroup ids;
	ids.m_allIDs = true;

	NANDRAD::ObjectList objList;
	objList.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	objList.m_filterID = ids;
	objList.m_name = "the objects";

	NANDRAD::Outputs outputs;
	outputs.m_timeUnit = IBK::Unit("h");
	std::vector<std::string> quantities = {"FluidMassFlux", "OutletNodeTemperature", "PressureDifference", "Reynolds"};

	for (std::string &q: quantities){
		NANDRAD::OutputDefinition def;
		def.m_quantity = q;
		def.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;
		def.m_gridName = grid.m_name;
		def.m_objectListName = objList.m_name;
		outputs.m_definitions.push_back(def);
	}
	outputs.m_grids.push_back(grid);
	p.m_outputs = outputs;
	p.m_objectLists.push_back(objList);


	// *** create Nandrad Network
	p.m_hydraulicNetworks.clear();
	NANDRAD::HydraulicNetwork nandradNetwork;
	nandradNetwork.m_modelType = NANDRAD::HydraulicNetwork::ModelType(m_ui->comboBoxModelType->currentData().toUInt());
	nandradNetwork.m_id = vicusNetwork.m_id;
	nandradNetwork.m_displayName = vicusNetwork.m_displayName.toStdString();
	nandradNetwork.m_para[NANDRAD::HydraulicNetwork::P_DefaultFluidTemperature] =
			vicusNetwork.m_para[VICUS::Network::P_DefaultFluidTemperature];
	nandradNetwork.m_para[NANDRAD::HydraulicNetwork::P_InitialFluidTemperature] =
			vicusNetwork.m_para[VICUS::Network::P_InitialFluidTemperature];
	nandradNetwork.m_para[NANDRAD::HydraulicNetwork::P_ReferencePressure] =
			vicusNetwork.m_para[VICUS::Network::P_ReferencePressure];


	// *** Transfer FLUID from Vicus to Nandrad
	const SVDatabase  & db = SVSettings::instance().m_db;
	VICUS::NetworkFluid fluid;
	fluid.defaultFluidWater(1);
	nandradNetwork.m_fluid.m_displayName = fluid.m_displayName.string();

	fluid.m_kinematicViscosity.m_values.setValues(std::vector<double>{0,20}, std::vector<double>{1.793e-6,1.793e-6});

	nandradNetwork.m_fluid.m_kinematicViscosity = fluid.m_kinematicViscosity;
	for (int i=0; i<VICUS::NetworkFluid::NUM_P; ++i)
		nandradNetwork.m_fluid.m_para[i] = fluid.m_para[i];


	// *** Transfer COMPONENTS from Vicus to Nandrad

	// --> collect sub networks
	std::vector<unsigned int> subNetworkIds, componentIds, scheduleIds;
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Mixer)
			continue;
		insertIfNotContained(subNetworkIds, node.m_subNetworkId);
	}

	// --> collect and check components
	unsigned int maxNumberElements = 1;
	for (unsigned int subId: subNetworkIds){
		const VICUS::SubNetwork *sub = db.m_subNetworks[subId];
		if (sub == nullptr)
			throw IBK::Exception(IBK::FormatString("Sub Network with id #%1 does not exist in database").arg(subId), FUNC_ID);
		if (sub->m_elements.size() > maxNumberElements)
			maxNumberElements = sub->m_elements.size();
		for (const NANDRAD::HydraulicNetworkElement &el: sub->m_elements){
			insertIfNotContained(componentIds, el.m_componentId);
		}
	}

	// --> transfer
	for (unsigned int id: componentIds){
		const VICUS::NetworkComponent *comp = db.m_networkComponents[id];
		Q_ASSERT(comp != nullptr);
		NANDRAD::HydraulicNetworkComponent nandradComp;
		nandradComp.m_id = comp->m_id;
		nandradComp.m_displayName = comp->m_displayName.string(IBK::MultiLanguageString::m_language, "en");
		nandradComp.m_modelType = (NANDRAD::HydraulicNetworkComponent::ModelType) comp->m_modelType;
		nandradComp.m_polynomCoefficients = comp->m_polynomCoefficients;
		for (int i=0; i<VICUS::NetworkComponent::NUM_P; ++i)
			nandradComp.m_para[i] = comp->m_para[i];
		nandradNetwork.m_components.push_back(nandradComp);
	}


	// *** Transform PIPES from Vicus to NANDRAD

	// --> collect all pipeIds used in vicus network
	std::vector<unsigned int> pipeIds;
	for (const VICUS::NetworkEdge &edge: vicusNetwork.m_edges){
		if(edge.m_pipeId == VICUS::INVALID_ID)
				throw IBK::Exception(IBK::FormatString("Edge '%1'->'%2' has no referenced pipe")
									 .arg(edge.nodeId1()).arg(edge.nodeId2()), FUNC_ID);
		if (std::find(pipeIds.begin(), pipeIds.end(), edge.m_pipeId) == pipeIds.end())
			pipeIds.push_back(edge.m_pipeId);
	}

	// --> transfer
	for(unsigned int id: pipeIds){
		const VICUS::NetworkPipe *pipe = db.m_pipes[id];
		Q_ASSERT(pipe != nullptr);
		NANDRAD::HydraulicNetworkPipeProperties pipeProp;
		pipeProp.m_id = pipe->m_id;

		// calculate length-specific pipe wall U-Value in W/mK, some references to understand the equation better
		double UValue;
		const double &dInsulation = pipe->m_para[VICUS::NetworkPipe::P_ThicknessInsulation].value;
		const double &lambdaInsulation = pipe->m_para[VICUS::NetworkPipe::P_ThermalConductivityInsulation].value;
		const double &da = pipe->m_para[VICUS::NetworkPipe::P_DiameterOutside].value;
		const double &lambdaWall = pipe->m_para[VICUS::NetworkPipe::P_ThermalConductivityWall].value;

		if (dInsulation>0 && lambdaInsulation>0){
			UValue = 2*PI/ ( 1/lambdaWall * IBK::f_log(da / pipe->diameterInside())
						+ 1/lambdaInsulation *
						  IBK::f_log((da + 2*dInsulation) / da) );
		}
		else {
			UValue = 2*PI/ ( lambdaWall * IBK::f_log(da / pipe->diameterInside()) );
		}

		// set pipe properties
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter, da);
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter, pipe->diameterInside());
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeRoughness, pipe->m_para[VICUS::NetworkPipe::P_RoughnessWall].value);
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall, UValue);
		nandradNetwork.m_pipeProperties.push_back(pipeProp);
	}


	// *** Transfer ELEMENTS from Vicus to Nandrad

	// estimated number of elements
	nandradNetwork.m_elements.reserve(vicusNetwork.m_nodes.size() * maxNumberElements + 2 * vicusNetwork.m_edges.size());

	// --> offset for ids of return pipes
	unsigned int idOffsetOutlet = (unsigned int) std::pow( 10, std::ceil( std::log10(vicusNetwork.m_nodes.size())) + 1 );


	std::vector<unsigned int> allNodeIds = {0};
	std::vector<unsigned int> allElementIds = {0};
	std::map<unsigned int, unsigned int> nodeTotalInletId;  // a map that stores the inlet node id of the first element of this node
	std::map<unsigned int, unsigned int> nodeTotalOutletId; // a map that stores the outlet node id of the last element of this node

	// iterate over all geometric network nodes
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes) {

		// check if this is a mixer
		if (node.m_type == VICUS::NetworkNode::NT_Mixer){
			nodeTotalInletId[node.m_id] = VICUS::Project::uniqueId(allNodeIds);
			nodeTotalOutletId[node.m_id] = nodeTotalInletId[node.m_id];
			allNodeIds.push_back(nodeTotalInletId[node.m_id]);
			continue;
		}

		// check if there is a valid sub network
		const VICUS::SubNetwork *sub = db.m_subNetworks[node.m_subNetworkId];
		if (sub == nullptr)
			throw IBK::Exception(IBK::FormatString("Node with id #%1 has not a valid referenced sub network!").arg(node.m_id), FUNC_ID);

		// now iterate over all elements of the sub network:
		bool subInletFound = false;
		bool subOutletFound = false;
		nodeTotalInletId[node.m_id] = VICUS::Project::uniqueId(allNodeIds);
		nodeTotalOutletId[node.m_id] = nodeTotalInletId[node.m_id] + maxNumberElements;

		for (const NANDRAD::HydraulicNetworkElement &el: sub->m_elements){

			// 1. copy the element and create a unique element id for it
			NANDRAD::HydraulicNetworkElement element = el;
			element.m_id = VICUS::Project::uniqueId(allElementIds);
			allElementIds.push_back(element.m_id);

			// 2. now check if we have at least one element that has inletNodeId=0 (this is the sub network inlet)
			// and at least one element with outletNodeId=0 (this is the sub network outlet)
			// 3. use a unique id (the nodeTotalInletId) for the sub network inlet
			// 4. and another id (the nodeTotalOutletId) for the sub network outlet
			// 5. for all other node ids: just add the nodeTotalInletId
			if (el.m_inletNodeId == 0) {
				subInletFound = true;
				if (node.m_type == VICUS::NetworkNode::NT_Source)  // in case this a source node: switch inlet and outlet
					element.m_inletNodeId = nodeTotalOutletId[node.m_id];
				else
					element.m_inletNodeId = nodeTotalInletId[node.m_id];
				element.m_outletNodeId += nodeTotalInletId[node.m_id];
			}
			else if(el.m_outletNodeId == 0){
				subOutletFound = true;
				if (node.m_type == VICUS::NetworkNode::NT_Source) // in case this a source node: switch inlet and outlet
					element.m_outletNodeId = nodeTotalInletId[node.m_id];
				else
					element.m_outletNodeId = nodeTotalOutletId[node.m_id];
				element.m_inletNodeId += nodeTotalInletId[node.m_id];
			}
			else{
				element.m_inletNodeId += nodeTotalInletId[node.m_id];
				element.m_outletNodeId += nodeTotalInletId[node.m_id];
			}

			allNodeIds.push_back(element.m_inletNodeId);
			allNodeIds.push_back(element.m_outletNodeId);

			// 6. small hack to get component name in display name
			const VICUS::NetworkComponent *comp = db.m_networkComponents[element.m_componentId];
			Q_ASSERT(comp!=nullptr);
			element.m_displayName = IBK::FormatString("%1_%2_%3")
					.arg(comp->m_displayName.string()).arg(element.m_id).arg(node.m_displayName.toStdString()).str();

			// 7. if this is a source node: set the respective reference element id of the network (for pressure calculation)
			if (node.m_type == VICUS::NetworkNode::NT_Source)
				nandradNetwork.m_referenceElementId = element.m_id;

			// 8. if this element is the one which shall exchange heat: we copy the respective heat exchange properties from the node
			// we recognize this using the original element id (el.m_id)
			if (el.m_id == sub->m_heatExchangeElementId)
				element.m_heatExchange = node.m_heatExchange;

			// 9. add element to the nandrad network
			nandradNetwork.m_elements.push_back(element);
		}

		if (!subInletFound)
			throw IBK::Exception(IBK::FormatString("Sub Network with id #%1 does not have an element with inletNodeId=0, "
												   "This is required for every sub network").arg(node.m_id), FUNC_ID);
		if (!subOutletFound)
			throw IBK::Exception(IBK::FormatString("Sub Network with id #%1 does not have an element with outletNodeId=0, "
												   "This is required for every sub network").arg(node.m_id), FUNC_ID);

	}



	// *** Transfer ELEMENTS of EDGES from Vicus to Nandrad

	// find source node and create set of edges, which are ordered according to their distance to the source node
	std::set<const VICUS::NetworkNode *> dummyNodeSet;
	std::set<VICUS::NetworkEdge *> orderedEdges;
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Source){
			node.setInletOutletNode(dummyNodeSet, orderedEdges);
			break;
		}
	}

	// now iterate over edges
	for (const VICUS::NetworkEdge *edge: orderedEdges) {



		// ....


		// TODO Hauke:

//		// check if the component has a model type which corresponds to a pipe
//		const VICUS::NetworkComponent *comp = db.m_networkComponents[edge->m_componentId];
//		Q_ASSERT(comp!=nullptr);
//		if ( ! (comp->m_modelType == VICUS::NetworkComponent ::MT_SimplePipe ||
//				comp->m_modelType == VICUS::NetworkComponent ::MT_DynamicPipe) )
//			throw IBK::Exception(IBK::FormatString("Component of edge %1->%2 does not represent a pipe")
//													.arg(edge->nodeId1()).arg(edge->nodeId2()), FUNC_ID);

//		// check if there is a reference to a pipe from DB
//		const VICUS::NetworkPipe *pipe = db.m_pipes[edge->m_pipeId];
//		if (pipe == nullptr)
//			throw IBK::Exception(IBK::FormatString("Edge  %1->%2 has no defined pipe from database")
//								 .arg(edge->m_node1->m_id).arg(edge->m_node2->m_id), FUNC_ID);



//		// add inlet pipe element
//		NANDRAD::HydraulicNetworkElement inletPipe(VICUS::Project::largestUniqueId(nandradNetwork.m_elements),
//													edge->m_nodeIdInlet,
//													edge->m_nodeIdOutlet,
//													edge->m_componentId,
//													edge->m_pipeId,
//													edge->length());
//		inletPipe.m_displayName = edge->m_displayName.toStdString();
//		inletPipe.m_heatExchange = edge->m_heatExchange.toNandradHeatExchange();

//		// small hack to get component name in display name
//		inletPipe.m_displayName = IBK::FormatString("%1_%2_%3").arg(comp->m_displayName.string())
//									.arg(inletPipe.m_id).arg(inletPipe.m_displayName).str();

//		nandradNetwork.m_elements.push_back(inletPipe);

//		// add outlet pipe element
//		NANDRAD::HydraulicNetworkElement outletPipe(VICUS::Project::largestUniqueId(nandradNetwork.m_elements),
//													edge->m_nodeIdOutlet + idOffsetOutlet,
//													edge->m_nodeIdInlet + idOffsetOutlet,
//													edge->m_componentId,
//													edge->m_pipeId,
//													edge->length());
//		outletPipe.m_displayName = edge->m_displayName.toStdString();
//		outletPipe.m_heatExchange = edge->m_heatExchange.toNandradHeatExchange();

//		// small hack to get component name in display name
//		outletPipe.m_displayName = IBK::FormatString("%1_%2_%3").arg(comp->m_displayName.string())
//									.arg(outletPipe.m_id).arg(outletPipe.m_displayName).str();

//		nandradNetwork.m_elements.push_back(outletPipe);

	}

	// DONE !!!
	// finally add to nandrad project
	p.m_hydraulicNetworks.push_back(nandradNetwork);

	return true;

}



void SVSimulationStartNetworkSim::modifyParameters()
{
	VICUS::Project proj = project();
	unsigned int networkId = m_networksMap.value(m_ui->comboBoxNetwork->currentText());
	VICUS::Network network = *proj.element(proj.m_geometricNetworks, networkId);
	if (m_ui->lineEditReferencePressure->isValid())
		VICUS::KeywordList::setParameter(network.m_para, "Network::para_t", VICUS::Network::P_ReferencePressure,
										 m_ui->lineEditReferencePressure->value());
	if (m_ui->lineEditDefaultFluidTemperature->isValid()){
		network.m_para[VICUS::Network::P_DefaultFluidTemperature] =
				IBK::Parameter("DefaultFluidTemperature", m_ui->lineEditDefaultFluidTemperature->value(), IBK::Unit("C"));
		network.m_para[VICUS::Network::P_InitialFluidTemperature] =
				IBK::Parameter("InitialFluidTemperature", m_ui->lineEditDefaultFluidTemperature->value(), IBK::Unit("C"));
	}

	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(),
											  project().element(project().m_geometricNetworks, networkId));
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), networkIndex, network);
	undo->push(); // modifies project and updates views
}

void SVSimulationStartNetworkSim::updateLineEdits()
{
	const VICUS::Project &proj = project();
	unsigned int networkId = m_networksMap.value(m_ui->comboBoxNetwork->currentText());
	const VICUS::Network *network = proj.element(proj.m_geometricNetworks, networkId);
	if (!network->m_para[VICUS::Network::P_ReferencePressure].empty())
		m_ui->lineEditReferencePressure->setValue(network->m_para[VICUS::Network::P_ReferencePressure].value);
	if (!network->m_para[VICUS::Network::P_DefaultFluidTemperature].empty())
		m_ui->lineEditDefaultFluidTemperature->setValue(network->m_para[VICUS::Network::P_DefaultFluidTemperature].get_value("C"));
}


void SVSimulationStartNetworkSim::toggleRunButton()
{
	bool check = m_ui->lineEditReferencePressure->isValid() && m_ui->lineEditDefaultFluidTemperature->isValid();
	m_ui->pushButtonRun->setEnabled(check);
}


void SVSimulationStartNetworkSim::on_lineEditReferencePressure_editingFinished()
{
	modifyParameters();
	toggleRunButton();
}

void SVSimulationStartNetworkSim::on_lineEditDefaultFluidTemperature_editingFinished()
{
	modifyParameters();
	toggleRunButton();
}

#if 0


	// create dummy zone
	NANDRAD::Zone z;
	z.m_id = 1;
	z.m_displayName = "dummy";
	z.m_type = NANDRAD::Zone::ZT_Active;
	NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Volume, 100);
	NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Area, 10);
	p.m_zones.push_back(z);

	// create dummy location/climate data
	p.m_location.m_climateFilePath = (QtExt::Directories::databasesDir() + "/DB_climate/Konstantopol_20C.c6b").toStdString();
	NANDRAD::KeywordList::setParameter(p.m_location.m_para, "Location::para_t", NANDRAD::Location::P_Albedo, 20); // %
	NANDRAD::KeywordList::setParameter(p.m_location.m_para, "Location::para_t", NANDRAD::Location::P_Latitude, 53); // Deg
	NANDRAD::KeywordList::setParameter(p.m_location.m_para, "Location::para_t", NANDRAD::Location::P_Longitude, 13); // Deg

	if(!m_ui->lineEditEndTime->isValid())
		return false;
	double endTime = m_ui->lineEditEndTime->value();

	// set simulation duration and solver parameters
	NANDRAD::KeywordList::setParameter(p.m_simulationParameter.m_para, "SimulationParameter::para_t", NANDRAD::SimulationParameter::P_InitialTemperature, 20); // C
	NANDRAD::KeywordList::setParameter(p.m_simulationParameter.m_interval.m_para,
									   "Interval::para_t", NANDRAD::Interval::P_End, endTime); // d


	// *** test example 1: very simple, one pump, one pipe, one HX ***

	// geometric network
	VICUS::Network geoNetwork;
	geoNetwork.m_id = 0;
	geoNetwork.m_name = "simple test";
	unsigned id1 = geoNetwork.addNodeExt(IBKMK::Vector3D(0,0,0), VICUS::NetworkNode::NT_Source);
	unsigned id2 = geoNetwork.addNodeExt(IBKMK::Vector3D(0,100,0), VICUS::NetworkNode::NT_Building);
	geoNetwork.addEdge(id1, id2, true);

	//	*** Components

	// -> pump
	NANDRAD::HydraulicNetworkComponent pump;
	pump.m_id = 0;
	pump.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel;
	pump.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].set("PressureHead", -1000, IBK::Unit("Pa"));
	pump.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].set("Volume", 0.01, IBK::Unit("m3"));
	pump.m_heatExchangeType = NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant;
	geoNetwork.m_hydraulicComponents.push_back(pump);

	// -> heat exchanger
	NANDRAD::HydraulicNetworkComponent heatExchanger;
	heatExchanger.m_id = 1;
	heatExchanger.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger;
	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].set("Volume", 0.1, IBK::Unit("m3"));
	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_UAValue].set("UA", 1, IBK::Unit("W/m2K"));
	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureLossCoefficient].set("PressureLossCoefficient", 5, IBK::Unit("-"));
	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter].set("HydraulicDiameter", 25.6, IBK::Unit("mm"));
	heatExchanger.m_heatExchangeType = NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant;
	geoNetwork.m_hydraulicComponents.push_back(heatExchanger);

	// -> pipe
	NANDRAD::HydraulicNetworkComponent pipeComponent;
	pipeComponent.m_id = 2;
	pipeComponent.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe;
	pipeComponent.m_heatExchangeType = NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant;
	geoNetwork.m_hydraulicComponents.push_back(pipeComponent);

	// *** pipe catalog
	VICUS::NetworkPipe pipe1;
	pipe1.m_id = 0;
	pipe1.m_displayName = "PE 32 x 3.2";
	pipe1.m_diameterOutside = 32;
	pipe1.m_wallThickness = 3.2;
	pipe1.m_roughness = 0.007;
	geoNetwork.m_networkPipeDB.push_back(pipe1);

	// *** add IDs to nodes, edges
	geoNetwork.m_nodes[id1].m_componentId = pump.m_id;
	geoNetwork.m_nodes[id2].m_componentId = heatExchanger.m_id;
	geoNetwork.m_nodes[id2].m_heatFlux = IBK::Parameter("heat flux", 100, IBK::Unit("W"));

	geoNetwork.edge(id1, id2)->m_pipeId = pipe1.m_id;
	geoNetwork.edge(id1, id2)->m_componentId = pipeComponent.m_id;
	geoNetwork.edge(id1, id2)->m_ambientTemperature = IBK::Parameter("Temperature", 10, IBK::Unit("C"));

	geoNetwork.updateNodeEdgeConnectionPointers();

	// *** test example 1 until here ***

#endif



#if 0
	// *** test example 2: bit larger ***

	// geometric network
	VICUS::Network geoNetwork;
	geoNetwork.m_id = 1;
	geoNetwork.m_name = "validation example";
	unsigned id1 = geoNetwork.addNodeExt(IBKMK::Vector3D(0,0,0), VICUS::NetworkNode::NT_Source);
	unsigned id2 = geoNetwork.addNodeExt(IBKMK::Vector3D(0,100,0), VICUS::NetworkNode::NT_Mixer);
	unsigned id3 = geoNetwork.addNodeExt(IBKMK::Vector3D(100,100,0), VICUS::NetworkNode::NT_Building);
	unsigned id4 = geoNetwork.addNodeExt(IBKMK::Vector3D(100,-100,0), VICUS::NetworkNode::NT_Building);
	geoNetwork.addEdge(id1, id2, true);
	geoNetwork.addEdge(id2, id3, true);
	geoNetwork.addEdge(id2, id4, true);

	//	*** Components

	// -> pump
	NANDRAD::HydraulicNetworkComponent pump;
	pump.m_id = 1;
	pump.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel;
	NANDRAD::KeywordList::setParameter(pump.m_para, "HydraulicNetworkComponent::para_t",
									   NANDRAD::HydraulicNetworkComponent::P_PressureHead, 1000);
	NANDRAD::KeywordList::setParameter(pump.m_para, "HydraulicNetworkComponent::para_t",
									   NANDRAD::HydraulicNetworkComponent::P_Volume, 0.01);
	pump.m_heatExchangeType = NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant;
	geoNetwork.m_hydraulicComponents.push_back(pump);

	// -> heat exchanger
	NANDRAD::HydraulicNetworkComponent heatExchanger;
	heatExchanger.m_id = 2;
	heatExchanger.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger;
	NANDRAD::KeywordList::setParameter(heatExchanger.m_para, "HydraulicNetworkComponent::para_t",
									   NANDRAD::HydraulicNetworkComponent::P_Volume, 0.01);
	NANDRAD::KeywordList::setParameter(heatExchanger.m_para, "HydraulicNetworkComponent::para_t",
									   NANDRAD::HydraulicNetworkComponent::P_UAValue, 1);
	NANDRAD::KeywordList::setParameter(heatExchanger.m_para, "HydraulicNetworkComponent::para_t",
									   NANDRAD::HydraulicNetworkComponent::P_PressureLossCoefficient, 5);
	NANDRAD::KeywordList::setParameter(heatExchanger.m_para, "HydraulicNetworkComponent::para_t",
									   NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter, 25.6);
	heatExchanger.m_heatExchangeType = NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant;
	geoNetwork.m_hydraulicComponents.push_back(heatExchanger);

	// -> pipe
	NANDRAD::HydraulicNetworkComponent pipeComponent;
	pipeComponent.m_id = 3;
	pipeComponent.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe;
	pipeComponent.m_heatExchangeType = NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant;
	NANDRAD::KeywordList::setParameter(pipeComponent.m_para, "HydraulicNetworkComponent::para_t",
									   NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient, 5);
	geoNetwork.m_hydraulicComponents.push_back(pipeComponent);

	// *** pipe catalog
	VICUS::NetworkPipe pipe1;
	pipe1.m_id = 1;
	pipe1.m_displayName = IBK::MultiLanguageString("PE 32 x 3.2");
	pipe1.m_diameterOutside = 32;
	pipe1.m_wallThickness = 3.2;
	pipe1.m_roughness = 0.007;
	pipe1.m_lambdaWall = 0.4;
	geoNetwork.m_networkPipeDB.push_back(pipe1);
	VICUS::NetworkPipe pipe2;
	pipe2.m_id = 2;
	pipe2.m_displayName = IBK::MultiLanguageString("PE 50 x 4.6");
	pipe2.m_diameterOutside = 50;
	pipe2.m_wallThickness = 4.6;
	pipe2.m_roughness = 0.007;
	pipe2.m_lambdaWall = 0.4;
	geoNetwork.m_networkPipeDB.push_back(pipe2);

	// *** add IDs to nodes, edges
	geoNetwork.m_nodes[id1].m_componentId = pump.m_id;
	geoNetwork.m_nodes[id3].m_componentId = heatExchanger.m_id;
	geoNetwork.m_nodes[id3].m_heatFlux = IBK::Parameter("heat flux", -100, IBK::Unit("W"));
	geoNetwork.m_nodes[id4].m_componentId = heatExchanger.m_id;
	geoNetwork.m_nodes[id4].m_heatFlux = IBK::Parameter("heat flux", -200, IBK::Unit("W"));

	IBK::Parameter temp10("Temperature", 10, IBK::Unit("C"));

	geoNetwork.edge(id1, id2)->m_pipeId = pipe2.m_id;
	geoNetwork.edge(id1, id2)->m_componentId = pipeComponent.m_id;
	geoNetwork.edge(id1, id2)->m_ambientTemperature = temp10;

	geoNetwork.edge(id2, id3)->m_pipeId = pipe1.m_id;
	geoNetwork.edge(id2, id3)->m_componentId = pipeComponent.m_id;
	geoNetwork.edge(id2, id3)->m_ambientTemperature = temp10;

	geoNetwork.edge(id2, id4)->m_pipeId = pipe1.m_id;
	geoNetwork.edge(id2, id4)->m_componentId = pipeComponent.m_id;
	geoNetwork.edge(id2, id4)->m_ambientTemperature = temp10;


	geoNetwork.updateNodeEdgeConnectionPointers();

	// *** test example 2 until here ***

#endif


