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

#include "SVDBInternalLoadsPersonEditWidget.h"
#include "ui_SVDBInternalLoadsPersonEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <SVConversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBInternalLoadsTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"
#include "SVSettings.h"
#include "SVChartUtils.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>


SVDBInternalLoadsPersonEditWidget::SVDBInternalLoadsPersonEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBInternalLoadsPersonEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	// *** populate combo boxes ***

	m_ui->comboBoxPersonMethod->blockSignals(true);

	for (unsigned int i=0; i<VICUS::InternalLoad::NUM_PCM; ++i) {
		m_ui->comboBoxPersonMethod->addItem(QString("%1").arg(VICUS::KeywordListQt::Description("InternalLoad::PersonCountMethod", (int)i)));
		//.arg(VICUS::KeywordListQt::Keyword("InternalLoad::PersonCountMethod", (int)i)), i);
	}
	m_ui->comboBoxPersonMethod->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Internal loads person model name"));

	m_ui->lineEditPersonCount->setup(0, 10000, tr("Person count according to the given unit"), true, true);
	m_ui->lineEditConvectiveFactor->setup(0, 1, tr("Convective heat factor"), true, true);

	configureChart(m_ui->widgetPlot);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBInternalLoadsPersonEditWidget::~SVDBInternalLoadsPersonEditWidget() {
	delete m_ui;
}


void SVDBInternalLoadsPersonEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBInternalLoadsTableModel*>(dbModel);
}


void SVDBInternalLoadsPersonEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditPersonCount->setText("-");
		m_ui->lineEditConvectiveFactor->setText("-");
		return;
	}

	m_current = const_cast<VICUS::InternalLoad *>(m_db->m_internalLoads[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	//change invalid person count method to a valid one
	if (m_current->m_personCountMethod == VICUS::InternalLoad::PersonCountMethod::NUM_PCM){
		m_current->m_personCountMethod = VICUS::InternalLoad::PersonCountMethod::PCM_PersonCount;
		m_db->m_internalLoads.m_modified=true;
	}

	m_ui->comboBoxPersonMethod->blockSignals(true);
	m_ui->comboBoxPersonMethod->setCurrentIndex(m_current->m_personCountMethod);
	m_ui->comboBoxPersonMethod->blockSignals(false);

	m_ui->lineEditConvectiveFactor->setValue(m_current->m_para[VICUS::InternalLoad::P_ConvectiveHeatFactor].value);

	switch (m_current->m_personCountMethod) {
		case VICUS::InternalLoad::PCM_AreaPerPerson:{
			m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_AreaPerPerson].value);

		}break;
		case VICUS::InternalLoad::PCM_PersonPerArea: {
			m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_PersonPerArea].value);
		}break;
		case VICUS::InternalLoad::PCM_PersonCount:
		case VICUS::InternalLoad::NUM_PCM:{
			m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_PersonCount].value);
		}break;
	}
	m_ui->labelPersonCount->setText(tr("Max. person count [%1]:").arg(VICUS::KeywordListQt::Description("InternalLoad::PersonCountMethod", m_current->m_personCountMethod)));

	VICUS::Schedule * occSched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idOccupancySchedule ]);
	if (occSched != nullptr)
		m_ui->lineEditOccupancyScheduleName->setText(QtExt::MultiLangString2QString(occSched->m_displayName));
	else
		m_ui->lineEditOccupancyScheduleName->setText(tr("<select schedule>"));

	VICUS::Schedule * actSched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idActivitySchedule ]);
	if (actSched != nullptr)
		m_ui->lineEditActivityScheduleName->setText(QtExt::MultiLangString2QString(actSched->m_displayName));
	else
		m_ui->lineEditActivityScheduleName->setText(tr("<select schedule>"));

	VICUS::Schedule * moistSched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_idMoistureProductionRatePerAreaSchedule ]);
	if (moistSched != nullptr)
		m_ui->lineEditMoistureRateScheduleName->setText(QtExt::MultiLangString2QString(moistSched->m_displayName));
	else
		m_ui->lineEditMoistureRateScheduleName->setText(tr("<select schedule>"));

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxPersonMethod->setEnabled(!isbuiltIn);
	m_ui->lineEditPersonCount->setEnabled(!isbuiltIn);
	m_ui->lineEditConvectiveFactor->setEnabled(!isbuiltIn);
	m_ui->lineEditActivityScheduleName->setEnabled(!isbuiltIn);
	m_ui->lineEditMoistureRateScheduleName->setEnabled(!isbuiltIn);
	m_ui->lineEditOccupancyScheduleName->setEnabled(!isbuiltIn);
	m_ui->toolButtonRemoveActivitySchedule->setEnabled(!isbuiltIn);
	m_ui->toolButtonRemoveOccupancySchedule->setEnabled(!isbuiltIn);

	updatePlot();
}


void SVDBInternalLoadsPersonEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBInternalLoadsPersonEditWidget::on_comboBoxPersonMethod_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::InternalLoad::PersonCountMethod::NUM_PCM; ++i){
		if(index == i){
			m_current->m_personCountMethod = static_cast<VICUS::InternalLoad::PersonCountMethod>(i);
			modelModify();
			switch (m_current->m_personCountMethod) {
				case VICUS::InternalLoad::PCM_PersonPerArea:
					m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_PersonPerArea].value);
				break;
				case VICUS::InternalLoad::PCM_AreaPerPerson:
					m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_AreaPerPerson].value);
				break;
				case VICUS::InternalLoad::PCM_PersonCount:
				case VICUS::InternalLoad::NUM_PCM:
					m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_PersonCount].value);
				break;

			}
			break;
		}
	}

	m_ui->labelPersonCount->setText(tr("Max. person count [%1]:").arg(VICUS::KeywordListQt::Description("InternalLoad::PersonCountMethod", index)));

	updatePlot();
}


void SVDBInternalLoadsPersonEditWidget::on_lineEditPersonCount_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditPersonCount->value();
	// update database but only if different from original
	VICUS::InternalLoad::para_t paraName;
	switch (m_current->m_personCountMethod) {
		case VICUS::InternalLoad::PCM_PersonPerArea:				paraName = VICUS::InternalLoad::P_PersonPerArea;	break;
		case VICUS::InternalLoad::PCM_AreaPerPerson:				paraName = VICUS::InternalLoad::P_AreaPerPerson;	break;
		case VICUS::InternalLoad::PCM_PersonCount:					paraName = VICUS::InternalLoad::P_PersonCount;		break;
		case VICUS::InternalLoad::NUM_PCM:							paraName = VICUS::InternalLoad::NUM_P;				break;
	}
	if (m_current->m_para[paraName].empty() ||
		val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, "InternalLoad::para_t", paraName, val);
		modelModify();
		updatePlot();
	}
}


void SVDBInternalLoadsPersonEditWidget::on_lineEditConvectiveFactor_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditConvectiveFactor->value();
	// update database but only if different from original
	VICUS::InternalLoad::para_t paraName = VICUS::InternalLoad::P_ConvectiveHeatFactor;
	if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
	{
		VICUS::KeywordList::setParameter(m_current->m_para, "InternalLoad::para_t", paraName, val);
		modelModify();
	}
}


void SVDBInternalLoadsPersonEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBInternalLoadsPersonEditWidget::on_toolButtonSelectOccupancy_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idOccupancySchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idOccupancySchedule != newId) {
		m_current->m_idOccupancySchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);

}

void SVDBInternalLoadsPersonEditWidget::on_toolButtonSelectActivity_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idActivitySchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idActivitySchedule != newId) {
		m_current->m_idActivitySchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);

}

void SVDBInternalLoadsPersonEditWidget::on_toolButtonSelectMoistureRate_clicked()
{
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idMoistureProductionRatePerAreaSchedule);
	if (newId != VICUS::INVALID_ID && m_current->m_idMoistureProductionRatePerAreaSchedule != newId) {
		m_current->m_idMoistureProductionRatePerAreaSchedule = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}

void SVDBInternalLoadsPersonEditWidget::modelModify() {
	m_db->m_internalLoads.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	updateInput((int)m_current->m_id);
}


void SVDBInternalLoadsPersonEditWidget::updatePlot() {

	m_ui->widgetPlot->detachItems( QwtPlotItem::Rtti_PlotCurve );
	m_ui->widgetPlot->detachItems( QwtPlotItem::Rtti_PlotMarker );
	m_ui->widgetPlot->replot();
	m_ui->widgetPlot->setEnabled(false);

	if (m_current == nullptr)
		return;

	// multiply by scalar
	double factor = 1;
	QString label = " ";
	if (m_current->m_personCountMethod == VICUS::InternalLoad::PCM_PersonCount) {
		label = tr("Power [W]");
		if (!m_current->m_para[VICUS::InternalLoad::P_PersonCount].empty())
			factor = m_current->m_para[VICUS::InternalLoad::P_PersonCount].value;
	}
	else if (m_current->m_personCountMethod == VICUS::InternalLoad::PCM_PersonPerArea) {
		label = tr("Power per area [W/m²]");
		if (!m_current->m_para[VICUS::InternalLoad::PCM_PersonPerArea].empty())
			factor = m_current->m_para[VICUS::InternalLoad::PCM_PersonPerArea].value;
	}
	else if (m_current->m_personCountMethod == VICUS::InternalLoad::PCM_AreaPerPerson) {
			label = tr("Power per area [W/m²]");
			if (!m_current->m_para[VICUS::InternalLoad::PCM_AreaPerPerson].empty())
				factor = 1 / m_current->m_para[VICUS::InternalLoad::PCM_AreaPerPerson].value;
	}

	// check schedules, if invalid return
	const VICUS::Schedule *sched1 = m_db->m_schedules[m_current->m_idActivitySchedule];
	const VICUS::Schedule *sched2 = m_db->m_schedules[m_current->m_idOccupancySchedule];
	if (sched1==nullptr || sched2==nullptr)
		return;

	std::vector<double> time1, time2, vals1, vals2;
	createDataVectorFromSchedule(*sched1, time1, vals1);
	createDataVectorFromSchedule(*sched2, time2, vals2);
	if ( (time1.size() != time2.size()) || (vals1.size() != vals2.size()) )
		 return;

	m_xData.clear();
	m_yData.clear();
	for (unsigned int i=0; i<time1.size(); ++i){
		// this must not happen, maybe throw an error message here?
		if (time1[i] != time2[i])
			return;
		m_xData.push_back(time1[i]);
		m_yData.push_back( vals1[i] * vals2[i] * factor );
	}

	// dont plot if there is no data
	if (m_xData.size()==0)
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
	QwtText yl(label);
	yl.setFont(ft);
	m_ui->widgetPlot->setAxisTitle(QwtPlot::yLeft, yl);
	m_ui->widgetPlot->replot();
}


void SVDBInternalLoadsPersonEditWidget::createDataVectorFromSchedule(const VICUS::Schedule & sched, std::vector<double> & time, std::vector<double> & vals) {

	// we dont consider annual schedule here
	if (sched.m_periods.size()>0) {
		if (sched.m_periods[0].isValid())
			sched.m_periods[0].createWeekDataVector(time, vals);
	}
	else
		return;

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

void SVDBInternalLoadsPersonEditWidget::on_toolButtonRemoveOccupancySchedule_clicked() {

	m_current->m_idOccupancySchedule = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}

void SVDBInternalLoadsPersonEditWidget::on_toolButtonRemoveActivitySchedule_clicked() {

	m_current->m_idActivitySchedule = VICUS::INVALID_ID;

	modelModify();
	updateInput((int)m_current->m_id);
}

