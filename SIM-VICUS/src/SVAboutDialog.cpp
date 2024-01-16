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

#include "SVAboutDialog.h"
#include "ui_SVAboutDialog.h"

#include <VICUS_Constants.h>

#include <QtExt_LanguageHandler.h>

#include <QRandomGenerator>
#include <QIcon>

#include "SVStyle.h"
#include "SVSettings.h"

SVAboutDialog::SVAboutDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVAboutDialog)
{
	m_ui->setupUi(this);

	setWindowTitle(QString("SIM-VICUS %1").arg(VICUS::LONG_VERSION));

	QPixmap pixmap = QIcon::fromTheme("simvicus_logo").pixmap(300);
	m_ui->label->setPixmap(pixmap);
}


SVAboutDialog::~SVAboutDialog() {
	delete m_ui;
}

