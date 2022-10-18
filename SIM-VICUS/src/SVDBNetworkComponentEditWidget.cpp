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

#include "SVDBNetworkComponentEditWidget.h"
#include "ui_SVDBNetworkComponentEditWidget.h"

#include "SVSettings.h"
#include "SVDBNetworkComponentTableModel.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVConversions.h"
#include "SVChartUtils.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>

#include <VICUS_NetworkComponent.h>
#include <VICUS_KeywordListQt.h>
#include <VICUS_utilities.h>

#include <NANDRAD_HydraulicNetworkComponent.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Locale.h>



SVDBNetworkComponentEditWidget::SVDBNetworkComponentEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBNetworkComponentEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(),THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Component identification name"));

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	// enter categories into combo box
	// block signals to avoid getting "changed" calls
	m_ui->comboBoxComponentType->blockSignals(true);
	for (int i=0; i<VICUS::NetworkComponent::NUM_MT; ++i)
		m_ui->comboBoxComponentType->addItem(QString("%1 [%2]").arg(VICUS::KeywordListQt::Description("NetworkComponent::ModelType", i))
											 .arg(VICUS::KeywordListQt::Keyword("NetworkComponent::ModelType", i)), i);
	m_ui->comboBoxComponentType->blockSignals(false);

	// no headers
	m_ui->tableWidgetParameters->horizontalHeader()->setVisible(false);
	m_ui->tableWidgetParameters->verticalHeader()->setVisible(false);
	m_ui->tableWidgetParameters->setColumnCount(3);
	m_ui->tableWidgetParameters->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableWidgetParameters->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_ui->tableWidgetParameters->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetParameters);
	m_ui->tableWidgetParameters->setSortingEnabled(false);

	m_ui->tableWidgetPolynomCoefficients->horizontalHeader()->setVisible(false);
	m_ui->tableWidgetPolynomCoefficients->verticalHeader()->setVisible(true);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetPolynomCoefficients);
	m_ui->tableWidgetPolynomCoefficients->setSortingEnabled(false);

	// create chart and curve
	configureChart(m_ui->widgetPlot1);
	configureChart(m_ui->widgetPlot2);

	updateInput(-1);
}


SVDBNetworkComponentEditWidget::~SVDBNetworkComponentEditWidget() {
	delete m_ui;
}


void SVDBNetworkComponentEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBNetworkComponentTableModel*>(dbModel);
}


void SVDBNetworkComponentEditWidget::updateInput(int id) {
	FUNCID(SVDBNetworkComponentEditWidget::updateInput);

	m_current = nullptr; // disable edit triggers

	// clear input controls
	m_ui->lineEditName->setString(IBK::MultiLanguageString());
	m_ui->lineEditSchedule1->clear();
	m_ui->lineEditSchedule1->setEnabled(false);
	m_ui->lineEditSchedule2->clear();
	m_ui->lineEditSchedule2->setEnabled(false);

	if (id == -1) {
		// disable all controls - note: this does not disable signals of the components below
		setEnabled(false);

		// construction property info fields
		m_ui->comboBoxComponentType->blockSignals(true);
		m_ui->comboBoxComponentType->setCurrentIndex(-1);
		m_ui->comboBoxComponentType->blockSignals(false);

		// Note: color button is disabled, hence color is gray
		return;
	}
	// re-enable all controls
	setEnabled(true);

	VICUS::NetworkComponent * comp = const_cast<VICUS::NetworkComponent*>(m_db->m_networkComponents[(unsigned int)id]);
	m_current = comp;

	// now update the GUI controls
	m_ui->lineEditName->setString(comp->m_displayName);

	m_ui->comboBoxComponentType->blockSignals(true);
	int typeIdx = m_ui->comboBoxComponentType->findData(comp->m_modelType);
	m_ui->comboBoxComponentType->setCurrentIndex(typeIdx);
	m_ui->comboBoxComponentType->blockSignals(false);

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType =
			VICUS::NetworkComponent::nandradNetworkComponentModelType(m_current->m_modelType);

	// enable schedules tool buttons (based on required schedules)
	std::vector<std::string> reqScheduleNames = NANDRAD::HydraulicNetworkComponent::requiredScheduleNames(nandradModelType);
	m_ui->groupBoxSchedules->setEnabled(!reqScheduleNames.empty());
	m_ui->toolButtonSchedule1->setEnabled(reqScheduleNames.size()==1 || reqScheduleNames.size()==2);
	m_ui->toolButtonSchedule2->setEnabled(reqScheduleNames.size()==2);

	// update schedule labels
	m_ui->labelSchedule1->clear();
	m_ui->labelSchedule2->clear();
	if (reqScheduleNames.size()>0)
		m_ui->labelSchedule1->setText(VICUS::camelCase2ReadableString(reqScheduleNames[0]));
	if (reqScheduleNames.size()>1)
		m_ui->labelSchedule2->setText(VICUS::camelCase2ReadableString(reqScheduleNames[1]));

	// update Schedule names (based on existing schedules)
	if (m_current->m_scheduleIds.size()>0){
		m_ui->lineEditSchedule1->setEnabled(true);
		if (m_db->m_schedules[m_current->m_scheduleIds[0]] == nullptr)
			m_ui->lineEditSchedule1->setText(tr("Invalid Schedule"));
		else
			m_ui->lineEditSchedule1->setText(QtExt::MultiLangString2QString(
											 m_db->m_schedules[m_current->m_scheduleIds[0]]->m_displayName));
	}
	if (m_current->m_scheduleIds.size()>1){
		m_ui->lineEditSchedule2->setEnabled(true);
		if (m_db->m_schedules[m_current->m_scheduleIds[1]] == nullptr)
			m_ui->lineEditSchedule2->setText(tr("Invalid Schedule"));
		else
			m_ui->lineEditSchedule2->setText(QtExt::MultiLangString2QString(
											 m_db->m_schedules[m_current->m_scheduleIds[1]]->m_displayName));
	}

	// update pipe properties
	m_ui->lineEditPipeProperties->clear();
	m_ui->groupBoxPipeProperties->setEnabled(false);
	if (VICUS::NetworkComponent::hasPipeProperties(m_current->m_modelType)){
		m_ui->groupBoxPipeProperties->setEnabled(true);
		const VICUS::NetworkPipe *pipe = m_db->m_pipes[m_current->m_pipePropertiesId];
		if(pipe != nullptr)
			m_ui->lineEditPipeProperties->setText(QtExt::MultiLangString2QString(pipe->m_displayName));
	}

	bool isEditable = !comp->m_builtIn;

	// update table widgets and plot
	updateParameterTableWidget(!isEditable);
	updatePolynomCoeffTableWidget(!isEditable);
	updatePolynomPlot();

	// for built-ins, disable editing/make read-only
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
	m_ui->comboBoxComponentType->setEnabled(isEditable);
	if (!isEditable) {
		m_ui->toolButtonSchedule1->setEnabled(false);
		m_ui->toolButtonSchedule2->setEnabled(false);
	}
}



void SVDBNetworkComponentEditWidget::updateParameterTableWidget(bool readOnly) const{
	FUNCID(SVDBNetworkComponentEditWidget::updateParameterTableWidget);

	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType =
			VICUS::NetworkComponent::nandradNetworkComponentModelType(m_current->m_modelType);

	// populate table widget with properties
	m_ui->tableWidgetParameters->clearContents();

	// get required parameters of the current component
	std::vector<unsigned int> paraVecStd = NANDRAD::HydraulicNetworkComponent::requiredParameter(nandradModelType, 1);
	std::vector<unsigned int> paraVecAdd = m_current->additionalRequiredParameter(m_current->m_modelType);
	std::vector<unsigned int> paraVec;
	for (unsigned int i: paraVecStd)
		paraVec.push_back(i);
	for (unsigned int i: paraVecAdd)
		paraVec.push_back(i);

	// get integer parameters
	std::vector<unsigned int> paraVecInt = m_current->requiredIntParameter(m_current->m_modelType);

	// get optional parameters
	std::vector<unsigned int> paraVecOpt = m_current->optionalParameter(m_current->m_modelType);

	// populate table widget with parameters
	m_ui->tableWidgetParameters->setRowCount(paraVec.size() + paraVecInt.size() + paraVecOpt.size());
	if (paraVec.empty() && paraVecInt.empty() && paraVecOpt.empty())
		m_ui->groupBoxModelParameters->setEnabled(false);
	else
		m_ui->groupBoxModelParameters->setEnabled(true);

	m_ui->tableWidgetParameters->blockSignals(true);

	int rowCount = 0;
	for (unsigned int para: paraVec) {
		QTableWidgetItem * item = new QTableWidgetItem(VICUS::KeywordListQt::Keyword("NetworkComponent::para_t", (int)para));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetParameters->setItem(rowCount, 0, item);
		try {
			IBK::Unit ioUnit(VICUS::KeywordListQt::Unit("NetworkComponent::para_t", (int)para));
			item = new QTableWidgetItem(QString::fromStdString(ioUnit.name()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetParameters->setItem(rowCount, 2, item);

			if (m_current->m_para[para].name.empty())
				item = new QTableWidgetItem(); // TODO : Hauke, set some meaningful initial value?
			else
				item = new QTableWidgetItem(QString("%L1").arg(m_current->m_para[para].get_value(ioUnit)));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			if ((unsigned int)rowCount < paraVecStd.size())
				item->setData(Qt::UserRole, DT_DoubleStd);
			else
				item->setData(Qt::UserRole, DT_DoubleAdditional);
			if (readOnly)
				item->setFlags(item->flags() & ~Qt::ItemIsEditable);
			item->setData(Qt::UserRole+1, para);
			m_ui->tableWidgetParameters->setItem(rowCount, 1, item);

			++rowCount;
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}
	}


	// populate table widget with integer parameters

	for (unsigned int paraInt: paraVecInt) {
		// parameter name
		QTableWidgetItem * item = new QTableWidgetItem(VICUS::KeywordListQt::Keyword("NetworkComponent::intPara_t", (int)paraInt));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetParameters->setItem(rowCount, 0, item);
		try {
			// parameter unit
			item = new QTableWidgetItem("-");
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetParameters->setItem(rowCount, 2, item);
			// parameter value
			if (m_current->m_intPara[paraInt].name.empty())
				item = new QTableWidgetItem(); // TODO : Hauke, set some meaningful initial value?
			else
				item = new QTableWidgetItem(QString("%L1").arg(m_current->m_intPara[paraInt].value));
			if (readOnly)
				item->setFlags(item->flags() & ~Qt::ItemIsEditable);
			item->setData(Qt::UserRole, DT_Integer);
			item->setData(Qt::UserRole+1, paraInt);
			m_ui->tableWidgetParameters->setItem(rowCount, 1, item);

			++rowCount;
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}
	}


	// populate table widget with optional parameters

	QFont fnt;
	fnt.setItalic(true);
	for (unsigned int paraOpt: paraVecOpt) {
		// parameter name
		QTableWidgetItem * item = new QTableWidgetItem(VICUS::KeywordListQt::Keyword("NetworkComponent::para_t", (int)paraOpt));
		item->setFont(fnt);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetParameters->setItem(rowCount, 0, item);
		try {
			// parameter unit
			IBK::Unit ioUnit(VICUS::KeywordListQt::Unit("NetworkComponent::para_t", (int)paraOpt));
			item = new QTableWidgetItem(QString::fromStdString(ioUnit.name()));
			item->setFont(fnt);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetParameters->setItem(rowCount, 2, item);
			// parameter value
			if (m_current->m_para[paraOpt].name.empty())
				item = new QTableWidgetItem(); // TODO : Hauke, set some meaningful initial value?
			else
				item = new QTableWidgetItem(QString("%L1").arg(m_current->m_para[paraOpt].get_value(ioUnit)));
			item->setFont(fnt);
			if (readOnly)
				item->setFlags(item->flags() & ~Qt::ItemIsEditable);
			item->setData(Qt::UserRole, DT_DoubleOptional);
			item->setData(Qt::UserRole+1, paraOpt);
			m_ui->tableWidgetParameters->setItem(rowCount, 1, item);

			++rowCount;
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}
	}

	m_ui->tableWidgetParameters->blockSignals(false);
	m_ui->tableWidgetParameters->resizeColumnsToContents();
}


void SVDBNetworkComponentEditWidget::updatePolynomCoeffTableWidget(bool readOnly) const {

	m_ui->groupBoxPolynom->setEnabled(false);
	m_ui->tableWidgetPolynomCoefficients->blockSignals(true);
	m_ui->tableWidgetPolynomCoefficients->clearContents();
	m_ui->tableWidgetPolynomCoefficients->setRowCount(0);
	m_ui->tableWidgetPolynomCoefficients->setColumnCount(0);

	unsigned int rowCount=0;
	unsigned int columnCount=0;
	bool isOptional = false;
	std::vector<std::string> header;
	switch (m_current->m_modelType ) {
		case VICUS::NetworkComponent::MT_HeatPumpOnOffSourceSide: {
			header = {"QdotCondensator", "ElectricalPower"};
			columnCount = 6;
			isOptional = false;
		} break;
		case VICUS::NetworkComponent::MT_HeatPumpVariableSourceSide: {
			header = {"COP"};
			columnCount = 6;
			isOptional = false;
		} break;
		case VICUS::NetworkComponent::MT_ConstantPressurePump:
		case VICUS::NetworkComponent::MT_ControlledPump:
		case VICUS::NetworkComponent::MT_VariablePressurePump: {
			header = {"MaximumElectricalPower", "MaximumPressureHead"};
			columnCount = 3;
			isOptional = true;
		} break;
		default: {
			m_ui->tableWidgetPolynomCoefficients->blockSignals(false);
			return;
		}
	}

	// set header, row and column count
	m_ui->groupBoxPolynom->setEnabled(true);
	m_ui->tableWidgetPolynomCoefficients->verticalHeader()->setVisible(true);
	rowCount = header.size();
	m_ui->tableWidgetPolynomCoefficients->setRowCount((int)rowCount);
	m_ui->tableWidgetPolynomCoefficients->setColumnCount((int)columnCount);

	// better to read reference
	const std::map<std::string, std::vector<double> > &values = m_current->m_polynomCoefficients.m_values;

	QFont font;
	font.setItalic(isOptional);

	// in case the value map is empty we write zeros into the table, otherwise we use the according values
	for (unsigned int row=0; row<rowCount; ++row) {
		// set header
		QTableWidgetItem *headItem = new QTableWidgetItem( QString::fromStdString(header[row]));
		headItem->setFont(font);
		m_ui->tableWidgetPolynomCoefficients->setVerticalHeaderItem((int)row, headItem);
		// populate values
		for (unsigned int col=0; col<columnCount; ++col) {
			double val = 0; // default value
			// if we have a value in the map, use that
			if (values.find(header[row]) != values.end() && col < values.at(header[row]).size() )
				val = values.at(header[row])[col];
			QTableWidgetItem *item = new QTableWidgetItem(QString("%1").arg(val));
			item->setFont(font);
			if (readOnly)
				item->setFlags(item->flags() & ~Qt::ItemIsEditable);
			m_ui->tableWidgetPolynomCoefficients->setItem((int)row, (int)col, item);
		}
	}
	m_ui->tableWidgetPolynomCoefficients->resizeRowsToContents();
	m_ui->tableWidgetPolynomCoefficients->blockSignals(false);
}


void SVDBNetworkComponentEditWidget::updatePolynomPlot() {

	m_ui->widgetPlot1->detachItems( QwtPlotItem::Rtti_PlotCurve );
	m_ui->widgetPlot1->detachItems( QwtPlotItem::Rtti_PlotMarker );
	m_ui->widgetPlot1->setAxisTitle(QwtPlot::xBottom, "");
	m_ui->widgetPlot1->setAxisTitle(QwtPlot::yLeft, "");
	m_ui->widgetPlot1->replot();
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
	m_ui->tabWidgetPlots->setTabVisible(0, false);
#else
	m_ui->plotTab1->setEnabled(false);
	m_ui->widgetPlot1->setVisible(false);
#endif

	m_ui->widgetPlot2->detachItems( QwtPlotItem::Rtti_PlotCurve );
	m_ui->widgetPlot2->detachItems( QwtPlotItem::Rtti_PlotMarker );
	m_ui->widgetPlot2->setAxisTitle(QwtPlot::xBottom, "");
	m_ui->widgetPlot2->setAxisTitle(QwtPlot::yLeft, "");
	m_ui->widgetPlot2->replot();
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
	m_ui->tabWidgetPlots->setTabVisible(1, false);
#else
	m_ui->plotTab2->setEnabled(false);
	m_ui->widgetPlot2->setVisible(false);
#endif

	QString xLabel;
	QString yLabel1, yLabel2;
	std::vector<QString> curveLabels1, curveLabels2;
	switch (m_current->m_modelType ) {
		case VICUS::NetworkComponent::MT_HeatPumpVariableSourceSide: {

			// if invalid polynom, do nothing
			if (m_current->m_polynomCoefficients.m_values.find("COP") == m_current->m_polynomCoefficients.m_values.end())
				return;
			const std::vector<double> &coeffs = m_current->m_polynomCoefficients.m_values.at("COP");
			if (coeffs.size() != 6)
				return;

			// create x data (evaporator temperature)
			m_xData.clear();
			double T=-5;
			while (T<15){
				m_xData.push_back(T);
				T += 0.1;
			}

			// create y data (COP)
			m_yData1.clear();
			std::vector<double> TcVec = {35, 45, 55};
			for (double &Tc: TcVec) {
				m_yData1.push_back(std::vector<double>());
				curveLabels1.push_back(QString("%1 °C").arg(Tc));
				for (double &Te: m_xData)
					m_yData1.back().push_back(coeffs[0] + coeffs[1] * (Te+273.15) + coeffs[2] * (Tc+273.15) + coeffs[3] * (Te+273.15) * (Tc+273.15) +
									coeffs[4] * (Te+273.15) * (Te+273.15) + coeffs[5] * (Tc+273.15) * (Tc+273.15));
			}
			xLabel = "Evaporator Temperature [C]";
			yLabel1 = "COP [-]";
			m_ui->tabWidgetPlots->setTabText(0, "COP");
			m_yData2.clear();

		} break;

		case VICUS::NetworkComponent::MT_ConstantPressurePump:
		case VICUS::NetworkComponent::MT_ControlledPump:
		case VICUS::NetworkComponent::MT_VariablePressurePump: {

			// check if we have a polynom
			bool havePolynom = false;
			if (m_current->m_polynomCoefficients.m_values.find("MaximumPressureHead") != m_current->m_polynomCoefficients.m_values.end())
				havePolynom = true;
			// if so check size
			std::vector<double> coeffs1;
			std::vector<double> coeffs2;
			if (havePolynom) {
				coeffs1 = m_current->m_polynomCoefficients.m_values.at("MaximumPressureHead");
				coeffs2 = m_current->m_polynomCoefficients.m_values.at("MaximumElectricalPower");
				if (coeffs1.size() != 3)
					return;
			}

			// parameter readability
			double &maxDpPara = m_current->m_para[VICUS::NetworkComponent::P_MaximumPressureHead].value;
			double &maxPelPara = m_current->m_para[VICUS::NetworkComponent::P_PumpMaximumElectricalPower].value;
			double &maxEtaPara = m_current->m_para[VICUS::NetworkComponent::P_PumpMaximumEfficiency].value;

			// find maximum Vdot
			double VdotMax=0; // in m3/s
			double dpMax;
			while (VdotMax<1) {
				VdotMax += 1e-3;
				if (havePolynom) // polynom model
					dpMax = coeffs1[0] * VdotMax * VdotMax + coeffs1[1] * VdotMax + coeffs1[2];
				else // simple model
					dpMax = maxDpPara * (1 - maxDpPara / (4 * maxPelPara * maxEtaPara) * VdotMax);
				if (dpMax<=0)
					break;
			}

			// create x data
			m_xData.clear();
			double Vdot=0;
			while (Vdot<VdotMax){
				m_xData.push_back(Vdot*3600);
				Vdot += VdotMax/100;
			}
			// create y data1
			m_yData1.clear();
			m_yData1.push_back(std::vector<double>());
			double dp;
			for (double &Vdot: m_xData) {
				if (havePolynom)
					dp = coeffs1[0] * (Vdot/3600) * (Vdot/3600) + coeffs1[1] * (Vdot/3600) + coeffs1[2];
				else
					dp = maxDpPara * (1 - maxDpPara / (4 * maxPelPara * maxEtaPara) * Vdot/3600);
				m_yData1.back().push_back(dp/1e5);
			}
			xLabel = "Volume flow rate [m³/h]";
			yLabel1 = "Max. Pressure Head [bar]";
			curveLabels1.push_back(QString("max. dp"));
			m_ui->tabWidgetPlots->setTabText(0, "Max. Pressure Head");

			// create y data2
			m_yData2.clear();
			m_yData2.push_back(std::vector<double>());
			for (double &Vdot: m_xData) {
				double Pel;
				if (havePolynom)
					Pel = coeffs2[0] * (Vdot/3600) * (Vdot/3600) + coeffs2[1] * (Vdot/3600) + coeffs2[2];
				else
					Pel = maxPelPara;
				m_yData2.back().push_back(Pel);
			}
			xLabel = "Volume flow rate [m³/h]";
			yLabel2 = "Max. Electrical Power [W]";
			curveLabels2.push_back(QString("Pel"));
			m_ui->tabWidgetPlots->setTabText(1, "Max. El. Power");
		} break;

		default:
			return;
	}

	std::vector<QColor> colors = {"#db2b39", "#29335c", "#f3a712"};
	// now do all the plotting
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
	m_ui->tabWidgetPlots->setTabVisible(0, true);
#else
	m_ui->plotTab1->setEnabled(true);
	m_ui->widgetPlot1->setVisible(true);
#endif
	m_curves1.clear();
	Q_ASSERT(curveLabels1.size()==m_yData1.size());
	for (unsigned int i=0; i<m_yData1.size(); ++i) {
		m_curves1.push_back(addConfiguredCurve(m_ui->widgetPlot1));
		// adjust styling based on current theme's settings
		configureCurveTheme(m_curves1.back());
		m_curves1.back()->setPen(colors[i]);
		// set data
		m_curves1.back()->setRawSamples(m_xData.data(), m_yData1[i].data(), (int)m_xData.size());
		m_curves1.back()->setTitle(curveLabels1[i]);
	}
	QFont ft;
	ft.setPointSize(10);
	QwtText xl(xLabel);
	xl.setFont(ft);
	m_ui->widgetPlot1->setAxisTitle(QwtPlot::xBottom, xl);
	QwtText yl(yLabel1);
	yl.setFont(ft);
	m_ui->widgetPlot1->setAxisTitle(QwtPlot::yLeft, yl);
	if (curveLabels1.size()>1) {
		QwtLegend* legend = new QwtLegend;
		m_ui->widgetPlot1->insertLegend(legend, QwtPlot::RightLegend);
	}
	m_ui->widgetPlot1->replot();


	if (!m_yData2.empty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
		m_ui->tabWidgetPlots->setTabVisible(1, true);
#else
		m_ui->plotTab2->setEnabled(false);
		m_ui->widgetPlot2->setVisible(true);
#endif
		m_curves2.clear();
		Q_ASSERT(curveLabels2.size()==m_yData2.size());
		for (unsigned int i=0; i<m_yData2.size(); ++i) {
			m_curves2.push_back(addConfiguredCurve(m_ui->widgetPlot2));
			// adjust styling based on current theme's settings
			configureCurveTheme(m_curves2.back());
			m_curves2.back()->setPen(colors[i]);
			// set data
			m_curves2.back()->setRawSamples(m_xData.data(), m_yData2[i].data(), (int)m_xData.size());
			m_curves2.back()->setTitle(curveLabels2[i]);
		}
		QFont ft;
		ft.setPointSize(10);
		QwtText xl(xLabel);
		xl.setFont(ft);
		m_ui->widgetPlot2->setAxisTitle(QwtPlot::xBottom, xl);
		QwtText yl2(yLabel2);
		yl2.setFont(ft);
		m_ui->widgetPlot2->setAxisTitle(QwtPlot::yLeft, yl2);
		if (curveLabels2.size()>1) {
			QwtLegend* legend2 = new QwtLegend;
			m_ui->widgetPlot2->insertLegend(legend2, QwtPlot::RightLegend);
		}
		m_ui->widgetPlot2->replot();
	}


}


void SVDBNetworkComponentEditWidget::on_lineEditName_editingFinished(){
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBNetworkComponentEditWidget::on_comboBoxComponentType_currentIndexChanged(int /*index*/) {
	if (m_current == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty"

	VICUS::NetworkComponent::ModelType ct = VICUS::NetworkComponent::ModelType(
													m_ui->comboBoxComponentType->currentData().toUInt());

	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType =
			VICUS::NetworkComponent::nandradNetworkComponentModelType(m_current->m_modelType);
	std::vector<unsigned int> paraVecStd = NANDRAD::HydraulicNetworkComponent::requiredParameter(nandradModelType, 1);
	std::vector<unsigned int> paraVecAdd = m_current->additionalRequiredParameter(m_current->m_modelType);
	std::vector<unsigned int> paraVecOpt = m_current->optionalParameter(m_current->m_modelType);
	std::vector<unsigned int> paraVecInt = m_current->requiredIntParameter(m_current->m_modelType);

	if (ct != m_current->m_modelType) {
		// set new model type
		m_current->m_modelType = ct;
		// we keep those parameters tat are still valid
		for (unsigned int i=0; i<VICUS::NetworkComponent::NUM_P; ++i){
			if (std::find(paraVecStd.begin(), paraVecStd.end(), i) != paraVecStd.end() ||
				std::find(paraVecAdd.begin(), paraVecAdd.end(), i) != paraVecAdd.end() ||
				std::find(paraVecOpt.begin(), paraVecOpt.end(), i) != paraVecOpt.end())
				continue;
			m_current->m_para[i].clear();
		}
		for (unsigned int i=0; i<VICUS::NetworkComponent::NUM_IP; ++i){
			if (std::find(paraVecInt.begin(), paraVecInt.end(), i) == paraVecInt.end())
				m_current->m_intPara[i].clear();
		}
		// clear all other properties
		m_current->m_notes.clear();
		m_current->m_dataSource.clear();
		m_current->m_scheduleIds.clear();
		m_current->m_manufacturer.clear();
		m_current->m_polynomCoefficients.m_values.clear();
		m_current->m_pipePropertiesId = VICUS::INVALID_ID;

		modelModify();
		updateInput((int)m_current->m_id);
	}
}


void SVDBNetworkComponentEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}


void SVDBNetworkComponentEditWidget::on_toolButtonSchedule1_clicked()
{
	Q_ASSERT(m_current != nullptr);

	// open schedule edit dialog in selection mode
	unsigned int id = 0;
	if (m_current->m_scheduleIds.size()>0)
		id = m_current->m_scheduleIds[0];

	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(id);

	// if dialog was canceled do nothing
	if (newId == VICUS::INVALID_ID)
		return;

	// else if we have a new id set it
	if (id != newId) {
		if (m_current->m_scheduleIds.size()>0)
			m_current->m_scheduleIds[0] = newId;
		else
			m_current->m_scheduleIds.push_back(newId);
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBNetworkComponentEditWidget::on_toolButtonSchedule2_clicked()
{
	Q_ASSERT(m_current != nullptr);

	// open schedule edit dialog in selection mode
	unsigned int id = 0;
	if (m_current->m_scheduleIds.size()==2)
		id = m_current->m_scheduleIds[1];

	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(id);

	// if dialog was canceled do nothing
	if (newId == VICUS::INVALID_ID)
		return;

	if (id != newId) {
		if (m_current->m_scheduleIds.size()==2)
			m_current->m_scheduleIds[1] = newId;
		else
			m_current->m_scheduleIds.push_back(newId);
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBNetworkComponentEditWidget::on_tableWidgetParameters_cellChanged(int row, int column) {

	Q_ASSERT(column==1); // only values can be edited

	QString errMsg("");

	// get pointer to item and its content
	QTableWidgetItem * item = m_ui->tableWidgetParameters->item(row, 1);
	QString text = item->text();

	bool ok = false;

	// Double parameters

	if (item->data(Qt::UserRole) == DT_DoubleStd ||
		item->data(Qt::UserRole) == DT_DoubleAdditional ||
		item->data(Qt::UserRole) == DT_DoubleOptional ) {

		VICUS::NetworkComponent::para_t paraNum = VICUS::NetworkComponent::para_t(
												item->data(Qt::UserRole + 1).toUInt());
		// empty parameters are allowed
		if (text.isEmpty()){
			m_current->m_para[paraNum] = IBK::Parameter();
			modelModify();
			return;
		}

		// check number
		double val = QtExt::Locale().toDouble(text, &ok);
		if (!ok)
			val = text.toDouble(&ok);
		if (!ok)
			errMsg = "Only numbers allowed!";

		// now do parameter specific checks
		if (ok) {
			IBK::Parameter parameter(VICUS::KeywordListQt::Keyword("NetworkComponent::para_t", paraNum), val,
									 VICUS::KeywordListQt::Unit("NetworkComponent::para_t", paraNum));
			try {
				if (item->data(Qt::UserRole) == DT_DoubleStd ||
					item->data(Qt::UserRole) == DT_DoubleOptional)
					NANDRAD::HydraulicNetworkComponent::checkModelParameter(parameter, paraNum);
				else
					VICUS::NetworkComponent::checkAdditionalParameter(parameter, paraNum);
			} catch (IBK::Exception &ex) {
				errMsg = ex.what();
				ok = false;
			}
			// finally set value
			VICUS::KeywordList::setParameter(m_current->m_para, "NetworkComponent::para_t", paraNum, val);
			modelModify();
		}
		else {
			m_ui->tableWidgetParameters->blockSignals(true);
			if (m_current->m_para[paraNum].empty())
				item->setText("");
			else
				item->setText(QString("%1").arg(m_current->m_para[paraNum].value));
			m_ui->tableWidgetParameters->blockSignals(false);
		}

	}


	// Integer parameters

	else if (item->data(Qt::UserRole) == DT_Integer)  {

		VICUS::NetworkComponent::intPara_t paraNum = VICUS::NetworkComponent::intPara_t(
												item->data(Qt::UserRole + 1).toUInt());
		// empty parameters are allowed
		if (text.isEmpty()){
			m_current->m_intPara[paraNum] = IBK::IntPara();
			modelModify();
			return;
		}

		// check if is integer and if it is double then cast it to integer
		int val = QtExt::Locale().toInt(text, &ok);
		if (!ok){
			val = (int)QtExt::Locale().toDouble(text, &ok);
		}
		if (!ok)
			errMsg = "Only numbers allowed!";

		// now do parameter specific checks
		std::string paraName = VICUS::KeywordListQt::Keyword("NetworkComponent::intPara_t", paraNum);
		if (ok) {
			IBK::IntPara parameter(paraName, val);
			try {
				VICUS::NetworkComponent::checkIntParameter(parameter, paraNum);
			} catch (IBK::Exception &ex) {
				errMsg = ex.what();
				ok = false;
			}
			// finally set value
			m_current->m_intPara[paraNum] = IBK::IntPara(paraName, val);
			modelModify();
		}
		else {
			m_ui->tableWidgetParameters->blockSignals(true);
			if (m_current->m_para[paraNum].empty())
				item->setText("");
			else
				item->setText(QString("%1").arg(m_current->m_intPara[paraNum].value));
			m_ui->tableWidgetParameters->blockSignals(false);
		}
	}

	if (!ok){
		QMessageBox msgBox(QMessageBox::Critical, "Invalid Value", errMsg, QMessageBox::Ok, this);
		msgBox.exec();
	}
}



void SVDBNetworkComponentEditWidget::modelModify() {
	m_db->m_networkComponents.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id);
	updateParameterTableWidget(m_current->m_builtIn);
	updatePolynomPlot();
}


void SVDBNetworkComponentEditWidget::on_toolButtonPipeProperties_clicked()
{
	Q_ASSERT(m_current != nullptr);

	// open schedule edit dialog in selection mode
	unsigned int id = 0;
	if (m_current->m_pipePropertiesId != VICUS::INVALID_ID)
		id = m_current->m_pipePropertiesId;

	unsigned int newId = SVMainWindow::instance().dbPipeEditDialog()->select(id);

	// if dialog was canceled do nothing
	if (newId == VICUS::INVALID_ID)
		return;

	// else if we have a new id set it
	if (id != newId) {
		m_current->m_pipePropertiesId = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBNetworkComponentEditWidget::on_tableWidgetPolynomCoefficients_cellChanged(int row, int /*column*/) {

	std::string header = m_ui->tableWidgetPolynomCoefficients->verticalHeaderItem(row)->text().toStdString();

	m_current->m_polynomCoefficients.m_values[header].clear();
	bool allZero = true;
	for (int col=0; col<m_ui->tableWidgetPolynomCoefficients->columnCount(); ++col) {

		// check number
		QString text = m_ui->tableWidgetPolynomCoefficients->item(row, col)->text();
		bool ok = false;
		double val=0;
		if (!text.isEmpty()) {
			val = QtExt::Locale().toDouble(text, &ok);
			if (!ok)
				val = text.toDouble(&ok);
		}
		// set value back
		if (ok)
			m_current->m_polynomCoefficients.m_values[header].push_back(val);
		else
			m_current->m_polynomCoefficients.m_values[header].push_back(0);

		if (m_current->m_polynomCoefficients.m_values[header][(unsigned int)col]>0 ||
			m_current->m_polynomCoefficients.m_values[header][(unsigned int)col]<0)
			allZero = false;
	}
	// if all values are zero we remove this entry (so if it is optional it would still be considered valid)
	if (allZero)
		m_current->m_polynomCoefficients.m_values.erase(header);

	updateInput((int)m_current->m_id);
}

