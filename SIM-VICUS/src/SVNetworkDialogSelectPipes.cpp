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

#include "SVNetworkDialogSelectPipes.h"
#include "ui_SVDialogSelectNetworkPipes.h"

#include "SVSettings.h"

#include <VICUS_Network.h>

SVNetworkDialogSelectPipes::SVNetworkDialogSelectPipes(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDialogSelectNetworkPipes)
{
	m_ui->setupUi(this);
}

SVNetworkDialogSelectPipes::~SVNetworkDialogSelectPipes()
{
	delete m_ui;
}

void SVNetworkDialogSelectPipes::edit(VICUS::Network &network)
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
