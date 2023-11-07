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

#include "SVSimulationLocationOptions.h"
#include "ui_SVSimulationLocationOptions.h"

#include <QFileInfo>
#include <QMainWindow>
#include <QProgressDialog>
#include <QProcess>

#include <NANDRAD_Location.h>
#include <NANDRAD_KeywordList.h>

#include "SVSettings.h"
#include "SVClimateDataTableModel.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVClimateDataSortFilterProxyModel.h"
#include "SVProjectHandler.h"
#include "SVUndoModifyClimate.h"

#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_date_scale_draw.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_grid.h>
#include <qwt_date_scale_engine.h>
#include <qwt_scale_div.h>
#include "qwt_scale_widget.h"
#include "qwt_plot_zoomer.h"


SVSimulationLocationOptions::SVSimulationLocationOptions(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationLocationOptions)
{
	m_ui->setupUi(this);
	layout()->setContentsMargins(0,0,0,0);

	// source model
	m_climateDataModel = SVSettings::instance().climateDataTableModel();

	// proxy model
	m_filterModel = new SVClimateDataSortFilterProxyModel(this);
	m_filterModel->setSourceModel(m_climateDataModel);

	// set proxy model into table
	m_ui->tableViewClimateFiles->setModel(m_filterModel);

	SVDBModelDelegate * delegate = new SVDBModelDelegate(this, Role_BuiltIn, Role_Local, Role_Referenced);
	m_ui->tableViewClimateFiles->setItemDelegate(delegate);
	SVStyle::formatDatabaseTableView(m_ui->tableViewClimateFiles);

	// stretch column with city name
	m_ui->tableViewClimateFiles->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVSimulationLocationOptions::onModified);

	// populate combo boxes
	m_ui->comboBoxTimeZone->blockSignals(true);
	for (int i=-12; i<13; ++i) {
		m_ui->comboBoxTimeZone->addItem(tr("UTC %1%2").arg(i>=0 ? "+" : "").arg(i), i);
	}
	m_ui->comboBoxTimeZone->blockSignals(false);

	m_ui->lineEditLatitude->setup(-90, 90, tr("Latitude in degrees, -90 (south pole) to +90 (north pole).") );
	m_ui->lineEditLongitude->setup(-180, 180, tr("Longitude in degrees -180 (west) to +180 (east).") );

	// Albedo values from VDI 3789-2
	m_ui->comboBoxAlbedo->blockSignals(true);
	m_ui->comboBoxAlbedo->addItem(tr("Dry leveled soil - 0.2"), 0.2);
	m_ui->comboBoxAlbedo->addItem(tr("Clay soil - 0.23"), 0.23);
	m_ui->comboBoxAlbedo->addItem(tr("Light sand - 0.37"), 0.37);
	m_ui->comboBoxAlbedo->addItem(tr("Coniferous forest - 0.12"), 0.12);
	m_ui->comboBoxAlbedo->addItem(tr("Deciduous forest - 0.2"), 0.2);
	m_ui->comboBoxAlbedo->addItem(tr("Pavement - 0.15"), 0.15);
	m_ui->comboBoxAlbedo->addItem(tr("Red tiles - 0.33"), 0.33);
	m_ui->comboBoxAlbedo->addItem(tr("Wet medium grained snow - 0.64"), 0.64);
	m_ui->comboBoxAlbedo->addItem(tr("Dry new fallen snow - 0.82"), 0.82);
	m_ui->comboBoxAlbedo->setCompleter(nullptr); // no auto-completion, otherwise we have text in a value-only combo box
	m_ui->comboBoxAlbedo->blockSignals(false);

	// setup and connect combobox
	m_ui->comboBoxAlbedo->setup(0, 1, tr("Albedo of ambient ground surface"), false, true);
	connect(m_ui->comboBoxAlbedo, &QtExt::ValueInputComboBox::editingFinishedSuccessfully,
			this, &SVSimulationLocationOptions::on_comboboxAlbedoEditingFinishedSuccessfully);

	m_ui->filepathClimateDataFile->setup("", true, true, tr("Climate data container files (*.c6b *.epw *.wac *dat);;All files (*.*)"),
										 SVSettings::instance().m_dontUseNativeDialogs);

	QButtonGroup *group = new QButtonGroup;
	group->addButton(m_ui->radioButtonCustomFilePath);
	group->addButton(m_ui->radioButtonFromDB);

	// disable plots initially
	m_ui->plotRelHum->setEnabled(false);
	m_ui->plotWind->setEnabled(false);
	m_ui->plotRadShortWave->setEnabled(false);
	m_ui->plotRadLongWave->setEnabled(false);
	m_ui->plotTemp->setEnabled(false);

	// format plots
	QDate startDate(QDate::currentDate().year(), 1, 1);
	QDate endDate(QDate::currentDate().year() + 1, 1, 1);
	QDateTime startTime = QDateTime(startDate, QTime());
	QDateTime endTime = QDateTime(endDate, QTime());
	formatPlots(startTime, endTime, true);

	m_zoomerRelHum = new QwtPlotZoomer( m_ui->plotRelHum->canvas() );
	m_zoomerTemp = new QwtPlotZoomer( m_ui->plotTemp->canvas() );
	m_zoomerWind = new QwtPlotZoomer( m_ui->plotWind->canvas() );
	m_zoomerRadLongWave = new QwtPlotZoomer( m_ui->plotRadLongWave->canvas() );
	m_zoomerRadShortWave = new QwtPlotZoomer( m_ui->plotRadShortWave->canvas() );
}


SVSimulationLocationOptions::~SVSimulationLocationOptions() {
	delete m_ui;
}


void SVSimulationLocationOptions::updateUi(bool updatingPlotsRequired) {

	bool haveDWDConv = QFileInfo(SVSettings::instance().m_DWDConverterExecutable).exists();
	m_ui->pushButtonOpenDWDConverter->setVisible(haveDWDConv);

	m_ui->tableViewClimateFiles->resizeRowsToContents();

	bool climateFromDB = !m_ui->radioButtonCustomFilePath->isChecked();

	m_ui->tableViewClimateFiles->setEnabled(climateFromDB);
	m_ui->labelTextFilter->setEnabled(climateFromDB);
	m_ui->lineEditTextFilter->setEnabled(climateFromDB);
	m_ui->filepathClimateDataFile->setEnabled(!climateFromDB);
	m_ui->widgetUserPathOptions->setEnabled(!climateFromDB);

	m_ui->radioButtonUserPathAbsolute->setEnabled(!climateFromDB);
	m_ui->radioButtonUserPathRelative->setEnabled(!climateFromDB);

	const SVClimateFileInfo * climateInfoPtr = nullptr;
	if (climateFromDB) {
		QModelIndex srcIndex = m_filterModel->mapToSource(m_ui->tableViewClimateFiles->currentIndex());
		climateInfoPtr = (const SVClimateFileInfo *)srcIndex.data(Role_RawPointer).value<void*>();
	}
	else {
		readUserClimateFileInfo();
		climateInfoPtr = &m_userClimateFileInfo;
	}

	const VICUS::Project &p = project();

	int index = -1;
	double albedo = p.m_location.m_para[NANDRAD::Location::P_Albedo].value;
	for (int i = 0; i < m_ui->comboBoxAlbedo->count(); ++i) {
		if (IBK::near_equal(m_ui->comboBoxAlbedo->itemData(i).toDouble(), albedo)) {
			index = i;
			break;
		}
	}
	m_ui->comboBoxAlbedo->blockSignals(true);
	if (index>-1)
		m_ui->comboBoxAlbedo->setCurrentIndex(index);
	else
		m_ui->comboBoxAlbedo->setValue(albedo);
	m_ui->comboBoxAlbedo->blockSignals(false);


	// if no location: clear and return
	if (climateInfoPtr == nullptr) {
		m_ui->lineEditLatitude->clear();
		m_ui->lineEditLongitude->clear();
		m_ui->comboBoxTimeZone->setCurrentIndex(13);
		return;
	}
	else {
		// update the location info values (get_value in try statement)
		try {
			m_ui->lineEditLatitude->setValue(p.m_location.m_para[NANDRAD::Location::P_Latitude].get_value("Deg"));
			m_ui->lineEditLongitude->setValue(p.m_location.m_para[NANDRAD::Location::P_Longitude].get_value("Deg"));
		}  catch (...) {
		}
		m_ui->comboBoxTimeZone->setCurrentIndex(p.m_location.m_timeZone);
	}

	// finally update location info text and Plots
	updateLocationInfoText(climateInfoPtr);

	if (updatingPlotsRequired)
		updatePlots(climateInfoPtr);
}


void SVSimulationLocationOptions::updatePlots(const SVClimateFileInfo * climateInfoPtr) {

	m_ui->plotRadShortWave->detachItems();
	m_ui->plotRadLongWave->detachItems();
	m_ui->plotRelHum->detachItems();
	m_ui->plotWind->detachItems();
	m_ui->plotTemp->detachItems();
	m_ui->plotRadShortWave->setEnabled(false);
	m_ui->plotRadLongWave->setEnabled(false);
	m_ui->plotRelHum->setEnabled(false);
	m_ui->plotWind->setEnabled(false);
	m_ui->plotTemp->setEnabled(false);

	// no valid climate data selected, bail out
	if (climateInfoPtr == nullptr || climateInfoPtr->m_absoluteFilePath.isEmpty()) {
		// clear all plots
		m_ui->plotRadShortWave->replot();
		m_ui->plotRadLongWave->replot();
		m_ui->plotRelHum->replot();
		m_ui->plotWind->replot();
		m_ui->plotTemp->replot();
		return;
	}

	// data reading may take some time ...
	QProgressDialog progDiag(tr("Reading ..."), tr("Cancel"), 0, 2, this);
	progDiag.setWindowTitle(tr("Reading climate file"));
	progDiag.setMinimumDuration(200);
	progDiag.setAutoClose(true);
	progDiag.setValue(1);
	qApp->processEvents();

	// read data
	m_climateTimePoints.clear();
	m_loader.initDataWithDefault();
	m_loader.readClimateData(IBK::Path(climateInfoPtr->m_absoluteFilePath.toStdString()), false);

	QDate startDate(QDate::currentDate().year(), 1, 1);
	QDateTime startTime(startDate, QTime(), Qt::UTC);
	QDate endDate(QDate::currentDate().year() + 1, 1, 1);
	QDateTime endTime(endDate, QTime(), Qt::UTC);

	if (m_loader.m_dataTimePoints.empty()) {
		for (unsigned int i=0; i<8760; ++i)
			m_climateTimePoints.push_back( (double)startTime.toMSecsSinceEpoch() +  (double)i*3600*1000 );
	}
	else {
		m_climateTimePoints = m_loader.m_dataTimePoints;
	}

	// enable plots
	m_ui->plotRadShortWave->setEnabled(m_loader.m_checkBits[CCM::ClimateDataLoader::DirectRadiationNormal] == 0);
	m_ui->plotRadLongWave->setEnabled(m_loader.m_checkBits[CCM::ClimateDataLoader::LongWaveCounterRadiation] == 0);
	m_ui->plotRelHum->setEnabled(m_loader.m_checkBits[CCM::ClimateDataLoader::RelativeHumidity] == 0);
	m_ui->plotWind->setEnabled(m_loader.m_checkBits[CCM::ClimateDataLoader::WindVelocity] == 0);
	m_ui->plotTemp->setEnabled(m_loader.m_checkBits[CCM::ClimateDataLoader::Temperature] == 0);

	// create a new curve to be shown in the plot and set some properties
	QwtPlotCurve *curveRadShortNormal = new QwtPlotCurve();
	QwtPlotCurve *curveRadShortHorizontal = new QwtPlotCurve();
	QwtPlotCurve *curveRadLongWave = new QwtPlotCurve();
	QwtPlotCurve *curveRelHum = new QwtPlotCurve();
	QwtPlotCurve *curveWind = new QwtPlotCurve();
	QwtPlotCurve *curveTemp = new QwtPlotCurve();

	QColor colorShortWave = QColor("#E9B44C");
	QColor colorLongWave = QColor("#833ac7");
	QColor colorTemperatureAndHumidity = QColor("#AA0000");
	QColor colorWind = QColor("#004E98");
	QColor colorRelHum;
	if (SVSettings::instance().m_theme == SVSettings::TT_Dark)
		colorRelHum = QColor("#88c1f7");
	else
		colorRelHum = QColor("#022c54");

	curveTemp->setPen( colorTemperatureAndHumidity, 1 ); // color and thickness in pixels
	curveTemp->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // use antialiasing

	curveRadShortNormal->setPen( colorShortWave, 1 ); // color and thickness in pixels
	curveRadShortNormal->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // use antialiasing

	curveRadShortHorizontal->setPen( colorShortWave.darker(), 1 ); // color and thickness in pixels
	curveRadShortHorizontal->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // use antialiasing

	curveRadLongWave->setPen( colorLongWave, 1 ); // color and thickness in pixels
	curveRadLongWave->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // use antialiasing

	curveWind->setPen( colorWind, 1 ); // color and thickness in pixels
	curveWind->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // use antialiasing

	curveRelHum->setPen( colorRelHum, 1 ); // color and thickness in pixels
	curveRelHum->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // use antialiasing


	// set curves
	if (m_ui->plotTemp->isEnabled()) {
		curveTemp->setRawSamples(m_climateTimePoints.data(), m_loader.m_data[CCM::ClimateDataLoader::Temperature].data(), m_climateTimePoints.size());
		curveTemp->attach(m_ui->plotTemp);
		m_ui->plotTemp->replot();
		m_ui->plotTemp->show();
	}
	if (m_ui->plotRadShortWave->isEnabled()) {
		curveRadShortNormal->setRawSamples(m_climateTimePoints.data(), m_loader.m_data[CCM::ClimateDataLoader::DirectRadiationNormal].data(), m_climateTimePoints.size());
		curveRadShortNormal->attach(m_ui->plotRadShortWave);
		curveRadShortHorizontal->setRawSamples(m_climateTimePoints.data(), m_loader.m_data[CCM::ClimateDataLoader::DiffuseRadiationHorizontal].data(), m_climateTimePoints.size());
		curveRadShortHorizontal->attach(m_ui->plotRadShortWave);
		m_ui->plotRadShortWave->replot();
		m_ui->plotRadShortWave->show();
	}
	if (m_ui->plotRadLongWave->isEnabled()) {
		curveRadLongWave->setRawSamples(m_climateTimePoints.data(), m_loader.m_data[CCM::ClimateDataLoader::LongWaveCounterRadiation].data(), m_climateTimePoints.size());
		curveRadLongWave->attach(m_ui->plotRadLongWave);
		m_ui->plotRadLongWave->replot();
		m_ui->plotRadLongWave->show();
	}
	if (m_ui->plotRelHum->isEnabled()) {
		curveRelHum->setRawSamples(m_climateTimePoints.data(), m_loader.m_data[CCM::ClimateDataLoader::RelativeHumidity].data(), m_climateTimePoints.size());
		curveRelHum->attach(m_ui->plotRelHum);
		m_ui->plotRelHum->replot();
		m_ui->plotRelHum->show();
	}
	if (m_ui->plotWind->isEnabled()) {
		curveWind->setRawSamples(m_climateTimePoints.data(), m_loader.m_data[CCM::ClimateDataLoader::WindVelocity].data(), m_climateTimePoints.size());
		curveWind->attach(m_ui->plotWind);
		m_ui->plotWind->replot();
		m_ui->plotWind->show();
	}

	// axis settings

	// not cyclic climate data ?
	if (!m_loader.m_dataTimePoints.empty()) {
		// modify start time
		int startYear = m_loader.m_startYear;
		startTime = QDateTime(QDate(startYear,1,1), QTime());
		// and end time
		qint64 secondsToAdd = static_cast<qint64>(m_loader.m_dataTimePoints.back() );
		endTime = startTime.addSecs(secondsToAdd);
	}

	formatPlots(startTime, endTime, false);

	// reformat the now initialized plots to align left axes
	int maxAxisWidth = std::max({m_ui->plotRelHum->axisWidget(QwtPlot::yLeft)->width(),
								 m_ui->plotTemp->axisWidget(QwtPlot::yLeft)->width(),
								 m_ui->plotWind->axisWidget(QwtPlot::yLeft)->width(),
								 m_ui->plotRadLongWave->axisWidget(QwtPlot::yLeft)->width(),
								 m_ui->plotRadShortWave->axisWidget(QwtPlot::yLeft)->width()});

	m_ui->plotRelHum->setContentsMargins(maxAxisWidth-m_ui->plotRelHum->axisWidget(QwtPlot::yLeft)->width(),0,0,0);
	m_ui->plotTemp->setContentsMargins(maxAxisWidth-m_ui->plotTemp->axisWidget(QwtPlot::yLeft)->width(),0,0,0);
	m_ui->plotWind->setContentsMargins(maxAxisWidth-m_ui->plotWind->axisWidget(QwtPlot::yLeft)->width(),0,0,0);
	m_ui->plotRadShortWave->setContentsMargins(maxAxisWidth-m_ui->plotRadShortWave->axisWidget(QwtPlot::yLeft)->width(),0,0,0);
	m_ui->plotRadLongWave->setContentsMargins(maxAxisWidth-m_ui->plotRadLongWave->axisWidget(QwtPlot::yLeft)->width(),0,0,0);


	m_zoomerRelHum->setZoomBase();
	m_zoomerTemp->setZoomBase();
	m_zoomerWind->setZoomBase();
	m_zoomerRadLongWave->setZoomBase();
	m_zoomerRadShortWave->setZoomBase();

	progDiag.setValue(2);
	qApp->processEvents();
}


void SVSimulationLocationOptions::formatPlots(const QDateTime & start, const QDateTime & end, bool init){
	formatQwtPlot(init, *m_ui->plotTemp, start, end, "Ambient Temperature", "T [C]", -20, 40);
	formatQwtPlot(init, *m_ui->plotRadLongWave, start, end, "Long Wave Radiation", "I [W/m²]", 0, 1000);
	formatQwtPlot(init, *m_ui->plotRadShortWave, start, end, "Shortwave Radiation", "I [W/m²]", 0, 1400);
	formatQwtPlot(init, *m_ui->plotWind, start, end, "Wind speed", "v [m/s]", 0, 40);
	formatQwtPlot(init, *m_ui->plotRelHum, start, end, "Relative Humidity", "r.H. [%]", 0, 100);
}


void SVSimulationLocationOptions::minMaxValuesInPlot(const QwtPlot & plot, double &minY, double &maxY) {
	// Get the list of all attached items; could be curves, grids, etc.
	QList<QwtPlotItem*> itemList = plot.itemList();
	QwtPlotCurve *curve = nullptr;
	// Iterate through the list to find your curve
	for (QwtPlotItem *item : itemList) {
		if (item->rtti() == QwtPlotItem::Rtti_PlotCurve) {  // Check if it's a curve
			curve = static_cast<QwtPlotCurve*>(item);
			break;
		}
	}

	if (curve != nullptr) {
		// Now myCurve points to your curve and you can do something with it
		const QwtSeriesData<QPointF> * seriesData = curve->data();

		minY = std::numeric_limits<double>::max();
		maxY = std::numeric_limits<double>::min();
		for (size_t i = 0; i < seriesData->size(); ++i) {
			QPointF sample = seriesData->sample(i);
			// Update min and max y-values
			minY = std::min(minY, sample.y());
			maxY = std::max(maxY, sample.y());
		}
	}
}


void SVSimulationLocationOptions::formatQwtPlot(bool init, QwtPlot &plot, QDateTime start, QDateTime end, QString title, QString leftYAxisTitle, double yLeftMin, double yLeftMax, unsigned int yNumSteps,
							   bool hasRightAxis, QString rightYAxisTitle, double yRightMin, double yRightMax, double yRightStepSize) {

	// we set also the time spec
	start.setTimeSpec(Qt::UTC);
	end.setTimeSpec(Qt::UTC);

	// initialize all major ticks in grid and Axis
	QList<double> majorTicks;

	// assume an average month has 30 days
	unsigned int days = start.daysTo(end);
	unsigned int months = days / 30;

	for(unsigned int i=0; i<months/2; ++i)
		majorTicks.push_back(QwtDate::toDouble(start.addMonths(2*(int)i) ) );

	// Init Scale Divider
	QwtScaleDiv scaleDiv(QwtDate::toDouble(start), QwtDate::toDouble(end), QList<double>(), QList<double>(), majorTicks);

	// inti plot title
	QFont font;
	font.setPointSize(10);
	QwtText qwtTitle;
	qwtTitle.setFont(font);
	qwtTitle.setText(title);

	// Set plot title
	plot.setTitle(qwtTitle);

	// try to find accurate min, max values
	minMaxValuesInPlot(plot, yLeftMin, yLeftMax);

	if (yLeftMin>0)
		yLeftMin *= 0.9;
	else
		yLeftMin *= 1.1;
	if (yLeftMax>0)
		yLeftMax *= 1.1;
	else
		yLeftMax *= 0.9;

	// Scale all y axises
	double yLeftStepSize = (yLeftMax - yLeftMin) / double(yNumSteps);
	plot.setAxisScale(QwtPlot::yLeft, yLeftMin, yLeftMax, yLeftStepSize);
	plot.setAxisFont(QwtPlot::yLeft, font);

	// Init QWT Text
	QwtText axisTitle;
	axisTitle.setFont(font);

	// left Axis title
	axisTitle.setText(leftYAxisTitle);
	plot.setAxisTitle(QwtPlot::yLeft, axisTitle);

	// do we have a right y axis?
	if(hasRightAxis) {
		plot.setAxisFont(QwtPlot::yRight, font);
		plot.enableAxis(QwtPlot::yRight, true);
		plot.setAxisScale(QwtPlot::yRight, yRightMin, yRightMax, yRightStepSize);

		// right Axis title
		axisTitle.setText(rightYAxisTitle);
		plot.setAxisTitle(QwtPlot::yRight, axisTitle);
		plot.setTitle(title);
	}

	// Bottom axis
	plot.setAxisFont(QwtPlot::xBottom, font);

	// Canvas Background
	plot.setCanvasBackground(Qt::white);

	// Init Scale draw engine
	QwtDateScaleDraw *scaleDrawTemp = new QwtDateScaleDraw(Qt::UTC);
	scaleDrawTemp->setDateFormat(QwtDate::Month, "MMM");
	scaleDrawTemp->setDateFormat(QwtDate::Year, "yyyy");

	QwtDateScaleEngine *scaleEngine = new QwtDateScaleEngine(Qt::UTC);

	plot.enableAxis(QwtPlot::xBottom, !init && plot.isEnabled());
	// Set scale draw engine
	plot.setAxisScaleDraw(QwtPlot::xBottom, scaleDrawTemp);
	plot.setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);
	plot.setMinimumWidth(350);


	// Init Grid
	QwtPlotGrid *grid = new QwtPlotGrid;
	grid->enableXMin(true);
	grid->enableYMin(true);
	grid->enableX(true);
	grid->enableY(true);
	grid->setVisible(true);
	grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
	grid->setMinorPen(QPen(Qt::NoPen));
	grid->attach(&plot);

	plot.replot();
}


void SVSimulationLocationOptions::onModified(int modificationType, ModificationInfo * /*data*/) {

	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;

	switch (modType) {
		case SVProjectHandler::AllModified: {

			// The following update needs to be done only when the project has changed

			m_ui->filepathClimateDataFile->setFilename("");

			bool climateFromDB;
			m_ui->radioButtonCustomFilePath->blockSignals(true);
			const VICUS::Project &p = project();
			// do we have a climate file path given?
			if (p.m_location.m_climateFilePath.isValid()) {
				// is the referenced file in the climate database? If so, it has a placeholder "${Database}" in path or
				// ${User Database}
				QModelIndex idx;
				for (int i=0, count = m_climateDataModel->rowCount(QModelIndex()); i< count; ++i) {
					QModelIndex curIdx = m_climateDataModel->index(i, 0);
					// get the path including potential placeholders
					IBK::Path path(m_climateDataModel->data(curIdx, Role_FilePath).toString().toStdString());
					if (path == p.m_location.m_climateFilePath) {
						idx = curIdx;
						break;
					}
					// now try the absolute path, maybe that matches
					path = m_climateDataModel->data(curIdx, Role_AbsoluteFilePath).toString().toStdString();
					if (path == p.m_location.m_climateFilePath) {
						idx = curIdx;
						break;
					}
				}

				// now update state of radio buttons "from database" / "custom file path" and select row in table view or custom file name

				climateFromDB = idx.isValid();
				if (climateFromDB) {
					m_ui->radioButtonCustomFilePath->setChecked(false);

					// convert to proxy-index
					QModelIndex proxy = m_filterModel->mapFromSource(idx);
					// if not visible, reset all filters and convert to proxy index again
					if (!proxy.isValid()) {
						m_filterModel->setFilterText("");
					}
					proxy = m_filterModel->mapFromSource(idx);
					// select row
					m_ui->tableViewClimateFiles->selectionModel()->blockSignals(true);
					m_ui->tableViewClimateFiles->setCurrentIndex(proxy);
					m_ui->tableViewClimateFiles->selectionModel()->blockSignals(false);
				}
				else {
					// not a database file, might still contain a placeholder
					IBK::Path absPath = SVProjectHandler::instance().replacePathPlaceholders(p.m_location.m_climateFilePath);
					absPath.removeRelativeParts(); // remove any remaining ../.. in the middle

					m_ui->radioButtonCustomFilePath->setChecked(true);
					// file is not contained in the database or user database, assume absolute file path
					m_ui->filepathClimateDataFile->blockSignals(true);
					m_ui->filepathClimateDataFile->setFilename(QString::fromStdString(absPath.str()) );
					m_ui->filepathClimateDataFile->blockSignals(false);

					// select the correct radio button
					m_ui->radioButtonUserPathAbsolute->blockSignals(true);
					if (p.m_location.m_climateFilePath.str().find("${Project Directory}") != std::string::npos)
						m_ui->radioButtonUserPathRelative->setChecked(true);
					else
						m_ui->radioButtonUserPathAbsolute->setChecked(true);
					m_ui->radioButtonUserPathAbsolute->blockSignals(false);
				}
			}
			else {
				// no climate file path, but we assume that user wants to use DB climate
				m_ui->radioButtonCustomFilePath->setChecked(false);
			}
			m_ui->radioButtonCustomFilePath->blockSignals(false);

			// for some reason we need to manually set the checked state of the DB radio button as well.
			m_ui->radioButtonFromDB->blockSignals(true);
			m_ui->radioButtonFromDB->setChecked(!m_ui->radioButtonCustomFilePath->isChecked());
			m_ui->radioButtonFromDB->blockSignals(false);

			// finally update remaining ui
			updateUi(true);
		} break;

		// in case just the location was modified, only update ui
		case SVProjectHandler::ClimateLocationModified: {
			updateUi(false);
		} break;

		// in case just the location was modified, only update ui
		case SVProjectHandler::ClimateLocationAndFileModified: {
			updateUi(true);
		} break;

		default:;
	}
}


void SVSimulationLocationOptions::on_tableViewClimateFiles_clicked(const QModelIndex &index) {
	if (!m_ui->radioButtonCustomFilePath->isChecked()) {
		// get filename from current model and then update the climate station info text box
		QModelIndex srcIndex = m_filterModel->mapToSource(index);
		const SVClimateFileInfo * climateInfoPtr = (const SVClimateFileInfo *)srcIndex.data(Role_RawPointer).value<void*>();
		modifyClimateFileAndLocation(climateInfoPtr);
	}
}


void SVSimulationLocationOptions::updateLocationInfoText(const SVClimateFileInfo * climateInfoPtr) {

	// clear info text on climate location
	m_ui->textBrowserDescription->clear();

	// no valid climate data selected, bail out
	if (climateInfoPtr == nullptr)
		return;

	QString infoText;
	infoText = "<html><body>";
	if (climateInfoPtr->m_name.isEmpty()) {
		infoText += "<p>" + tr("Invalid climate data file path.") + "</p>";
	}
	else {
		if (m_ui->radioButtonCustomFilePath->isChecked()) {
			infoText += "<p>" + tr("User climate data file.");
		}
		else {
			if (!climateInfoPtr->m_builtIn)
				infoText += "<p>" + tr("Climate data from user database.");
			else
				infoText += "<p>" + tr("Climate data from standard database.");
		}
		infoText += "<br>" + climateInfoPtr->m_timeBehaviour + "</p>";
		infoText += "<p>" + tr("City/Country") + ": <b>" + climateInfoPtr->m_city + "</b>/";
		infoText += "<b>" + climateInfoPtr->m_country + "</b>, ";
		infoText += tr("Source") + ": <b>" + climateInfoPtr->m_source + "</b><br>";
		infoText += tr("Longitude") + ": <b>" + QString("%L1 Deg").arg(climateInfoPtr->m_longitudeInDegree, 0, 'f', 2) + " </b>, ";
		infoText += tr("Latitude") + ": <b>" + QString("%L1 Deg").arg(climateInfoPtr->m_latitudeInDegree, 0, 'f', 2) + " </b>, ";
		infoText += tr("Elevation") + ": <b>" + QString("%L1 m").arg(climateInfoPtr->m_elevation, 0, 'f', 0) + " </b></p>";
		infoText += "<p>" + climateInfoPtr->m_comment + "</p>";
	}
	infoText += "</body></html>";

	m_ui->textBrowserDescription->setHtml(infoText);
	QFont f;
	f.setPointSizeF(f.pointSizeF());
	m_ui->textBrowserDescription->setFont(f);

}


void SVSimulationLocationOptions::modifyClimateFileAndLocation(const SVClimateFileInfo * climateInfoPtr) {

	NANDRAD::Location location = project().m_location;
	if (climateInfoPtr == nullptr || climateInfoPtr->m_absoluteFilePath.isEmpty()) {
		location.m_climateFilePath.clear();
		location.m_para[NANDRAD::Location::P_Latitude] = IBK::Parameter();
		location.m_para[NANDRAD::Location::P_Latitude] = IBK::Parameter();
		location.m_timeZone = 13;
	}
	else {
		location.m_climateFilePath = climateInfoPtr->m_absoluteFilePath.toStdString();
		NANDRAD::KeywordList::setParameter(location.m_para, "Location::para_t",
										   NANDRAD::Location::P_Latitude, climateInfoPtr->m_latitudeInDegree);
		NANDRAD::KeywordList::setParameter(location.m_para, "Location::para_t",
										   NANDRAD::Location::P_Longitude, climateInfoPtr->m_longitudeInDegree);
		location.m_timeZone = climateInfoPtr->m_timeZone /*+ 12*/;
	}

	SVUndoModifyClimate *undo = new SVUndoModifyClimate("Climate file changed", location, true);
	undo->push();
}


void SVSimulationLocationOptions::readUserClimateFileInfo() {
	QString climateFile = m_ui->filepathClimateDataFile->filename().trimmed();
	m_userClimateFileInfo = SVClimateFileInfo();
	if (climateFile.isEmpty()) {
		return;
	}
	try {
		m_userClimateFileInfo.readInfo(QString(), climateFile, false, true);
	}
	catch (...) {
		m_userClimateFileInfo = SVClimateFileInfo();
	}
}


void SVSimulationLocationOptions::modifyLocationFromLineEdits(){
	NANDRAD::Location location = project().m_location;

	NANDRAD::KeywordList::setParameter(location.m_para, "Location::para_t",
									   NANDRAD::Location::P_Albedo, m_ui->comboBoxAlbedo->currentData().toDouble()*100);
	NANDRAD::KeywordList::setParameter(location.m_para, "Location::para_t",
									   NANDRAD::Location::P_Latitude, m_ui->lineEditLatitude->value());
	NANDRAD::KeywordList::setParameter(location.m_para, "Location::para_t",
									   NANDRAD::Location::P_Longitude, m_ui->lineEditLongitude->value());
	location.m_timeZone = m_ui->comboBoxTimeZone->currentData().toInt() + 12;

	SVUndoModifyClimate *undo = new SVUndoModifyClimate("Location changed", location, false);
	undo->push();
}


void SVSimulationLocationOptions::on_lineEditTextFilter_editingFinished() {
	// update the filter text
	m_filterModel->setFilterWildcard(m_ui->lineEditTextFilter->text().trimmed());
}


void SVSimulationLocationOptions::on_lineEditTextFilter_textChanged(const QString &arg1) {
	m_filterModel->setFilterWildcard(arg1);
}


void SVSimulationLocationOptions::on_filepathClimateDataFile_editingFinished() {
	readUserClimateFileInfo();
	modifyClimateFileAndLocation(&m_userClimateFileInfo);
	on_radioButtonUserPathAbsolute_toggled(m_ui->radioButtonUserPathAbsolute->isChecked());
}


void SVSimulationLocationOptions::on_radioButtonUserPathAbsolute_toggled(bool checked) {
	// update local file path
	// we need a project file path for that
	QString p = SVProjectHandler::instance().projectFile();
	if (p.isEmpty())
		checked = true; // fall back to absolute path if project file hasn't been saved, yet

	QString composedFilePath;
	if (checked) {
		composedFilePath = m_ui->filepathClimateDataFile->filename();
	}
	else {
		IBK::Path proPath(p.toStdString());
		proPath = proPath.parentPath();
		IBK::Path climateFilePath(m_ui->filepathClimateDataFile->filename().toStdString());
		try {
			IBK::Path relPath = climateFilePath.relativePath(proPath);
			composedFilePath = QString::fromStdString( (IBK::Path("${Project Directory}") / relPath).str());
		} catch (...) {
			// can't relate paths... keep absolute
			composedFilePath = m_ui->filepathClimateDataFile->filename();
		}
	}

	// finally update project data
	NANDRAD::Location location = project().m_location;
	location.m_climateFilePath = IBK::Path(composedFilePath.toStdString());

	SVUndoModifyClimate *undo = new SVUndoModifyClimate("Climate file changed", location, false); // "false" because climate file path has not really changed
	undo->push();
}


void SVSimulationLocationOptions::on_lineEditLatitude_editingFinishedSuccessfully() {
	modifyLocationFromLineEdits();
}


void SVSimulationLocationOptions::on_lineEditLongitude_editingFinishedSuccessfully() {
	modifyLocationFromLineEdits();
}


void SVSimulationLocationOptions::on_comboBoxTimeZone_activated(int /*index*/) {
	modifyLocationFromLineEdits();
}


void SVSimulationLocationOptions::on_comboboxAlbedoEditingFinishedSuccessfully() {
	double val = m_ui->comboBoxAlbedo->currentData().toDouble();
	m_ui->comboBoxAlbedo->setValue(val);
	NANDRAD::Location location = project().m_location;
	NANDRAD::KeywordList::setParameter(location.m_para, "Location::para_t",
									   NANDRAD::Location::P_Albedo, m_ui->comboBoxAlbedo->currentData().toDouble()*100);

	SVUndoModifyClimate *undo = new SVUndoModifyClimate("Location changed", location, false);
	undo->push();
}



void SVSimulationLocationOptions::on_radioButtonCustomFilePath_toggled(bool checked) {
	m_ui->radioButtonUserPathAbsolute->setEnabled(checked);
	m_ui->radioButtonUserPathRelative->setEnabled(checked);

	if (checked) {
		readUserClimateFileInfo();
		modifyClimateFileAndLocation(&m_userClimateFileInfo);
	}
	else {
		on_tableViewClimateFiles_clicked(m_ui->tableViewClimateFiles->currentIndex());
	}
}


void SVSimulationLocationOptions::on_pushButtonOpenDWDConverter_clicked() {
	QString dwdPath = SVSettings::instance().m_DWDConverterExecutable;
	if (dwdPath.isEmpty() || !QFileInfo::exists(dwdPath)) {
		QMessageBox::information(this, tr("Setup external tool"), tr("Please select first the path to the external "
																	 "climate editor in the preferences dialog!"));
		return;
	}
	bool res = QProcess::startDetached(dwdPath, QStringList(), QString());
	if (!res) {
		QMessageBox::critical(this, tr("Error starting external application"), tr("Climate editor '%1' could not be started.")
							  .arg(dwdPath));
	}
}

