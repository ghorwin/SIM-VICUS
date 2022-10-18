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

#include "SVDBScheduleAddDialog.h"
#include "ui_SVDBScheduleAddDialog.h"

#include "VICUS_Schedule.h"

#include "QtExt_LanguageHandler.h"
#include <SVConversions.h>

SVDBScheduleAddDialog::SVDBScheduleAddDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBScheduleAddDialog)
{
	m_ui->setupUi(this);

	m_ui->comboBoxInterpolationMethod->addItem(tr("linear") );
	m_ui->comboBoxInterpolationMethod->addItem(tr("constant") );

	m_ui->comboBoxScheduleType->addItem(tr("periodic (daily cycle based schemes)") );
	m_ui->comboBoxScheduleType->addItem(tr("continuous (e.g. measured data)") );
}

SVDBScheduleAddDialog::~SVDBScheduleAddDialog() {
	delete m_ui;
}

bool SVDBScheduleAddDialog::requestScheduleData(const QString &title, VICUS::Schedule &sched) {
	SVDBScheduleAddDialog dlg(nullptr); // top level

	if (title.isEmpty())
		dlg.setWindowTitle(qApp->applicationName());
	else
		dlg.setWindowTitle(title);

	dlg.m_ui->lineEditName->setString(sched.m_displayName);

	int res = dlg.exec();

	if (res != QDialog::Accepted)
		return false;
	else {
		sched.m_displayName = dlg.m_ui->lineEditName->string();
		if ( dlg.m_ui->comboBoxScheduleType->currentText() == "periodic (daily cycle based schemes)" ) {
			sched.m_periods.push_back(VICUS::ScheduleInterval() );
			sched.m_periods.back().m_displayName.setEncodedString("en:All year|de:Ganzes Jahr");
		}
		sched.m_useLinearInterpolation = (dlg.m_ui->comboBoxInterpolationMethod->currentText() == "linear");

		return true;
	}
}
