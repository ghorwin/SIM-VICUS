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

#include "SVDBZoneControlVentilationNaturalEditWidget.h"
#include "ui_SVDBZoneControlVentilationNaturalEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <SV_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBZoneControlVentilationNaturalTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"
#include "SVSettings.h"

SVDBZoneControlVentilationNaturalEditWidget::SVDBZoneControlVentilationNaturalEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBZoneControlVentilationNaturalEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);


	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Zone control - natural ventilation model name"));

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditTemperatureAirRoomMaximum->setup(-100, 100, tr("Maximum room air temperature above which ventilation stops."), true, true);
	m_ui->lineEditTemperatureAirRoomMinimum->setup(-100, 100, tr("Minimum room air temperature below which ventilation stops."), true, true);
	m_ui->lineEditTemperatureAirOutsideMaximum->setup(-100, 100, tr("Maximum outside air temperature above which ventilation stops."), true, true);
	m_ui->lineEditTemperatureAirOutsideMinimum->setup(-100, 100, tr("Minimum outside air temperature below which ventilation stops."), true, true);
	m_ui->lineEditTemperatureDifference->setup(-100, 100, tr("Temperature Difference of Room - Outside. Is Difference lower ventilation stops."), true, true);
	m_ui->lineEditWindSpeedMax->setup(0, 40, tr("Maximum wind speed. Values above stops ventilation."), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBZoneControlVentilationNaturalEditWidget::~SVDBZoneControlVentilationNaturalEditWidget() {
	delete m_ui;
}


void SVDBZoneControlVentilationNaturalEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBZoneControlVentilationNaturalTableModel*>(dbModel);
}


void SVDBZoneControlVentilationNaturalEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->labelTemperatureAirRoomMaximum->setText(tr("Maximum room air Temperature:"));
	m_ui->labelTemperatureAirRoomMinimum->setText(tr("Minimum room air Temperature:"));
	m_ui->labelTemperatureAirOutsideMaximum->setText(tr("Maximum outside air Temperature:"));
	m_ui->labelTemperatureAirOutsideMinimum->setText(tr("Minimum outside air Temperature:"));
	m_ui->labelTemperatureDifference->setText(tr("Temperature difference (in - out):"));
	m_ui->labelWindSpeedMax->setText(tr("Maximum wind speed:"));

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditTemperatureAirOutsideMaximum->setText("");
		m_ui->lineEditTemperatureAirOutsideMinimum->setText("");
		m_ui->lineEditTemperatureAirRoomMaximum->setText("");
		m_ui->lineEditTemperatureAirRoomMinimum->setText("");
		m_ui->lineEditTemperatureDifference->setText("");
		m_ui->lineEditWindSpeedMax->setText("");
		return;
	}

	m_current = const_cast<VICUS::ZoneControlNaturalVentilation *>(m_db->m_zoneControlVentilationNatural[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	try {
		m_ui->lineEditTemperatureAirOutsideMaximum->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMax].get_value("C"));
	}  catch (IBK::Exception &) {
		m_ui->lineEditTemperatureAirOutsideMaximum->setValue(0);
	}
	try {
		m_ui->lineEditTemperatureAirOutsideMinimum->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMin].get_value("C"));
	}  catch (IBK::Exception &) {
		m_ui->lineEditTemperatureAirOutsideMinimum->setValue(0);
	}
	try {
		m_ui->lineEditTemperatureAirRoomMaximum->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMax].get_value("C"));

	}  catch (IBK::Exception &) {
		m_ui->lineEditTemperatureAirRoomMaximum->setValue(0);
	}
	try {
		m_ui->lineEditTemperatureAirRoomMinimum->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMin].get_value("C"));

	}  catch (IBK::Exception &) {
		m_ui->lineEditTemperatureAirRoomMinimum->setValue(0);
	}
	try {
		m_ui->lineEditTemperatureDifference->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureDifference].get_value("K"));

	}  catch (IBK::Exception &) {
		m_ui->lineEditTemperatureDifference->setValue(0);
	}
	try {
		m_ui->lineEditWindSpeedMax->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_WindSpeedMax].get_value("m/s"));

	}  catch (IBK::Exception &) {
		m_ui->lineEditWindSpeedMax->setValue(0);
	}

	//	VICUS::Schedule * sched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_heatingSetpointScheduleId]);
	//	if (sched != nullptr)
	//		m_ui->lineEditHeatingScheduleName->setText(QtExt::MultiLangString2QString(sched->m_displayName));
	//	else
	//		m_ui->lineEditHeatingScheduleName->setText(tr("<select schedule>"));


	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->lineEditTemperatureAirOutsideMaximum->setEnabled(!isbuiltIn);
	m_ui->lineEditTemperatureAirOutsideMinimum->setEnabled(!isbuiltIn);
	m_ui->lineEditTemperatureAirRoomMaximum->setEnabled(!isbuiltIn);
	m_ui->lineEditTemperatureAirRoomMinimum->setEnabled(!isbuiltIn);
	m_ui->lineEditTemperatureDifference->setEnabled(!isbuiltIn);
	m_ui->lineEditWindSpeedMax->setEnabled(!isbuiltIn);
}


void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditTemperatureAirOutsideMaximum_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditTemperatureAirOutsideMaximum;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMax;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	double val = lineEdit->value();

	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
		modelModify();
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditTemperatureAirOutsideMinimum_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditTemperatureAirOutsideMinimum;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMin;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	double val = lineEdit->value();

	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
		modelModify();
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditTemperatureAirRoomMaximum_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditTemperatureAirRoomMaximum;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMax;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	double val = lineEdit->value();

	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
		modelModify();
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditTemperatureAirRoomMinimum_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditTemperatureAirRoomMaximum;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMin;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	double val = lineEdit->value();

	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
		modelModify();
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditTemperatureDifference_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditTemperatureDifference;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_TemperatureDifference;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	double val = lineEdit->value();

	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
		modelModify();
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditWindSpeedMax_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditWindSpeedMax;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_WindSpeedMax;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	double val = lineEdit->value();

	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
		modelModify();

	}
}



void SVDBZoneControlVentilationNaturalEditWidget::modelModify() {
	m_db->m_zoneControlVentilationNatural.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}

void SVDBZoneControlVentilationNaturalEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}



