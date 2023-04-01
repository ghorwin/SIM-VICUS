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

#include "SVPreferencesDialog.h"
#include "ui_SVPreferencesDialog.h"

#include "SVPreferencesPageTools.h"
#include "SVPreferencesPageStyle.h"
#include "SVPreferencesPageMisc.h"
#include "SVViewStateHandler.h"
#include "SVMeasurementWidget.h"
#include "SVLocalCoordinateView.h"

#include <QStackedWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QShortcut>
#include <QDebug>

SVPreferencesDialog::SVPreferencesDialog(QWidget * parent) :
	QDialog(parent),
	m_ui(new Ui::SVPreferencesDialog)
{
	m_ui->setupUi(this);


	// setup configuration pages

	m_pageTools = new SVPreferencesPageTools(this);
	m_pageStyle = new SVPreferencesPageStyle(this);
	m_pageMisc = new SVPreferencesPageMisc(this);
	m_ui->tabWidget->addTab(m_pageTools, tr("External tools"));
	m_ui->tabWidget->addTab(m_pageStyle, tr("Style"));
	m_ui->tabWidget->addTab(m_pageMisc, tr("Miscellaneous"));

	// ... other pages

	// register to view state handler
	SVViewStateHandler::instance().m_preferencesDialog = this;
}


SVPreferencesDialog::~SVPreferencesDialog() {
	delete m_ui;
}


void SVPreferencesDialog::edit(int initialPage) {
	// transfer settings data to User Interface
	updateUi();

	m_ui->tabWidget->setCurrentIndex(initialPage);

	exec();
}


// ** private functions **

void SVPreferencesDialog::updateUi() {
	m_pageTools->updateUi();
	m_pageStyle->updateUi();
	m_pageMisc->updateUi();
	// ... other pages
}


