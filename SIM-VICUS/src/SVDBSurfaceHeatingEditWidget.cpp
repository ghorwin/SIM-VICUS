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

#include "SVDBSurfaceHeatingEditWidget.h"
#include "ui_SVDBSurfaceHeatingEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Locale.h>

#include "SVConversions.h"
#include "SVDBSurfaceHeatingTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"
#include "SVStyle.h"
#include "SVChartUtils.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>


SVDBSurfaceHeatingEditWidget::SVDBSurfaceHeatingEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBSurfaceHeatingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->masterLayout->setMargin(4);

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	// *** populate combo boxes ***

	m_ui->comboBoxType->blockSignals(true);

	for (unsigned int i=0; i<VICUS::SurfaceHeating::NUM_T; ++i) {
		m_ui->comboBoxType->addItem(QString("%1")
			.arg(VICUS::KeywordListQt::Description("SurfaceHeating::Type", (int)i)));
			//.arg(VICUS::KeywordListQt::Keyword("SurfaceHeating::Type", (int)i)), i);
	}
	m_ui->comboBoxType->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("SurfaceHeating Model Name"));

	m_ui->lineEditHeatingLimit->setup(0, 40000, tr("Maximum heating limit in W/m2."), true, true);
	m_ui->lineEditCoolingLimit->setup(0, 40000, tr("Maximum cooling limit in W/m2."), true, true);
	m_ui->lineEditPipeSpacing->setup(0, 5, tr("Maximum fluid velocity in m/s."), false, true);
	m_ui->lineEditTemperaturDifference->setup(0, 80, tr("Temperature difference of supply and return fluid."), false, true);

	// format table widgets
	m_ui->tableWidgetHeating->setColumnCount(2);
	m_ui->tableWidgetHeating->setHorizontalHeaderLabels(QStringList() << tr("Ambient Temperature [C]")<< tr("Supply Temperature [C]") );
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetHeating);
	m_ui->tableWidgetHeating->setSortingEnabled(false);
	m_ui->tableWidgetHeating->setColumnWidth(0, 200);
	m_ui->tableWidgetHeating->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	m_ui->tableWidgetCooling->setColumnCount(2);
	m_ui->tableWidgetCooling->setHorizontalHeaderLabels(QStringList() << tr("Ambient Temperature [C]")<< tr("Supply Temperature [C]") );
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetCooling);
	m_ui->tableWidgetCooling->setSortingEnabled(false);
	m_ui->tableWidgetCooling->setColumnWidth(0, 200);
	m_ui->tableWidgetCooling->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	// create chart
	configureChart(m_ui->widgetPlotHeatingCurve);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBSurfaceHeatingEditWidget::~SVDBSurfaceHeatingEditWidget() {
	delete m_ui;
}


void SVDBSurfaceHeatingEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBSurfaceHeatingTableModel*>(dbModel);
}


void SVDBSurfaceHeatingEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditCoolingLimit->setText("");
		m_ui->lineEditHeatingLimit->setText("");
		m_ui->stackedWidget->setCurrentIndex(0);
		m_ui->comboBoxType->blockSignals(true);
		m_ui->comboBoxType->setCurrentIndex(0);
		m_ui->comboBoxType->blockSignals(false);
		m_ui->stackedWidget->setEnabled(false);

		m_ui->tableWidgetHeating->blockSignals(true);
		m_ui->tableWidgetHeating->setRowCount(0);
		m_ui->tableWidgetHeating->blockSignals(false);
		m_ui->tableWidgetCooling->blockSignals(true);
		m_ui->tableWidgetCooling->setRowCount(0);
		m_ui->tableWidgetCooling->blockSignals(false);

		return;
	}
	m_ui->stackedWidget->setEnabled(true);

	m_current = const_cast<VICUS::SurfaceHeating *>(m_db->m_surfaceHeatings[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);

	//set method
	m_ui->comboBoxType->blockSignals(true);
	m_ui->comboBoxType->setCurrentIndex(m_current->m_type);
	Q_ASSERT(m_ui->comboBoxType->currentIndex() != -1);
	m_ui->stackedWidget->setCurrentIndex(m_ui->comboBoxType->currentIndex());
	m_ui->comboBoxType->blockSignals(false);

	try {
		// test user data for value input
		m_current->m_para[VICUS::SurfaceHeating::P_HeatingLimit].checkedValue("HeatingLimit", "W/m2", "W/m2",
																			  0, true, 1000, false, nullptr);
	}
	catch (...) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", VICUS::SurfaceHeating::P_HeatingLimit, 50);
		modelModify();
	}
	m_ui->lineEditHeatingLimit->setValue(m_current->m_para[VICUS::SurfaceHeating::P_HeatingLimit].value); // input is in base SI unit

	try {
		m_current->m_para[VICUS::SurfaceHeating::P_CoolingLimit].checkedValue("CoolingLimit", "W/m2", "W/m2",
																			  0, true, 1000, false, nullptr);
	}  catch (...) {
		//set up a new value
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", VICUS::SurfaceHeating::P_CoolingLimit, 40);
		modelModify();
	}
	m_ui->lineEditCoolingLimit->setValue(m_current->m_para[VICUS::SurfaceHeating::P_CoolingLimit].value);


	try {
		m_current->m_para[VICUS::SurfaceHeating::P_MaxFluidVelocity].checkedValue("MaxFluidVelocity", "m/s", "m/s", 0, false, 10, true, nullptr);
	}  catch (...) {
		//set up a new value
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", VICUS::SurfaceHeating::P_MaxFluidVelocity, 10);
		modelModify();
	}
	m_ui->lineEditFluidVelocity->setValue(m_current->m_para[VICUS::SurfaceHeating::P_MaxFluidVelocity].value);

	try {
		m_current->m_para[VICUS::SurfaceHeating::P_TemperatureDifferenceSupplyReturn].checkedValue("TemperatureDifferenceSupplyReturn",
																								   "K", "K", 1, true, 80, true, nullptr);
	}  catch (...) {
		//set up a new value
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", VICUS::SurfaceHeating::P_TemperatureDifferenceSupplyReturn, 7);
		modelModify();
	}
	m_ui->lineEditTemperaturDifference->setValue(m_current->m_para[VICUS::SurfaceHeating::P_TemperatureDifferenceSupplyReturn].value);

	try {
		m_current->m_para[VICUS::SurfaceHeating::P_PipeSpacing].checkedValue("PipeSpacing", "m", "m", 0, false, 10, true, nullptr);
	}  catch (...) {
		//set up a new value
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", VICUS::SurfaceHeating::P_PipeSpacing, 0.1);
		modelModify();
	}
	//check if table data is valid
	std::string tSupply = "Tsupply";
	std::string tOut = "Tout";
	bool isTableValid = false;
	std::map<std::string, std::vector<double> >		&values = m_current->m_heatingCoolingCurvePoints.m_values;
	if(values.find(tSupply) != values.end() && values.find(tOut) != values.end()){
		if(values[tSupply].size() == 4 && values[tOut].size() == 4 )
			isTableValid = true;
	}
	//create new data for a valid table
	if(!isTableValid){
		values.clear();
		values[tSupply] = std::vector<double>{35,20,23,18};
		values[tOut] = std::vector<double>{-14,20,23,30};
		modelModify();
	}

	//fill data in heating table
	QTableWidget *tab = m_ui->tableWidgetHeating;
	tab->blockSignals(true);
	tab->setRowCount(2);
	for(int row = 0; row<2; ++row){
		tab->setItem(row,0, new QTableWidgetItem(QString("%1").arg(values[tOut][(size_t)row])));
		tab->setItem(row,1, new QTableWidgetItem(QString("%1").arg(values[tSupply][(size_t)row])));
		tab->item(row,0)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
		tab->item(row,1)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
		if(m_current->m_builtIn){
			tab->item(row,0)->setFlags(tab->item(row,0)->flags() ^ Qt::ItemIsEditable);
			tab->item(row,1)->setFlags(tab->item(row,1)->flags() ^ Qt::ItemIsEditable);
		}
	}
	//select first row
	tab->setCurrentCell(0,0);
	tab->blockSignals(false);

	//fill data in cooling table
	QTableWidget *tab2 = m_ui->tableWidgetCooling;
	tab2->blockSignals(true);
	tab2->setRowCount(2);
	for(int row = 0; row<2; ++row){
		tab2->setItem(row,0, new QTableWidgetItem(QString("%1").arg(values[tOut][(size_t)row+2])));
		tab2->setItem(row,1, new QTableWidgetItem(QString("%1").arg(values[tSupply][(size_t)row+2])));
		tab2->item(row,0)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
		tab2->item(row,1)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
		if(m_current->m_builtIn){
			tab2->item(row,0)->setFlags(tab2->item(row,0)->flags() ^ Qt::ItemIsEditable);
			tab2->item(row,1)->setFlags(tab2->item(row,1)->flags() ^ Qt::ItemIsEditable);
		}
	}
	//select first row
	tab2->setCurrentCell(0,0);
	tab2->blockSignals(false);


	// stoe values and plot
	Q_ASSERT(values[tOut].size()==4 && values[tSupply].size()==4);
	m_xDataHeating = {values[tOut][0]-5, values[tOut][0], values[tOut][1], values[tOut][1]+5};
	m_yDataHeating = {values[tSupply][0], values[tSupply][0], values[tSupply][1], values[tSupply][1]};
	m_xDataCooling = {values[tOut][2]-5, values[tOut][2], values[tOut][3], values[tOut][3]+5};
	m_yDataCooling = {values[tSupply][2], values[tSupply][2], values[tSupply][3], values[tSupply][3]};
	updatePlot();

	m_ui->lineEditPipeSpacing->setValue(m_current->m_para[VICUS::SurfaceHeating::P_PipeSpacing].value);

	// lookup corresponding dataset entry in database
	const VICUS::NetworkPipe * pipe = m_db->m_pipes[m_current->m_idPipe];
	if (pipe == nullptr)
		m_ui->lineEditPipeName->setText("");
	else
		m_ui->lineEditPipeName->setText( QtExt::MultiLangString2QString(pipe->m_displayName) );

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxType->setEnabled(!isbuiltIn);
	m_ui->lineEditHeatingLimit->setEnabled(!isbuiltIn);
	m_ui->lineEditCoolingLimit->setEnabled(!isbuiltIn);
	m_ui->lineEditTemperaturDifference->setEnabled(!isbuiltIn);
	m_ui->lineEditFluidVelocity->setEnabled(!isbuiltIn);
	m_ui->lineEditPipeSpacing->setEnabled(!isbuiltIn);
	m_ui->lineEditPipeName->setReadOnly(isbuiltIn);
	m_ui->toolButtonSelectPipes->setEnabled(!isbuiltIn);
	//tab->setEnabled(!isbuiltIn);
}


void SVDBSurfaceHeatingEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBSurfaceHeatingEditWidget::on_comboBoxType_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	if (static_cast<VICUS::SurfaceHeating::Type>(index) != m_current->m_type) {
		m_current->m_type = static_cast<VICUS::SurfaceHeating::Type>(index);
		modelModify();
	}
	m_ui->stackedWidget->setCurrentIndex(index);
}


void SVDBSurfaceHeatingEditWidget::modelModify() {
	m_db->m_surfaceHeatings.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}


void SVDBSurfaceHeatingEditWidget::updatePlot() {

	m_ui->widgetPlotHeatingCurve->detachItems( QwtPlotItem::Rtti_PlotCurve );
	m_ui->widgetPlotHeatingCurve->detachItems( QwtPlotItem::Rtti_PlotMarker );
	m_ui->widgetPlotHeatingCurve->replot();
	m_ui->widgetPlotHeatingCurve->setEnabled(false);
	if (m_current == nullptr)
		return;

	// now do all the plotting
	m_ui->widgetPlotHeatingCurve->setEnabled(true);

	// heating curve
	m_curveHeating = addConfiguredCurve(m_ui->widgetPlotHeatingCurve);
	configureCurveTheme(m_curveHeating);
	m_curveHeating->setRawSamples(m_xDataHeating.data(), m_yDataHeating.data(), (int)m_xDataHeating.size());
	m_curveHeating->setTitle("Heating Curve");
	m_curveHeating->setPen("#9a031e", 2);

	// cooling curve
	m_curveCooling = addConfiguredCurve(m_ui->widgetPlotHeatingCurve);
	configureCurveTheme(m_curveCooling);
	m_curveCooling->setRawSamples(m_xDataCooling.data(), m_yDataCooling.data(), (int)m_xDataCooling.size());
	m_curveCooling->setTitle("Cooling Curve");
	m_curveCooling->setPen("#3d5a80", 2);

	QFont ft;
	ft.setPointSize(10);
	QwtText xl(tr("Ambient Temperature [C]"));
	xl.setFont(ft);
	m_ui->widgetPlotHeatingCurve->setAxisTitle(QwtPlot::xBottom, xl);
	QwtText yl(tr("Supply Temperature [C]"));
	yl.setFont(ft);
	m_ui->widgetPlotHeatingCurve->setAxisTitle(QwtPlot::yLeft, yl);
	m_ui->widgetPlotHeatingCurve->replot();

	QwtLegend* legend2 = new QwtLegend;
	m_ui->widgetPlotHeatingCurve->insertLegend(legend2, QwtPlot::TopLegend);
}


void SVDBSurfaceHeatingEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBSurfaceHeatingEditWidget::on_lineEditPipeSpacing_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditPipeSpacing->value();
	VICUS::SurfaceHeating::para_t paraName= VICUS::SurfaceHeating::P_PipeSpacing;
	if (m_current->m_para[paraName].empty() || val != m_current->m_para[paraName].value) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", paraName, val);
		modelModify();
	}
}


void SVDBSurfaceHeatingEditWidget::on_lineEditFluidVelocity_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditFluidVelocity->value();

	VICUS::SurfaceHeating::para_t paraName= VICUS::SurfaceHeating::P_MaxFluidVelocity;
	if (m_current->m_para[paraName].empty() || val != m_current->m_para[paraName].value) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", paraName, val);
		modelModify();
	}
}


void SVDBSurfaceHeatingEditWidget::on_lineEditTemperaturDifference_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditTemperaturDifference->value();

	VICUS::SurfaceHeating::para_t paraName= VICUS::SurfaceHeating::P_TemperatureDifferenceSupplyReturn;
	if (m_current->m_para[paraName].empty() || val != m_current->m_para[paraName].value) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", paraName, val);
		modelModify();
	}
}

void SVDBSurfaceHeatingEditWidget::on_lineEditHeatingLimit_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditHeatingLimit->value();

	VICUS::SurfaceHeating::para_t paraName= VICUS::SurfaceHeating::P_HeatingLimit;
	if (m_current->m_para[paraName].empty() || val != m_current->m_para[paraName].value) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", paraName, val);
		modelModify();
	}
}

void SVDBSurfaceHeatingEditWidget::on_lineEditCoolingLimit_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	double val = m_ui->lineEditCoolingLimit->value();

	VICUS::SurfaceHeating::para_t paraName= VICUS::SurfaceHeating::P_CoolingLimit;
	if (m_current->m_para[paraName].empty() || val != m_current->m_para[paraName].value) {
		VICUS::KeywordList::setParameter(m_current->m_para, "SurfaceHeating::para_t", paraName, val);
		modelModify();
	}
}

void SVDBSurfaceHeatingEditWidget::on_toolButtonSelectPipes_clicked() {
	Q_ASSERT(m_current != nullptr);
	// get pipe edit dialog from mainwindow
	SVDatabaseEditDialog * pipeEditDialog = SVMainWindow::instance().dbPipeEditDialog();
	unsigned int pipeId = pipeEditDialog->select(m_current->m_idPipe);
	if (pipeId != VICUS::INVALID_ID && pipeId != m_current->m_idPipe){
		m_current->m_idPipe = pipeId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBSurfaceHeatingEditWidget::on_tableWidgetCooling_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous) {
	try {
		IBK::string2val<double>(current->text().toStdString().c_str());
		modelModify();
	}  catch (...) {
		previous->setText(current->text());
	}
}


void SVDBSurfaceHeatingEditWidget::on_tableWidgetCooling_itemChanged(QTableWidgetItem *item) {
	bool ok;
	double val = QtExt::Locale().toDoubleWithFallback(item->text(), &ok);

	if(ok){
		if(item->column() == 1) {
			// check if values differ
			std::vector<double> &vals = m_current->m_heatingCoolingCurvePoints.m_values["Tsupply"];
			double firstVal = item->row()==0 ? val : vals[2];
			double secondVal = item->row()==1 ? val : vals[3];
			if (IBK::near_equal(secondVal, firstVal, 0.01))
				QMessageBox::critical(this,QString(), tr("Supply temperature values should differ.").arg(item->text()));
			else {
				vals[(unsigned int)item->row() + 2] = val;
				modelModify();
			}
		}
		else { // column 0
			// we check if ambient temperature is increasing
			std::vector<double> &vals = m_current->m_heatingCoolingCurvePoints.m_values["Tout"];
			double firstVal = item->row()==0 ? val : vals[2];
			double secondVal = item->row()==1 ? val : vals[3];
			if (secondVal < firstVal)
				QMessageBox::critical(this,QString(), tr("Ambient temperature should be monotonic increasing.").arg(item->text()));
			else {
				vals[(unsigned int)item->row() + 2] = val;
				modelModify();
			}
		}
	}
	else{
		QMessageBox::critical(this,QString(), tr("Input '%1' is not valid. Only numerical data is valid.").arg(item->text()));
	}
	updateInput((int)m_current->m_id);
}


void SVDBSurfaceHeatingEditWidget::on_tableWidgetHeating_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous) {
	try {
		IBK::string2val<double>(current->text().toStdString().c_str());
		modelModify();
	}  catch (...) {
		previous->setText(current->text());
	}
}


void SVDBSurfaceHeatingEditWidget::on_tableWidgetHeating_itemChanged(QTableWidgetItem *item) {
	bool ok;
	double val = QtExt::Locale().toDoubleWithFallback(item->text(), &ok);

	if(ok){
		if(item->column() == 1) {
			// check if values differ
			std::vector<double> &vals = m_current->m_heatingCoolingCurvePoints.m_values["Tsupply"];
			double firstVal = item->row()==0 ? val : vals[0];
			double secondVal = item->row()==1 ? val : vals[1];
			if (IBK::near_equal(secondVal, firstVal, 0.01))
				QMessageBox::critical(this,QString(), tr("Supply temperature values should differ.").arg(item->text()));
			else {
				vals[(unsigned int)item->row()] = val;
				modelModify();
			}
		}
		else { // column 0
			// we check if ambient temperature is increasing
			std::vector<double> &vals = m_current->m_heatingCoolingCurvePoints.m_values["Tout"];
			double firstVal = item->row()==0 ? val : vals[0];
			double secondVal = item->row()==1 ? val : vals[1];
			if (secondVal < firstVal)
				QMessageBox::critical(this,QString(), tr("Ambient temperature should be monotonic increasing.").arg(item->text()));
			else {
				vals[(unsigned int)item->row()] = val;
				modelModify();
			}
		}
	}
	else{
		QMessageBox::critical(this,QString(), tr("Input '%1' is not valid. Only numerical data is valid.").arg(item->text()));
	}
	updateInput((int)m_current->m_id);

}

