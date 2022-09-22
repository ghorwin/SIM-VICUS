#include "SVPropEditNetwork.h"
#include "ui_SVPropEditNetwork.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"
#include "SVUndoModifyNetwork.h"
#include "SVUndoAddNetwork.h"
#include "SVNetworkDialogSelectPipes.h"
#include "SVViewStateHandler.h"

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include <QtExt_Conversions.h>


SVPropEditNetwork::SVPropEditNetwork(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditNetwork)
{
	m_ui->setupUi(this);

	// connect with project handler
	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified, this, &SVPropEditNetwork::onModified);

	// validating line edits
	m_ui->lineEditMaxPressureDrop->setup(std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), tr("Maximum pressure drop per pipe length"), true, true);
	m_ui->lineEditTemperatureDifference->setup(0.1, std::numeric_limits<double>::max(), tr("Temperature difference at sub station"), true, true);
	m_ui->lineEditTemperatureSetpoint->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Fluid temperature for calculation of viscosity"), true, true);
	m_ui->lineEditThresholdSmallEdge->setup(std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), tr("Pipes shorter than this value will be removed"), true, true);


	// hide some features for now
	m_ui->pushButtonReduceRedundantNodes->setVisible(false);
	m_ui->groupBoxRemoveShortEdges->setVisible(false);

}

SVPropEditNetwork::~SVPropEditNetwork()
{
	delete m_ui;
}


void SVPropEditNetwork::onModified(int modificationType, ModificationInfo * data) {

	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {

		case SVProjectHandler::AllModified:
		case SVProjectHandler::NetworkGeometryChanged:
		case SVProjectHandler::NetworkDataChanged: {
			updateComboBoxNetworks();
			updateUi();
		} break;
		default: ; // just to make compiler happy
	}
}



void SVPropEditNetwork::updateUi() {

	m_ui->labelNetworkName->clear();
	m_ui->labelEdgeCount->clear();
	m_ui->labelNodeCount->clear();
	m_ui->labelNetworkConnected->clear();
	m_ui->labelTotalLength->clear();
	m_ui->lineEditFluidName->clear();

	unsigned int netId = m_ui->comboBoxCurrentNetwork->currentData().toUInt();
	m_currentNetwork = VICUS::element(project().m_geometricNetworks, netId);

	// enable / disable based on existing network
	bool haveNetwork = m_currentNetwork != nullptr;
	m_ui->groupBoxEditNetwork->setEnabled(haveNetwork);
	m_ui->groupBoxFluid->setEnabled(haveNetwork);
	m_ui->groupBoxProperties->setEnabled(haveNetwork);
	m_ui->groupBoxRemoveShortEdges->setEnabled(haveNetwork);
	m_ui->groupBoxSizePipes->setEnabled(haveNetwork);
	m_ui->groupBoxVisualisation->setEnabled(haveNetwork);
	if (!haveNetwork)
		return;

	m_ui->labelNetworkName->setText(m_currentNetwork->m_displayName);
	m_ui->labelEdgeCount->setText(QString("%1").arg(m_currentNetwork->m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(m_currentNetwork->m_nodes.size()));

	const SVDatabase & db = SVSettings::instance().m_db;
	const VICUS::NetworkFluid * fluid = db.m_fluids[m_currentNetwork->m_idFluid];
	if (fluid != nullptr){
		m_ui->lineEditFluidName->setText(QtExt::MultiLangString2QString(fluid->m_displayName));
		m_ui->lineEditFluidName->setStyleSheet("QLabel {color: black}");
	}
	else{
		m_ui->lineEditFluidName->setText("No fluid selected");
		m_ui->lineEditFluidName->setStyleSheet("QLabel {color: red}");
	}

	if (m_currentNetwork->checkConnectedGraph()){
		m_ui->labelNetworkConnected->setText(tr("Network is connected"));
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: green}");
	}
	else{
		m_ui->labelNetworkConnected->setText("Network is unconnected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: red}");
	}
	m_ui->labelTotalLength->setText(QString("%1 m").arg(m_currentNetwork->totalLength()));
	m_ui->pushButtonConnectBuildings->setEnabled(m_currentNetwork->nextUnconnectedBuilding()>=0);
	m_ui->pushButtonReduceDeadEnds->setEnabled(m_currentNetwork->checkConnectedGraph() && m_currentNetwork->numberOfBuildings() > 0);

	// scales
	m_ui->horizontalSliderScaleEdges->setValue((int)m_currentNetwork->m_scaleEdges);
	m_ui->horizontalSliderScaleNodes->setValue((int)m_currentNetwork->m_scaleNodes);

	// Pipe sizing algotithm parameters
	m_ui->lineEditTemperatureSetpoint->setValue(m_currentNetwork->m_para[VICUS::Network::P_TemperatureSetpoint].get_value(IBK::Unit("C")));
	m_ui->lineEditTemperatureDifference->setValue(m_currentNetwork->m_para[VICUS::Network::P_TemperatureDifference].value);
	m_ui->lineEditMaxPressureDrop->setValue(m_currentNetwork->m_para[VICUS::Network::P_MaxPressureLoss].value);
}


void SVPropEditNetwork::updateComboBoxNetworks() {
	// remember selected id
	unsigned int id = m_ui->comboBoxCurrentNetwork->currentData().toUInt();
	// fill combobox
	m_ui->comboBoxCurrentNetwork->blockSignals(true);
	m_ui->comboBoxCurrentNetwork->clear();
	const VICUS::Project &p = project();
	for (const VICUS::Network &n : p.m_geometricNetworks)
		m_ui->comboBoxCurrentNetwork->addItem(n.m_displayName, n.m_id);
	// reselect index
	int idx = m_ui->comboBoxCurrentNetwork->findData(id);
	if (idx != -1)
		m_ui->comboBoxCurrentNetwork->setCurrentIndex(idx);
	m_ui->comboBoxCurrentNetwork->blockSignals(false);
}


void SVPropEditNetwork::on_horizontalSliderScaleNodes_valueChanged(int value) {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	network.m_scaleNodes = value;
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropEditNetwork::on_horizontalSliderScaleEdges_valueChanged(int value) {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	network.m_scaleEdges= value;
	const VICUS::Project p = project();
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropEditNetwork::on_pushButtonSelectFluid_clicked() {
	unsigned int currentId  = m_currentNetwork->m_idFluid;
	SVDatabaseEditDialog *dialog = SVMainWindow::instance().dbFluidEditDialog();
	unsigned int newId = dialog->select(currentId);
	if (newId > 0){
		Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
		VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
		network.m_idFluid = newId;
		network.updateNodeEdgeConnectionPointers();
		SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
		undo->push(); // modifies project and updates views
	}
}


void SVPropEditNetwork::on_pushButtonGenerateIntersections_clicked() {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	std::vector<unsigned int> addedNodes, addedEdges;
	network.updateNodeEdgeConnectionPointers();
	network.generateIntersections(project().nextUnusedID(), addedNodes, addedEdges);
	network.updateNodeEdgeConnectionPointers();

	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropEditNetwork::on_pushButtonConnectBuildings_clicked() {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);

//	QMessageBox::StandardButton reply;
//	reply = QMessageBox::question(this, "Connect sub stations", "Shall the pipes be extended?", QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
//	bool extendPipes = reply == QMessageBox::Yes;

	network.updateNodeEdgeConnectionPointers();
	network.connectBuildings(project().nextUnusedID(), false);

	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropEditNetwork::on_pushButtonReduceRedundantNodes_clicked() {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);

	// set current network invisible
	network.setVisible(false);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views

	// make copy with reduced nodes
	VICUS::Network newNetwork(network);
	newNetwork.m_id = project().nextUnusedID();
	newNetwork.m_edges.clear();
	newNetwork.m_nodes.clear();
	newNetwork.m_displayName = QString("%1_noRedundants").arg(network.m_displayName);
	newNetwork.setVisible(true);

	// algorithm
	network.updateNodeEdgeConnectionPointers();
	network.cleanRedundantEdges(project().nextUnusedID(), newNetwork);
	newNetwork.updateNodeEdgeConnectionPointers();
	newNetwork.updateExtends();

	SVUndoAddNetwork * undoAdd = new SVUndoAddNetwork(tr("modified network"), newNetwork);
	undoAdd->push(); // modifies project and updates views

	int idx = m_ui->comboBoxCurrentNetwork->findData(newNetwork.m_id);
	m_ui->comboBoxCurrentNetwork->setCurrentIndex(idx);
}


void SVPropEditNetwork::on_pushButtonReduceDeadEnds_clicked() {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);

	network.updateNodeEdgeConnectionPointers();
	network.cleanDeadEnds();
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropEditNetwork::on_pushButtonRemoveSmallEdge_clicked() {
	double threshold = 0;
	if (m_ui->lineEditThresholdSmallEdge->isValid())
		threshold = m_ui->lineEditThresholdSmallEdge->value();
	else
		return;

	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);

	// make a copy which will keep the original network data
	VICUS::Network reducedNetwork(network);
	reducedNetwork.m_displayName = QString("%1_noShortEdges").arg(m_currentNetwork->m_displayName);
	reducedNetwork.m_id = project().nextUnusedID();
	// TODO : Hauke, das kopierte Netzwerk enthält noch Edges/Nodes mit alten IDs, diese
	//        müssen zwingend auch neue IDs erhalten, und die Verknüpfungen untereinander müssen auch
	//        korrigiert werden!!!!
	network.updateNodeEdgeConnectionPointers();
	network.setVisible(false);
	reducedNetwork.setVisible(false);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views

	// now modify the current network (new id, new name)
	reducedNetwork.removeShortEdges(threshold);
	reducedNetwork.setVisible(true);
	reducedNetwork.updateExtends();

	int idx = m_ui->comboBoxCurrentNetwork->findData(reducedNetwork.m_id);
	m_ui->comboBoxCurrentNetwork->setCurrentIndex(idx);
}


void SVPropEditNetwork::on_pushButtonSelectPipes_clicked() {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);

	SVNetworkDialogSelectPipes *dialog = new SVNetworkDialogSelectPipes(this);
	dialog->edit(network);

	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropEditNetwork::on_pushButtonSizePipeDimensions_clicked() {
	FUNCID(SVPropNetworkEditWidget::on_pushButtonSizePipeDimensions_clicked);

	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);

	const SVDatabase & db = SVSettings::instance().m_db;
	const VICUS::NetworkFluid * fluid = db.m_fluids[network.m_idFluid];
	try {
		// check for fluid
		if (fluid == nullptr)
			throw IBK::Exception(tr("Could not find fluid with id #%1 in fluid database")
								.arg(network.m_idFluid).toStdString(), FUNC_ID);
		// filter out list of available pipes
		std::vector<const VICUS::NetworkPipe*> availablePipes;
		for (unsigned int pipeID : network.m_availablePipes) {
			const VICUS::NetworkPipe * pipe = db.m_pipes[pipeID];
			if (pipe == nullptr) // skip unavailable/undefined pipes
				continue;
			availablePipes.push_back(pipe);
		}

		// run algorithm
		network.sizePipeDimensions(fluid, availablePipes);

	}  catch (IBK::Exception &ex) {
		QMessageBox::critical(this, tr("Error sizing pipes"), ex.what());
	}

	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropEditNetwork::on_comboBoxCurrentNetwork_currentIndexChanged(int /*index*/) {
	updateUi();
}


void SVPropEditNetwork::on_lineEditMaxPressureDrop_editingFinishedSuccessfully() {
	 if (!m_ui->lineEditMaxPressureDrop->isValid())
		 return;
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	VICUS::KeywordList::setParameter(network.m_para, "Network::para_t", VICUS::Network::P_MaxPressureLoss,
									 m_ui->lineEditMaxPressureDrop->value());
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropEditNetwork::on_lineEditTemperatureSetpoint_editingFinishedSuccessfully() {
	if (!m_ui->lineEditTemperatureSetpoint->isValid())
		return;
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	VICUS::KeywordList::setParameter(network.m_para, "Network::para_t", VICUS::Network::P_TemperatureSetpoint,
									 m_ui->lineEditTemperatureSetpoint->value());
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropEditNetwork::on_lineEditTemperatureDifference_editingFinishedSuccessfully() {
	if (!m_ui->lineEditTemperatureDifference->isValid())
		return;
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	VICUS::KeywordList::setParameter(network.m_para, "Network::para_t", VICUS::Network::P_TemperatureDifference,
									 m_ui->lineEditTemperatureDifference->value());
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}
