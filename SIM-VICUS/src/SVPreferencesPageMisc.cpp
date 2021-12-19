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

#include "SVPreferencesPageMisc.h"
#include "ui_SVPreferencesPageMisc.h"

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVMainWindow.h"

SVPreferencesPageMisc::SVPreferencesPageMisc(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPreferencesPageMisc)
{
	m_ui->setupUi(this);
}


SVPreferencesPageMisc::~SVPreferencesPageMisc() {
	delete m_ui;
}


void SVPreferencesPageMisc::updateUi() {
	SVSettings & s = SVSettings::instance();
	// transfer data to Ui
	m_ui->checkBoxDontUseNativeDialogs->blockSignals(true);
	m_ui->checkBoxDontUseNativeDialogs->setChecked(s.m_dontUseNativeDialogs);
	m_ui->checkBoxDontUseNativeDialogs->blockSignals(false);
}


void SVPreferencesPageMisc::on_checkBoxDontUseNativeDialogs_toggled(bool checked) {
	SVSettings & s = SVSettings::instance();
	s.m_dontUseNativeDialogs = checked;
	QMessageBox::information(this, QString(), tr("Please restart application!"));
}


void SVPreferencesPageMisc::on_pushButtonResetDoNotShowAgainDialogs_clicked() {
	SVSettings & s = SVSettings::instance();
	for (QMap<QString, bool>::iterator it = s.m_doNotShowAgainDialogs.begin(); it != s.m_doNotShowAgainDialogs.end(); ++it) {
		(*it) = false;
	}
	QMessageBox::information(this, QString(), tr("All confirmations have been disabled and information/confirmation dialogs will pop up again."));
}
