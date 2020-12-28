#include "SVSimulationStartNetworkSim.h"
#include "ui_SVSimulationStartNetworkSim.h"

#include <QtExt_Directories.h>

#include <QFileInfo>
#include <QProcess>

#include "SVSettings.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

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

	m_networksMap.clear();
	for (const VICUS::Network & n : project().m_geometricNetworks)
		m_networksMap.insert(QString::fromStdString(n.m_name), n.m_id);
	m_ui->comboBoxNetwork->clear();
	m_ui->comboBoxNetwork->addItems(m_networksMap.keys());
	m_ui->comboBoxNetwork->setCurrentIndex(0);
	updateCmdLine();
	exec();
}


void SVSimulationStartNetworkSim::on_checkBoxCloseConsoleWindow_toggled(bool checked) {
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


bool SVSimulationStartNetworkSim::generateNandradProject(NANDRAD::Project & p) const {

	// create dummy zone
	NANDRAD::Zone z;
	z.m_id = 1;
	z.m_displayName = "dummy";
	z.m_type = NANDRAD::Zone::ZT_Active;
	NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Volume, 100);
	NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Area, 10);
	p.m_zones.push_back(z);

	// create dummy location/climate data
	p.m_location.m_climateFileName = (QtExt::Directories::databasesDir() + "/DB_climate/Konstantopol_20C.c6b").toStdString();
	NANDRAD::KeywordList::setParameter(p.m_location.m_para, "Location::para_t", NANDRAD::Location::P_Albedo, 20); // %
	NANDRAD::KeywordList::setParameter(p.m_location.m_para, "Location::para_t", NANDRAD::Location::P_Latitude, 53); // Deg
	NANDRAD::KeywordList::setParameter(p.m_location.m_para, "Location::para_t", NANDRAD::Location::P_Longitude, 13); // Deg

	// set simulation duration and solver parameters
	NANDRAD::KeywordList::setParameter(p.m_simulationParameter.m_para, "SimulationParameter::para_t", NANDRAD::SimulationParameter::P_InitialTemperature, 20); // C
	NANDRAD::KeywordList::setParameter(p.m_simulationParameter.m_interval.m_para,
									   "Interval::para_t", NANDRAD::Interval::P_End, 1.0/24); // d

//	// generate NANDRAD hydraulic network

	VICUS::Project proj = project();

	unsigned int networkId = m_networksMap.value(m_ui->comboBoxNetwork->currentText());
	VICUS::Network *network = proj.element(proj.m_geometricNetworks, networkId);

	/// TODO Hauke: this can be removed later

	//	 components
	NANDRAD::HydraulicNetworkComponent pump;
	pump.m_id = 0;
	pump.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel;
	pump.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].set("PressureHead", 1e5, IBK::Unit("Pa"));
	network->m_hydraulicComponents.push_back(pump);

	NANDRAD::HydraulicNetworkComponent heatExchanger;
	heatExchanger.m_id = 1;
	heatExchanger.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger;
//	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_].set("HeatFlux", 100, IBK::Unit("W"));
	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureLossCoefficient].set("PressureLossCoefficient", 10, IBK::Unit("-"));
	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter].set("HydraulicDiameter", 50, IBK::Unit("mm"));
	network->m_hydraulicComponents.push_back(heatExchanger);

	for (VICUS::NetworkNode &node: network->m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Source)
			node.m_componentId = pump.m_id;
		else if (node.m_type == VICUS::NetworkNode::NT_Building)
			node.m_componentId = heatExchanger.m_id;
	}

	for (VICUS::NetworkEdge &edge: network->m_edges)
		edge.m_modelType =  NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;

	network->updateNodeEdgeConnectionPointers();

	/// until here

	// create Nandrad Network
	NANDRAD::HydraulicNetwork hydraulicNetwork;
	std::vector<NANDRAD::HydraulicNetworkComponent> hydraulicComponents;
	hydraulicNetwork.m_id = networkId;
	hydraulicNetwork.m_displayName = network->m_name;
	network->createNandradHydraulicNetwork(hydraulicNetwork, hydraulicComponents);
	hydraulicNetwork.m_fluid.defaultFluidWater(1);

	// finally add to nandrad project
	p.m_hydraulicNetworks.clear();
	p.m_hydraulicNetworks.push_back(hydraulicNetwork);
	p.m_hydraulicComponents.clear();
	p.m_hydraulicComponents = hydraulicComponents;




#if 0
	// *** example network ****
	// geometric network
	VICUS::Network net;
	unsigned id1 = net.addNodeExt(IBKMK::Vector3D(0,0,0), VICUS::NetworkNode::NT_Source);
	unsigned id2 = net.addNodeExt(IBKMK::Vector3D(0,100,0), VICUS::NetworkNode::NT_Mixer);
	unsigned id3 = net.addNodeExt(IBKMK::Vector3D(100,100,0), VICUS::NetworkNode::NT_Building);
	unsigned id4 = net.addNodeExt(IBKMK::Vector3D(0,200,0), VICUS::NetworkNode::NT_Mixer);
	unsigned id5 = net.addNodeExt(IBKMK::Vector3D(100,200,0), VICUS::NetworkNode::NT_Building);
	unsigned id6 = net.addNodeExt(IBKMK::Vector3D(-50,200,0), VICUS::NetworkNode::NT_Building);
	net.addEdge(id1, id2, true);
	net.addEdge(id2, id3, true);
	net.addEdge(id2, id4, true);
	net.addEdge(id4, id5, true);
	net.addEdge(id4, id6, true);

	//	 components
	NANDRAD::HydraulicNetworkComponent pump;
	pump.m_id = 0;
	pump.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel;
	pump.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].set("PressureHead", 1000, IBK::Unit("Pa"));
	net.m_hydraulicComponents.push_back(pump);

	NANDRAD::HydraulicNetworkComponent heatExchanger;
	heatExchanger.m_id = 1;
	heatExchanger.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger;
	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_HeatFlux].set("HeatFlux", 100, IBK::Unit("W"));
	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureLossCoefficient].set("PressureLossCoefficient", 5, IBK::Unit("-"));
	heatExchanger.m_para[NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter].set("HydraulicDiameter", 25.6, IBK::Unit("mm"));
	net.m_hydraulicComponents.push_back(heatExchanger);

	net.m_nodes[id1].m_componentId = pump.m_id;
	net.m_nodes[id3].m_componentId = heatExchanger.m_id;
	net.m_nodes[id5].m_componentId = heatExchanger.m_id;
	net.m_nodes[id6].m_componentId = heatExchanger.m_id;


	// pipes
	VICUS::NetworkPipe pipe1;
	pipe1.m_id = 0;
	pipe1.m_displayName = "PE 32 x 3.2";
	pipe1.m_diameterOutside = 32;
	pipe1.m_sWall = 3.2;
	pipe1.m_roughness = 0.007;
	net.m_networkPipeDB.push_back(pipe1);
	VICUS::NetworkPipe pipe2;
	pipe2.m_id = 1;
	pipe2.m_displayName = "PE 50 x 4.6";
	pipe2.m_diameterOutside = 60;
	pipe2.m_sWall = 4.6;
	pipe2.m_roughness = 0.007;
	net.m_networkPipeDB.push_back(pipe2);

	net.edge(id1, id2)->m_pipeId = pipe2.m_id;
	net.edge(id1, id2)->m_modelType = NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;
	net.edge(id2, id3)->m_pipeId = pipe1.m_id;
	net.edge(id2, id3)->m_modelType = NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;
	net.edge(id2, id4)->m_pipeId = pipe1.m_id;
	net.edge(id2, id4)->m_modelType = NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;
	net.edge(id4, id5)->m_pipeId = pipe1.m_id;
	net.edge(id4, id5)->m_modelType = NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;
	net.edge(id4, id6)->m_pipeId = pipe1.m_id;
	net.edge(id4, id6)->m_modelType = NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe;

	net.updateNodeEdgeConnectionPointers();

	// create Nandrad Network
	NANDRAD::HydraulicNetwork hydraulicNetwork;
	std::vector<NANDRAD::HydraulicNetworkComponent> hydraulicComponents;
	hydraulicNetwork.m_id = 1;
	hydraulicNetwork.m_displayName = "auto generated from geometric network";
	net.createNandradHydraulicNetwork(hydraulicNetwork, hydraulicComponents);
	hydraulicNetwork.m_fluid.defaultFluidWater(1);


	// finally add to nandrad project
	p.m_hydraulicNetworks.clear();
	p.m_hydraulicNetworks.push_back(hydraulicNetwork);
	p.m_hydraulicComponents.clear();
	p.m_hydraulicComponents = hydraulicComponents;

#endif


	return true; // no errors, signal ok
}


void SVSimulationStartNetworkSim::on_pushButtonRun_clicked() {

	// generate NANDRAD project
	NANDRAD::Project p;

	generateNandradProject(p);

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


