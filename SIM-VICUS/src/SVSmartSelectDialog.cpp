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

	// pupulate combo boxes
	m_ui->comboBoxNodeType->addItem(tr("<all types>"));
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Mixer),
									VICUS::NetworkNode::NT_Mixer);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Source),
									VICUS::NetworkNode::NT_Source);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Building),
									VICUS::NetworkNode::NT_Building);
	m_ui->comboBoxNodeType->setCurrentIndex(0);

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

		// TODO : Hauke, add own selection rules below

		if (m_ui->tabWidgetNetworkSelections->currentIndex()==0) {
			// node selections

			VICUS::NetworkNode::NodeType nodeTypeFilter = VICUS::NetworkNode::NUM_NT; // no filter on node type
			if (m_ui->comboBoxNodeType->currentIndex() != 0)
				nodeTypeFilter = (VICUS::NetworkNode::NodeType)m_ui->comboBoxNodeType->currentData().toInt();

			// TODO : more filters


			// now process all network objects and store IDs of selected nodes
			std::set<unsigned int> nodeIDs;

			for (const VICUS::Network & nw : project().m_geometricNetworks) {
				for (const VICUS::NetworkNode & n : nw.m_nodes) {
					// apply filter rules

					// filter based on type
					if (nodeTypeFilter != VICUS::NetworkNode::NUM_NT && n.m_type != nodeTypeFilter)
						continue;

					// TODO : more filters

					// all filters match, store unique ID of this node
					nodeIDs.insert(n.uniqueID());
				}
			}

			SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selecting nodes"), SVUndoTreeNodeState::SelectedState, nodeIDs, true);
			undo->push();
		}
		else {
			// edge selections
		}
	}

	// when this even is done, the dialog is closed automatically
}
