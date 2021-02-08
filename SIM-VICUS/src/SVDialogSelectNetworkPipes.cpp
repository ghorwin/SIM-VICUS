#include "SVDialogSelectNetworkPipes.h"
#include "ui_SVDialogSelectNetworkPipes.h"

#include "SVSettings.h"

#include <VICUS_Network.h>

SVDialogSelectNetworkPipes::SVDialogSelectNetworkPipes(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDialogSelectNetworkPipes)
{
	m_ui->setupUi(this);
}

SVDialogSelectNetworkPipes::~SVDialogSelectNetworkPipes()
{
	delete m_ui;
}

void SVDialogSelectNetworkPipes::edit(VICUS::Network &network)
{
	if (exec() == QDialog::Accepted){

		// TODO Hauke: implement list widget

		// add all pipes
		network.m_availablePipes.clear();
		const SVDatabase & db = SVSettings::instance().m_db;
		for (auto pipe = db.m_pipes.begin(); pipe != db.m_pipes.end(); ++pipe)
			network.m_availablePipes.push_back(pipe->second.m_id);

	}

}
