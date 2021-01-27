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
									   "Interval::para_t", NANDRAD::Interval::P_End, 0.5); // d




	// generate NANDRAD hydraulic network



#if 0

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



//#if 0
	// *** test example 2: bit larger ***

	// geometric network
	VICUS::Network geoNetwork;
	geoNetwork.m_id = 1;
	geoNetwork.m_name = "validation example";
	unsigned id1 = geoNetwork.addNodeExt(IBKMK::Vector3D(0,0,0), VICUS::NetworkNode::NT_Source);
	unsigned id2 = geoNetwork.addNodeExt(IBKMK::Vector3D(0,100,0), VICUS::NetworkNode::NT_Mixer);
	unsigned id3 = geoNetwork.addNodeExt(IBKMK::Vector3D(100,100,0), VICUS::NetworkNode::NT_Building);
	unsigned id4 = geoNetwork.addNodeExt(IBKMK::Vector3D(0,200,0), VICUS::NetworkNode::NT_Mixer);
	unsigned id5 = geoNetwork.addNodeExt(IBKMK::Vector3D(100,200,0), VICUS::NetworkNode::NT_Building);
	unsigned id6 = geoNetwork.addNodeExt(IBKMK::Vector3D(-50,200,0), VICUS::NetworkNode::NT_Building);
	geoNetwork.addEdge(id1, id2, true);
	geoNetwork.addEdge(id2, id3, true);
	geoNetwork.addEdge(id2, id4, true);
	geoNetwork.addEdge(id4, id5, true);
	geoNetwork.addEdge(id4, id6, true);

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
	geoNetwork.m_nodes[id5].m_componentId = heatExchanger.m_id;
	geoNetwork.m_nodes[id5].m_heatFlux = IBK::Parameter("heat flux", -200, IBK::Unit("W"));
	geoNetwork.m_nodes[id6].m_componentId = heatExchanger.m_id;
	geoNetwork.m_nodes[id6].m_heatFlux = IBK::Parameter("heat flux", -300, IBK::Unit("W"));

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
	geoNetwork.edge(id4, id5)->m_pipeId = pipe1.m_id;
	geoNetwork.edge(id4, id5)->m_componentId = pipeComponent.m_id;
	geoNetwork.edge(id4, id5)->m_ambientTemperature = temp10;
	geoNetwork.edge(id4, id6)->m_pipeId = pipe1.m_id;
	geoNetwork.edge(id4, id6)->m_componentId = pipeComponent.m_id;
	geoNetwork.edge(id4, id6)->m_ambientTemperature = temp10;

	geoNetwork.updateNodeEdgeConnectionPointers();

	// *** test example 2 until here ***

//#endif



//	VICUS::Project proj = project();
//	unsigned int networkId = m_networksMap.value(m_ui->comboBoxNetwork->currentText());
//	const VICUS::Network geoNetwork = *proj.element(proj.m_geometricNetworks, networkId);

	// create Nandrad Network
	NANDRAD::HydraulicNetwork hydraulicNetwork;
	hydraulicNetwork.m_modelType = NANDRAD::HydraulicNetwork::MT_HydraulicNetwork;
	hydraulicNetwork.m_id = geoNetwork.m_id;
	hydraulicNetwork.m_displayName = geoNetwork.m_name;
	NANDRAD::KeywordList::setParameter(hydraulicNetwork.m_para,"HydraulicNetwork::para_t",
									   NANDRAD::HydraulicNetwork::P_DefaultFluidTemperature, 20);

	geoNetwork.createNandradHydraulicNetwork(hydraulicNetwork);

	hydraulicNetwork.m_fluid.defaultFluidWater(1);

	// finally add to nandrad project
	p.m_hydraulicNetworks.clear();
	p.m_hydraulicNetworks.push_back(hydraulicNetwork);


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


