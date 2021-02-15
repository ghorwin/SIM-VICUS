#include "SVSmartSelectDialog.h"
#include "ui_SVSmartSelectDialog.h"

#include <QPushButton>
#include <QDialogButtonBox>
#include <VICUS_KeywordList.h>
#include <VICUS_NetworkNode.h>
#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVUndoTreeNodeState.h"

SVSmartSelectDialog::SVSmartSelectDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSmartSelectDialog)
{
	m_ui->setupUi(this);

	m_ui->verticalLayoutNetwork->setMargin(0);

	QPushButton * btn = new QPushButton(tr("Select"));
	m_ui->buttonBox->addButton(btn, QDialogButtonBox::AcceptRole);
	connect(btn, &QPushButton::clicked, this, &SVSmartSelectDialog::onSelectClicked);

	// populate combo boxes

	m_ui->comboBoxNetwork->clear();
	m_ui->comboBoxNetwork->addItem(tr("<all networks>"), -1);
	for (const VICUS::Network & nw : project().m_geometricNetworks)
		m_ui->comboBoxNetwork->addItem(QString::fromStdString(nw.m_name), nw.m_id);
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

	m_ui->lineEditMaxHeatingDemandBelow->setEnabled(m_ui->checkBoxMaxHeatingDemandBelow->isChecked());
	m_ui->lineEditMaxHeatingDemandAbove->setEnabled(m_ui->checkBoxMaxHeatingDemandAbove->isChecked());
}


SVSmartSelectDialog::~SVSmartSelectDialog() {
	delete m_ui;
}


void SVSmartSelectDialog::onSelectClicked() {
	// evaluate selection and create undo action for the selection
	if (m_ui->tabWidgetGroup->currentIndex() == 0) {
		// TODO : Building object selections
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

					if (n.m_maxHeatingDemand > maxHeatingDemandBelowFilter)
						continue;

					if (n.m_maxHeatingDemand < maxHeatingDemandAboveFilter)
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

	// when this even is done, the dialog is closed automatically
}


void SVSmartSelectDialog::on_comboBoxNodeType_currentIndexChanged(int index)
{
	bool noBuilding = index==1 || index==2;
	m_ui->checkBoxMaxHeatingDemandAbove->setEnabled(!noBuilding);
	m_ui->lineEditMaxHeatingDemandAbove->setEnabled(!noBuilding);
	m_ui->checkBoxMaxHeatingDemandBelow->setEnabled(!noBuilding);
	m_ui->lineEditMaxHeatingDemandBelow->setEnabled(!noBuilding);
	if (noBuilding){
		m_ui->checkBoxMaxHeatingDemandBelow->setChecked(false);
		m_ui->checkBoxMaxHeatingDemandAbove->setChecked(false);
	}
}

void SVSmartSelectDialog::on_checkBoxMaxHeatingDemandBelow_clicked(bool checked)
{
	m_ui->lineEditMaxHeatingDemandBelow->setEnabled(checked);
}

void SVSmartSelectDialog::on_checkBoxMaxHeatingDemandAbove_clicked(bool checked)
{
	m_ui->lineEditMaxHeatingDemandAbove->setEnabled(checked);
}

void SVSmartSelectDialog::on_checkBoxLengthBelow_clicked(bool checked)
{
	m_ui->lineEditLengthBelow->setEnabled(checked);
}

void SVSmartSelectDialog::on_checkBoxLengthAbove_clicked(bool checked)
{
	m_ui->lineEditLengthAbove->setEnabled(checked);
}
