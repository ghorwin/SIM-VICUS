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

#include "SVDBZoneControlThermostatEditWidget.h"
#include "ui_SVDBZoneControlThermostatEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <SV_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBZoneControlThermostatTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"
#include "SVSettings.h"
#include "SVChartUtils.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>

SVDBZoneControlThermostatEditWidget::SVDBZoneControlThermostatEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBZoneControlThermostatEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridMasterLayout->setMargin(4);

	// *** populate combo boxes ***

	m_ui->comboBoxMethod->blockSignals(true);

	for (unsigned int i=0; i<VICUS::ZoneControlThermostat::NUM_CV; ++i) {
		m_ui->comboBoxMethod->addItem(QString("%1 [%2]")
									  .arg(VICUS::KeywordListQt::Description("ZoneControlThermostat::ControlValue", (int)i))
									  .arg(VICUS::KeywordListQt::Keyword("ZoneControlThermostat::ControlValue", (int)i)), i);
	}
	m_ui->comboBoxMethod->blockSignals(false);

	m_ui->comboBoxControllerType->blockSignals(true);

	for (unsigned int i=0; i<VICUS::ZoneControlThermostat::NUM_CT; ++i) {
		m_ui->comboBoxControllerType->addItem(QString("%1 [%2]")
											  .arg(VICUS::KeywordListQt::Description("ZoneControlThermostat::ControllerType", (int)i))
											  .arg(VICUS::KeywordListQt::Keyword("ZoneControlThermostat::ControllerType", (int)i)), i);
	}
	m_ui->comboBoxControllerType->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Zone control thermostat model name"));

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditTolerance->setup(0.1, 50, tr("Thermostat tolerance for heating and/or cooling mode. Min 0.1 K, Max 50 K."), true, true);

	// create chart
	configureChart(m_ui->widgetPlot);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBZoneControlThermostatEditWidget::~SVDBZoneControlThermostatEditWidget() {
	delete m_ui;
}


void SVDBZoneControlThermostatEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBZoneControlThermostatTableModel*>(dbModel);
}


void SVDBZoneControlThermostatEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditTolerance->setText("");
		m_ui->lineEditHeatingScheduleName->setText("");
		m_ui->lineEditCoolingScheduleName->setText("");
		return;
	}

	m_current = const_cast<VICUS::ZoneControlThermostat *>(m_db->m_zoneControlThermostat[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);


	//set method
	m_ui->comboBoxMethod->blockSignals(true);
	m_ui->comboBoxMethod->setCurrentIndex(m_current->m_controlValue);
	m_ui->comboBoxMethod->blockSignals(false);

	//set controller type
	m_ui->comboBoxControllerType->blockSignals(true);
	m_ui->comboBoxControllerType->setCurrentIndex(m_current->m_controllerType);
	m_ui->comboBoxControllerType->blockSignals(false);


	m_ui->lineEditTolerance->setValue(m_current->m_para[VICUS::ZoneControlThermostat::P_Tolerance].value);

	VICUS::Schedule * sched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idHeatingSetpointSchedule]);
	if (sched != nullptr)
		m_ui->lineEditHeatingScheduleName->setText(QtExt::MultiLangString2QString(sched->m_displayName));
	else
		m_ui->lineEditHeatingScheduleName->setText(tr("<select schedule>"));

	VICUS::Schedule * schedC = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idCoolingSetpointSchedule]);
	if (schedC != nullptr)
		m_ui->lineEditCoolingScheduleName->setText(QtExt::MultiLangString2QString(schedC->m_displayName));
	else
		m_ui->lineEditCoolingScheduleName->setText(tr("<select schedule>"));

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxMethod->setEnabled(!isbuiltIn);
	m_ui->comboBoxControllerType->setEnabled(!isbuiltIn);
	m_ui->lineEditHeatingScheduleName->setEnabled(!isbuiltIn);
	m_ui->lineEditCoolingScheduleName->setEnabled(!isbuiltIn);

	m_ui->toolButtonRemoveHeatingSetpointSchedule->setEnabled(!isbuiltIn);
	m_ui->toolButtonRemoveCoolingSetpointSchedule->setEnabled(!isbuiltIn);

	m_ui->lineEditTolerance->setEnabled(!isbuiltIn);

	updatePlot();
}


void SVDBZoneControlThermostatEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBZoneControlThermostatEditWidget::on_comboBoxControlValue_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::ZoneControlThermostat::ControlValue::NUM_CV; ++i){
		if(index == i){
			m_current->m_controlValue = static_cast<VICUS::ZoneControlThermostat::ControlValue>(i);
			modelModify();

		}
	}
}

void SVDBZoneControlThermostatEditWidget::on_comboBoxControllerType_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::ZoneControlThermostat::ControllerType::NUM_CT; ++i){
		if(index == i){
			m_current->m_controllerType= static_cast<VICUS::ZoneControlThermostat::ControllerType>(i);
			modelModify();

		}
	}
}


void SVDBZoneControlThermostatEditWidget::on_lineEditTolerance_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditTolerance->value();

	VICUS::ZoneControlThermostat::para_t paraName= VICUS::ZoneControlThermostat::P_Tolerance;
	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, "ZoneControlThermostat::para_t", paraName, val);
		modelModify();
	}

}


void SVDBZoneControlThermostatEditWidget::modelModify() {
	m_db->m_zoneControlThermostat.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}


void SVDBZoneControlThermostatEditWidget::updatePlot(){

	m_ui->widgetPlot->detachItems( QwtPlotItem::Rtti_PlotCurve );
	m_ui->widgetPlot->detachItems( QwtPlotItem::Rtti_PlotMarker );
	m_ui->widgetPlot->replot();
	m_ui->widgetPlot->setEnabled(false);

	m_xDataCooling.clear();
	m_xDataHeating.clear();
	m_yDataCooling.clear();
	m_yDataHeating.clear();

	if (m_current == nullptr)
		return;

	VICUS::Schedule *schedHeating = m_db->m_schedules[m_current->m_idHeatingSetpointSchedule];
	VICUS::Schedule *schedCooling = m_db->m_schedules[m_current->m_idCoolingSetpointSchedule];

	if(schedHeating != nullptr){
		createDataVectorFromSchedule(*schedHeating, m_xDataHeating, m_yDataHeating);
	}
	if(schedCooling != nullptr){
		createDataVectorFromSchedule(*schedCooling, m_xDataCooling, m_yDataCooling);

	}

	// now do all the plotting
	m_ui->widgetPlot->setEnabled(true);

	// heating curve
	if(m_xDataHeating.size()>1) {
		m_curveHeating = addConfiguredCurve(m_ui->widgetPlot);
		configureCurveTheme(m_curveHeating);
		m_curveHeating->setRawSamples(m_xDataHeating.data(), m_yDataHeating.data(), (int)m_xDataHeating.size());
		m_curveHeating->setTitle("Heating Curve");
		m_curveHeating->setPen("#9a031e", 2);
	}

	// cooling curve
	if(m_xDataCooling.size()>1) {
		m_curveCooling = addConfiguredCurve(m_ui->widgetPlot);
		configureCurveTheme(m_curveCooling);
		m_curveCooling->setRawSamples(m_xDataCooling.data(), m_yDataCooling.data(), (int)m_xDataCooling.size());
		m_curveCooling->setTitle("Cooling Curve");
		m_curveCooling->setPen("#3d5a80", 2);
	}

	QFont ft;
	ft.setPointSize(10);
	QwtText xl(tr("Ambient Temperature [C]"));
	xl.setFont(ft);
	m_ui->widgetPlot->setAxisTitle(QwtPlot::xBottom, xl);
	QwtText yl(tr("Supply Temperature [C]"));
	yl.setFont(ft);
	m_ui->widgetPlot->setAxisTitle(QwtPlot::yLeft, yl);
	m_ui->widgetPlot->replot();

	QwtLegend* legend2 = new QwtLegend;
	m_ui->widgetPlot->insertLegend(legend2, QwtPlot::TopLegend);

}


void SVDBZoneControlThermostatEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBZoneControlThermostatEditWidget::on_toolButtonSelectHeatingSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idHeatingSetpointSchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idHeatingSetpointSchedule != newId) {
		m_current->m_idHeatingSetpointSchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBZoneControlThermostatEditWidget::on_toolButtonSelectCoolingSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idCoolingSetpointSchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idCoolingSetpointSchedule != newId) {
		m_current->m_idCoolingSetpointSchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBZoneControlThermostatEditWidget::on_toolButtonRemoveHeatingSetpointSchedule_clicked() {

	m_current->m_idHeatingSetpointSchedule = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}


void SVDBZoneControlThermostatEditWidget::on_toolButtonRemoveCoolingSetpointSchedule_clicked() {

	m_current->m_idCoolingSetpointSchedule = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}


void SVDBZoneControlThermostatEditWidget::createDataVectorFromSchedule(const VICUS::Schedule & sched, std::vector<double> & time, std::vector<double> & vals) {

	// we dont consider annual schedule here
	if (sched.m_periods.empty() || !sched.m_periods[0].isValid())
		return;
	else {
		sched.m_periods[0].createWeekDataVector(time, vals);
	}

	// convert time points to days
	for (double & d : time)
		d /= 24;

	Q_ASSERT(time.size() == vals.size());
	// if we are in "constant" mode, duplicate values to get steps
	if (!sched.m_useLinearInterpolation) {
		std::vector<double> timepointsCopy;
		std::vector<double> dataCopy;
		for (unsigned int i = 0; i<time.size(); ++i) {
			timepointsCopy.push_back(time[i]);
			// get next time point, unless it is the last, in this case we use "end of week = 7 d"
			double nexttp = 7;
			if (i+1<time.size())
				nexttp = time[i+1]-2./(24*3600);
			timepointsCopy.push_back(nexttp);
			dataCopy.push_back(vals[i]);
			dataCopy.push_back(vals[i]);
		}
		// copy data to schedule
		time.swap(timepointsCopy);
		vals.swap(dataCopy);
	}
	else {
		// special handling for time series with just one point (constant schedule for all days)
		if (time.size() == 1) {
			time.push_back(7);
			vals.push_back(vals.back());
		}
	}
}

