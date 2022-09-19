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

#include "SVDBVentilationNaturalEditWidget.h"
#include "ui_SVDBVentilationNaturalEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <SV_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBVentilationNaturalTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"
#include "SVSettings.h"
#include "SVChartUtils.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>


SVDBVentilationNaturalEditWidget::SVDBVentilationNaturalEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBVentilationNaturalEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Zone natural ventilation model name"));

	m_ui->lineEditAirChangeRate->setup(0, 50, tr("Air change rate."), true, true);

	configureChart(m_ui->widgetPlot);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBVentilationNaturalEditWidget::~SVDBVentilationNaturalEditWidget() {
	delete m_ui;
}


void SVDBVentilationNaturalEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBVentilationNaturalTableModel*>(dbModel);
}


void SVDBVentilationNaturalEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->labelAirChangeRate->setText(tr("Air change rate:"));
	m_ui->labelScheduleHeating->setText(tr("Schedule name:"));

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditAirChangeRate->setText("");
		m_ui->lineEditScheduleName->setText("");
		return;
	}

	m_current = const_cast<VICUS::VentilationNatural *>(m_db->m_ventilationNatural[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);

	try {
		m_ui->lineEditAirChangeRate->setValue(m_current->m_para[VICUS::VentilationNatural::P_AirChangeRate].get_value(IBK::Unit("1/h")));
	}  catch (IBK::Exception &ex) {
		m_ui->lineEditAirChangeRate->setValue(0);
	}

	VICUS::Schedule * sched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idSchedule]);
	if (sched != nullptr)
		m_ui->lineEditScheduleName->setText(QtExt::MultiLangString2QString(sched->m_displayName));
	else
		m_ui->lineEditScheduleName->setText(tr("<select schedule>"));

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->toolButtonSelectSchedule->setEnabled(!isbuiltIn);
	m_ui->lineEditScheduleName->setEnabled(!isbuiltIn);

	m_ui->lineEditAirChangeRate->setEnabled(!isbuiltIn);

	updatePlot();
}


void SVDBVentilationNaturalEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}

void SVDBVentilationNaturalEditWidget::on_lineEditAirChangeRate_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditAirChangeRate;
	VICUS::VentilationNatural::para_t paraName = VICUS::VentilationNatural::P_AirChangeRate;
	std::string keywordList = "VentilationNatural::para_t";

	double val = lineEdit->value();

	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
		modelModify();
		updatePlot();
	}
}

void SVDBVentilationNaturalEditWidget::modelModify() {
	m_db->m_ventilationNatural.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}

void SVDBVentilationNaturalEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}

void SVDBVentilationNaturalEditWidget::on_toolButtonSelectSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idSchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idSchedule != newId) {
		m_current->m_idSchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBVentilationNaturalEditWidget::updatePlot() {

	const VICUS::Schedule *sched = m_db->m_schedules[m_current->m_idSchedule];

	if (sched==nullptr)
		return;

	m_xData.clear();
	m_yData.clear();
	if (sched->m_haveAnnualSchedule) {
			m_xData = sched->m_annualSchedule.m_values.x();
			m_yData = sched->m_annualSchedule.m_values.y();
	}
	else if (sched->m_periods.size()>0 && sched->m_periods[0].isValid()) {
		sched->m_periods[0].createWeekDataVector(m_xData, m_yData);
	}
	else
		return;

	// multiply by air change rate
	if (!m_current->m_para[VICUS::VentilationNatural::P_AirChangeRate].empty()) {
		for (double &y: m_yData)
			y *= m_current->m_para[VICUS::VentilationNatural::P_AirChangeRate].get_value("1/h");
	}

	// convert time points to days
	for (double & d : m_xData)
		d /= 24;

	// if we are in "constant" mode, duplicate values to get steps
	if (!sched->m_useLinearInterpolation) {
		std::vector<double> timepointsCopy;
		std::vector<double> dataCopy;
		for (unsigned int i = 0; i<m_xData.size(); ++i) {
			timepointsCopy.push_back(m_xData[i]);
			// get next time point, unless it is the last, in this case we use "end of week = 7 d"
			double nexttp = 7;
			if (i+1<m_xData.size())
				nexttp = m_xData[i+1]-2./(24*3600);
			timepointsCopy.push_back(nexttp);
			dataCopy.push_back(m_yData[i]);
			dataCopy.push_back(m_yData[i]);
		}
		// copy data to schedule
		m_xData.swap(timepointsCopy);
		m_yData.swap(dataCopy);
	}
	else {
		// special handling for time series with just one point (constant schedule for all days)
		if (m_xData.size() == 1) {
			m_xData.push_back(7);
			m_yData.push_back(m_yData.back());
		}
	}

	m_ui->widgetPlot->detachItems( QwtPlotItem::Rtti_PlotCurve );
	m_ui->widgetPlot->detachItems( QwtPlotItem::Rtti_PlotMarker );
	m_ui->widgetPlot->replot();
	m_ui->widgetPlot->setEnabled(false);
	if (m_current == nullptr)
		return;

	// now do all the plotting
	m_ui->widgetPlot->setEnabled(true);

	m_curve = addConfiguredCurve(m_ui->widgetPlot);
	// adjust styling based on current theme's settings
	configureCurveTheme(m_curve);

	// heating curve
	m_curve->setRawSamples(m_xData.data(), m_yData.data(), (int)m_xData.size());

	QFont ft;
	ft.setPointSize(10);
	QwtText xl(tr("Time [d]"));
	xl.setFont(ft);
	m_ui->widgetPlot->setAxisTitle(QwtPlot::xBottom, xl);
	QwtText yl(tr("Natural Ventilation [1/h]"));
	yl.setFont(ft);
	m_ui->widgetPlot->setAxisTitle(QwtPlot::yLeft, yl);
	m_ui->widgetPlot->replot();
}



