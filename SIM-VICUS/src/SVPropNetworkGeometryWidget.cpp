#include "SVPropNetworkGeometryWidget.h"
#include "ui_SVPropNetworkGeometryWidget.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"
#include "SVUndoModifyNetwork.h"
#include "SVUndoAddNetwork.h"
#include "SVNetworkDialogSelectPipes.h"
#include "SVNetworkSimultaneityDialog.h"

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include <QtExt_Conversions.h>


SVPropNetworkGeometryWidget::SVPropNetworkGeometryWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditNetwork)
{
	m_ui->setupUi(this);

	// validating line edits
	m_ui->lineEditThresholdSmallEdge->setup(std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), tr("Pipes shorter than this value will be removed"), true, true);

	// hide some features for now
	m_ui->pushButtonReduceRedundantNodes->setVisible(false);
	m_ui->groupBoxRemoveShortEdges->setVisible(false);

	m_iconConnected = QPixmap(":/gfx/actions/16x16/ok.png");
	m_iconUnconnected = QPixmap(":/gfx/actions/16x16/error.png");
}

SVPropNetworkGeometryWidget::~SVPropNetworkGeometryWidget() {
	delete m_ui;
}


void SVPropNetworkGeometryWidget::updateUi() {

	m_ui->labelEdgeCount->clear();
	m_ui->labelNodeCount->clear();
	m_ui->labelSubStationCount->clear();
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
	m_ui->groupBoxVisualisation->setEnabled(haveNetwork);
	if (!haveNetwork)
		return;

	m_ui->labelEdgeCount->setText(QString("%1").arg(m_currentNetwork->m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(m_currentNetwork->m_nodes.size()));
	// count sub stations
	unsigned int subStationCount = 0;
	for (const VICUS::NetworkNode &n: m_currentNetwork->m_nodes) {
		if (n.m_type == VICUS::NetworkNode::NT_SubStation)
			subStationCount++;
	}
	m_ui->labelSubStationCount->setText(QString("%1").arg(subStationCount));


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

	if (m_currentNetwork->checkConnectedGraph())
		m_ui->labelConnectedSymbol->setPixmap(m_iconConnected);
	else
		m_ui->labelConnectedSymbol->setPixmap(m_iconUnconnected);

	m_ui->labelTotalLength->setText(QString("%1 m").arg(m_currentNetwork->totalLength()));
	m_ui->pushButtonConnectBuildings->setEnabled(m_currentNetwork->nextUnconnectedBuilding()>=0);
	m_ui->pushButtonReduceDeadEnds->setEnabled(m_currentNetwork->checkConnectedGraph() && m_currentNetwork->numberOfBuildings() > 0);

	m_ui->lineEditMaxPressureDrop->setValue(m_currentNetwork->m_para[VICUS::Network::P_MaxPressureLoss].value);
	m_ui->lineEditTemperatureDifference->setValue(m_currentNetwork->m_para[VICUS::Network::P_TemperatureDifference].value);
	if (!m_currentNetwork->m_para[VICUS::Network::P_TemperatureSetpoint].empty())
		m_ui->lineEditTemperatureSetpoint->setValue(m_currentNetwork->m_para[VICUS::Network::P_TemperatureSetpoint].get_value("C"));

	// scales
	m_ui->horizontalSliderScaleEdges->setValue((int)m_currentNetwork->m_scaleEdges);
	m_ui->horizontalSliderScaleNodes->setValue((int)m_currentNetwork->m_scaleNodes);
}


void SVPropNetworkGeometryWidget::updateComboBoxNetworks() {
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
//	else
//		m_ui->comboBoxCurrentNetwork->setCurrentIndex(0);
	m_ui->comboBoxCurrentNetwork->blockSignals(false);
}


void SVPropNetworkGeometryWidget::setCurrentNetwork(unsigned int networkId) {
	int idx = m_ui->comboBoxCurrentNetwork->findData(networkId);
	m_ui->comboBoxCurrentNetwork->setCurrentIndex(idx);
}


void SVPropNetworkGeometryWidget::on_horizontalSliderScaleNodes_valueChanged(int value) {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	network.m_scaleNodes = value;
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkGeometryWidget::on_horizontalSliderScaleEdges_valueChanged(int value) {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	network.m_scaleEdges= value;
	const VICUS::Project p = project();
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkGeometryWidget::on_pushButtonSelectFluid_clicked() {
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


void SVPropNetworkGeometryWidget::on_pushButtonGenerateIntersections_clicked() {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	std::vector<unsigned int> addedNodes, addedEdges;
	network.updateNodeEdgeConnectionPointers();
	network.generateIntersections(project().nextUnusedID(), addedNodes, addedEdges);
	network.updateNodeEdgeConnectionPointers();

	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkGeometryWidget::on_pushButtonConnectBuildings_clicked() {
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


void SVPropNetworkGeometryWidget::on_pushButtonReduceRedundantNodes_clicked() {
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


void SVPropNetworkGeometryWidget::on_pushButtonReduceDeadEnds_clicked() {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);

	network.updateNodeEdgeConnectionPointers();
	network.cleanDeadEnds();
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkGeometryWidget::on_pushButtonRemoveSmallEdge_clicked() {
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


void SVPropNetworkGeometryWidget::on_comboBoxCurrentNetwork_currentIndexChanged(int /*index*/) {
	unsigned int netId = m_ui->comboBoxCurrentNetwork->currentData().toUInt();
	m_currentNetwork = VICUS::element(project().m_geometricNetworks, netId);
	updateUi();
}



void SVPropNetworkGeometryWidget::on_pushButtonSelectPipes_clicked() {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	SVNetworkDialogSelectPipes *dialog = new SVNetworkDialogSelectPipes(this);
	dialog->edit(network);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkGeometryWidget::on_lineEditMaxPressureDrop_editingFinishedSuccessfully() {
	if (!m_ui->lineEditMaxPressureDrop->isValid())
		return;
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
   VICUS::KeywordList::setParameter(network.m_para, "Network::para_t", VICUS::Network::P_MaxPressureLoss,
									m_ui->lineEditMaxPressureDrop->value());
   SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
   undo->push(); // modifies project and updates views
}


void SVPropNetworkGeometryWidget::on_lineEditTemperatureSetpoint_editingFinishedSuccessfully() {
	if (!m_ui->lineEditTemperatureSetpoint->isValid())
		return;
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	VICUS::KeywordList::setParameter(network.m_para, "Network::para_t", VICUS::Network::P_TemperatureSetpoint,
									 m_ui->lineEditTemperatureSetpoint->value());
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkGeometryWidget::on_lineEditTemperatureDifference_editingFinishedSuccessfully() {
	if (!m_ui->lineEditTemperatureDifference->isValid())
		return;
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);
	VICUS::KeywordList::setParameter(network.m_para, "Network::para_t", VICUS::Network::P_TemperatureDifference,
									 m_ui->lineEditTemperatureDifference->value());
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkGeometryWidget::on_pushButtonSizePipeDimensions_clicked() {
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

void SVPropNetworkGeometryWidget::on_pushButtonEditSimultaneity_clicked() {
	Q_ASSERT(VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id) != nullptr);
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, m_currentNetwork->m_id);

	SVNetworkSimultaneityDialog *diag = new SVNetworkSimultaneityDialog();
	diag->edit(network.m_simultaneity);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network simultaneity updated"), network);
	undo->push(); // modifies project and updates views
}

