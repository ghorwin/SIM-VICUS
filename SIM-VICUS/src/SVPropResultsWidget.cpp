#include "SVPropResultsWidget.h"
#include "ui_SVPropResultsWidget.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVStyle.h"
#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "SVColorLegend.h"

#include <QFileInfo>
#include <QTextStream>
#include <QProgressDialog>

#include <IBK_CSVReader.h>


SVPropResultsWidget::SVPropResultsWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropResultsWidget)
{
	m_ui->setupUi(this);

	m_ui->resultsDir->setup("", false, true, "", SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->tableWidgetAvailableResults->setColumnCount(3);
	m_ui->tableWidgetAvailableResults->setHorizontalHeaderLabels(QStringList() << tr("") << tr("Name") << tr("Unit") );
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetAvailableResults);
	m_ui->tableWidgetAvailableResults->setSortingEnabled(false);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->resizeSection(0,10);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->resizeSection(1,300);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->resizeSection(2,30);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetAvailableResults->setSelectionMode(QAbstractItemView::SingleSelection);
	m_ui->tableWidgetAvailableResults->setSelectionBehavior(QAbstractItemView::SelectRows);

	m_ui->comboBoxPipeType->clear();
	m_ui->comboBoxPipeType->addItem(tr("SupplyPipe"), "SupplyPipe");
	m_ui->comboBoxPipeType->addItem(tr("ReturnPipe"), "ReturnPipe");

	m_ui->doubleSpinBoxMaxValue->setMaximum(std::numeric_limits<double>::max());
	m_ui->doubleSpinBoxMaxValue->setMinimum(std::numeric_limits<double>::lowest());
	m_ui->doubleSpinBoxMinValue->setMaximum(std::numeric_limits<double>::max());
	m_ui->doubleSpinBoxMinValue->setMinimum(std::numeric_limits<double>::lowest());

	m_ui->pushButtonMaxColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->pushButtonMaxColor->setColor("#6f1d1b");
	m_maxColor = m_ui->pushButtonMaxColor->color();
	m_ui->pushButtonMinColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->pushButtonMinColor->setColor("#669bbc");
	m_minColor = m_ui->pushButtonMinColor->color();

	connect(m_ui->widgetTimeSlider, &SVTimeSliderWidget::cutValueChanged,
			this, &SVPropResultsWidget::onTimeSliderCutValueChanged);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropResultsWidget::onModified);

	// for the very first start: project handler has just been connected and we have missed the modified signal,
	// but still want to update our UI
	if (SVProjectHandler::instance().isValid())
		clearAll();

	// set pointer for color legend
	SVViewStateHandler::instance().m_geometryView->colorLegend()->init(&m_currentMin, &m_currentMax, &m_minColor, &m_maxColor);
}


SVPropResultsWidget::~SVPropResultsWidget() {
	delete m_ui;
}


void SVPropResultsWidget::onModified(int modificationType, ModificationInfo * /*data*/) {

	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::NetworkGeometryChanged:
		case SVProjectHandler::BuildingGeometryChanged: {
			clearAll();
		} break;
		default: ; // just to make compiler happy
	}
}


void SVPropResultsWidget::clearAll() {
	m_ui->resultsDir->setFilename("");
	m_ui->tableWidgetAvailableResults->setRowCount(0);
	m_ui->doubleSpinBoxMaxValue->setValue(0);
	m_ui->doubleSpinBoxMinValue->setValue(0);
	m_allResults.clear();
	m_availableOutputUnits.clear();

	m_ui->widgetTimeSlider->clear();

	// sets all object colors grey
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_objectColorMode = SVViewState::OCM_ResultColorView;
	SVViewStateHandler::instance().setViewState(vs);

	m_ui->groupBoxNetworkOptions->setEnabled(!project().m_geometricNetworks.empty());

	on_toolButtonSetDefaultDirectory_clicked();
}


void SVPropResultsWidget::readResultsDir() {
	FUNCID(SVPropResultsWidget::readResultsDir());

	m_availableOutputs.clear();
	m_availableOutputUnits.clear();
	m_outputFile.clear();
	m_allResults.clear();

	if (m_resultsDir.absolutePath().endsWith("/results"))
		m_resultsDir.cdUp();

	// read substitution file to get object name and VICUS ID
	m_objectName2Id.clear();
	QFile subsFile(m_resultsDir.absolutePath() + "/var/objectref_substitutions.txt");
	if (subsFile.open(QFile::ReadOnly)){
		QTextStream in(&subsFile);
		in.setCodec("UTF-8");
		QString line;
		while (in.readLineInto(&line)) {
			QStringList fields = line.split("\t");
			if (fields.size()==2) {
				// extract the id
				int start = fields[1].indexOf("ID=")+3;
				int end = fields[1].lastIndexOf(")");
				unsigned int id = fields[1].mid(start, end-start).toUInt();
				m_objectName2Id[fields[1]] = id;
			}
		}
	}
	else {
		throw IBK::Exception(IBK::FormatString("Substitution file '%1' not found!").arg(subsFile.fileName().toStdString()), FUNC_ID);
	}

	// now read result tsv files
	m_resultsDir = QDir(m_resultsDir.absolutePath() + "/results");
	QStringList tsvFiles = m_resultsDir.entryList(QStringList() << "*.tsv", QDir::Files);
	for (const QString &fileName: tsvFiles) {
		// read tsv header
		IBK::CSVReader reader;
		reader.read(IBK::Path(m_resultsDir.absoluteFilePath(fileName).toStdString()), true, true);
		std::vector<std::string> captions = reader.m_captions;
		std::vector<std::string> units = reader.m_units;
		if (captions.size() != units.size() || captions.empty())
			continue;

		// we go through the tsv header and look for our object names
		for (unsigned int i=0; i<captions.size(); ++i) {
			QString caption = QString::fromStdString(captions[i]);
			QString unit = QString::fromStdString(units[i]);
			QString outputName;
			for (auto it=m_objectName2Id.begin(); it!=m_objectName2Id.end(); ++it) {
				if (caption.contains(it->first)){
					outputName = caption.remove(it->first + ".");
					m_availableOutputs[outputName].push_back(it->second); // store output name to ids
					m_availableOutputUnits[outputName] = unit;
					m_outputFile[outputName] = m_resultsDir.absoluteFilePath(fileName);
				}
			}
		}
	}

	// update table widget
	m_ui->tableWidgetAvailableResults->clearContents();
	m_ui->tableWidgetAvailableResults->setRowCount(m_availableOutputs.size());
	int row=0;
	for (auto it=m_availableOutputUnits.begin(); it!=m_availableOutputUnits.end(); ++it) {

		QTableWidgetItem * item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setText(it->first);
		m_ui->tableWidgetAvailableResults->setItem(row, 1, item);

		QTableWidgetItem * itemUnit = new QTableWidgetItem;
		itemUnit->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		itemUnit->setText(it->second);
		m_ui->tableWidgetAvailableResults->setItem(row, 2, itemUnit);

		++row;
	}
	m_ui->tableWidgetAvailableResults->selectRow(0);
}


void SVPropResultsWidget::readCurrentResult(bool forceToRead) {

	// set current row bold
	QTableWidgetItem *item = m_ui->tableWidgetAvailableResults->item(m_ui->tableWidgetAvailableResults->currentRow(), 1);
	Q_ASSERT(item!=nullptr);
	QFont font = item->font();
	font.setBold(false);
	for (int col=1; col<m_ui->tableWidgetAvailableResults->columnCount(); ++col) {
		for (int row=0; row < m_ui->tableWidgetAvailableResults->rowCount(); ++row)
			m_ui->tableWidgetAvailableResults->item(row, col)->setFont(font);
	}
	font.setBold(true);
	for (int col=1; col<m_ui->tableWidgetAvailableResults->columnCount(); ++col) {
		m_ui->tableWidgetAvailableResults->item(m_ui->tableWidgetAvailableResults->currentRow(), col)->setFont(font);
	}

	m_currentOutput = item->text();

	QString filter;
	if (!project().m_geometricNetworks.empty())
	filter = m_ui->comboBoxPipeType->currentData().toString();

	// we only read results which have not yet been read and stored
	if (forceToRead || m_allResults.find(m_currentOutput) == m_allResults.end()) {

		QProgressDialog progDiag(tr("Read results..."), tr("Cancel"), 0, 100, this);
		progDiag.setValue(1);

		// read entire file
		IBK::CSVReader reader;
		reader.read(IBK::Path(m_outputFile[m_currentOutput].toStdString()), false, true);
		if (reader.m_nColumns < 2 || reader.m_nRows < 5)
			return;

		progDiag.setMaximum((int)reader.m_nColumns-1);

		// we convert time to seconds so its in accordance with time slider
		std::vector<double> time = reader.colData(0);
		IBK::Unit timeUnit = IBK::Unit(reader.m_units[0]);
		if (timeUnit.base_unit() != IBK::Unit("s"))
			return;
		IBK::UnitVector timeSeconds(time.begin(), time.end(), timeUnit);
		timeSeconds.convert(IBK::Unit("s"));

		for (unsigned int i=0; i<reader.m_captions.size(); ++i) {

			progDiag.setValue((int)i);

			// go through all objects
			for (auto it=m_objectName2Id.begin(); it!=m_objectName2Id.end(); ++it) {
				std::string objectName = it->first.toStdString();
				// look if object name is in this caption
				if (reader.m_captions[i].find(objectName) != std::string::npos){
					// additional filter
					if (filter.isEmpty() || reader.m_captions[i].find(filter.toStdString()) != std::string::npos) {
						QString qCaption = QString::fromStdString(reader.m_captions[i]);
						int start = qCaption.indexOf("ID=")+3;
						int end = qCaption.lastIndexOf(")");
						unsigned int id = qCaption.mid(start, end-start).toUInt();
						QString outputName = qCaption.remove(it->first + ".");
						m_allResults[outputName][id] = NANDRAD::LinearSplineParameter(reader.m_captions[i], NANDRAD::LinearSplineParameter::I_LINEAR,
																				   timeSeconds.m_data, reader.colData(i), timeUnit, IBK::Unit("s"));
					}
				}
			}
		}

		// set slider
		m_ui->widgetTimeSlider->setValues(timeSeconds);
		m_ui->widgetTimeSlider->setCurrentValue(timeSeconds.m_data.back());
	}

	for (int row=0; row<m_ui->tableWidgetAvailableResults->rowCount(); ++row) {
		const QString &property = m_ui->tableWidgetAvailableResults->item(row, 1)->text();
		QTableWidgetItem *item = new QTableWidgetItem();
		if (m_allResults.find(property) != m_allResults.end())
			item->setIcon(QIcon(":/gfx/actions/16x16/ok.png"));
		m_ui->tableWidgetAvailableResults->setItem(row, 0, item);
	}

	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_objectColorMode = SVViewState::OCM_ResultColorView;
	SVViewStateHandler::instance().setViewState(vs);

	// determine max/min values, also updates colors
	setCurrentMinMaxValues(false);

	// set legend title
	item = m_ui->tableWidgetAvailableResults->item(m_ui->tableWidgetAvailableResults->currentRow(), 2);
	Q_ASSERT(item!=nullptr);
	QString currentUnit = item->text();
	SVViewStateHandler::instance().m_geometryView->colorLegend()->setTitle(QString("%1 [%2]").arg(m_currentOutput).arg(currentUnit));
}


void SVPropResultsWidget::onTimeSliderCutValueChanged(double currentTime) {
	updateColors(currentTime);
}


void SVPropResultsWidget::setCurrentMinMaxValues(bool localMinMax) {

	// determine overall max/min
	m_currentMin = std::numeric_limits<double>::max();
	m_currentMax = std::numeric_limits<double>::lowest();
	double currentTime = m_ui->widgetTimeSlider->currentCutValue();
	double max, min;

	for (auto it=m_allResults[m_currentOutput].begin(); it!=m_allResults[m_currentOutput].end(); ++it) {
		const IBK::LinearSpline &vals = it->second.m_values;
		if (localMinMax) {
			max = vals.value(currentTime);
			min = max;
		}
		else {
			max = *std::max_element(vals.y().begin(), vals.y().end());
			min = *std::min_element(vals.y().begin(), vals.y().end());
		}

		if (max > m_currentMax)
			m_currentMax = max;
		if (min < m_currentMin)
			m_currentMin = min;
	}

	m_ui->doubleSpinBoxMaxValue->setValue(m_currentMax);
	m_ui->doubleSpinBoxMinValue->setValue(m_currentMin);
}


void SVPropResultsWidget::interpolateColor(const double & yint, QColor & col) {
	double ys = (yint - m_currentMin) / (m_currentMax - m_currentMin);
	if (ys>1)
		ys=1;
	if (ys<0)
		ys=0;
	double hMax, sMax, vMax, hMin, sMin, vMin;
	m_maxColor.getHsvF(&hMax, &sMax, &vMax);
	m_minColor.getHsvF(&hMin, &sMin, &vMin);
	// only hue is interpolated
	double hNew = hMin + ys * (hMax - hMin);
	col.setHsvF(hNew, sMax, vMax);
}


void SVPropResultsWidget::updateColors(const double &currentTime) {

	QColor col;
	const VICUS::Project &p = project();
	for (const VICUS::Network &net: p.m_geometricNetworks) {
		for (const VICUS::NetworkEdge &e: net.m_edges) {
			if (m_allResults[m_currentOutput].find(e.m_id) != m_allResults[m_currentOutput].end()) {
				double yint = m_allResults[m_currentOutput][e.m_id].m_values.value(currentTime);
				interpolateColor(yint, col);
				e.m_color = col;
			}
		}
	}

	for (const VICUS::Building & b : p.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				// a room related property (e.g. AirTemperature)
				if ( m_allResults[m_currentOutput].find(r.m_id) != m_allResults[m_currentOutput].end() ) {
					double yint = m_allResults[m_currentOutput][r.m_id].m_values.value(currentTime);
					interpolateColor(yint, col);
					for (const VICUS::Surface & s : r.m_surfaces)
						s.m_color = col;
				}
				// a wall related property (e.g. SurfaceTemperature)
				else {
					for (const VICUS::Surface & s : r.m_surfaces) {
						if (m_allResults[m_currentOutput].find(s.m_id) != m_allResults[m_currentOutput].end()) {
							double yint = m_allResults[m_currentOutput][s.m_id].m_values.value(currentTime);
							interpolateColor(yint, col);
							s.m_color = col;
						}
					}
				}
			}
		}
	}

	SVViewStateHandler::instance().repaintGeometry();
	SVViewStateHandler::instance().m_geometryView->colorLegend()->update();
}


void SVPropResultsWidget::on_doubleSpinBoxMaxValue_valueChanged(double arg1) {
	if (arg1*0.95 < m_currentMin) {
		m_ui->doubleSpinBoxMaxValue->blockSignals(true);
		m_ui->doubleSpinBoxMaxValue->setValue(m_currentMax);
		m_ui->doubleSpinBoxMaxValue->blockSignals(false);
		return;
	}
	m_currentMax = arg1;
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


void SVPropResultsWidget::on_doubleSpinBoxMinValue_valueChanged(double arg1) {
	if (arg1*1.05 > m_currentMax) {
		m_ui->doubleSpinBoxMinValue->blockSignals(true);
		m_ui->doubleSpinBoxMinValue->setValue(m_currentMin);
		m_ui->doubleSpinBoxMinValue->blockSignals(false);
		return;
	}
	m_currentMin = arg1;
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


void SVPropResultsWidget::on_pushButtonMaxColor_clicked() {
	m_maxColor = m_ui->pushButtonMaxColor->color();
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


void SVPropResultsWidget::on_pushButtonMinColor_clicked() {
	m_minColor = m_ui->pushButtonMinColor->color();
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


void SVPropResultsWidget::on_pushButtonSetGlobalMinMax_clicked() {
	setCurrentMinMaxValues();
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


void SVPropResultsWidget::on_pushButtonSetLocalMinMax_clicked() {
	setCurrentMinMaxValues(true);
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


void SVPropResultsWidget::on_tableWidgetAvailableResults_cellDoubleClicked(int /*row*/, int /*column*/) {
	readCurrentResult(false);
}


void SVPropResultsWidget::on_toolButtonSetDefaultDirectory_clicked() {
	QFileInfo finfo(SVProjectHandler::instance().projectFile());
	m_resultsDir = QDir(finfo.dir().absoluteFilePath(finfo.completeBaseName()));
	m_ui->resultsDir->setFilename(m_resultsDir.absolutePath());
	readResultsDir();
}


void SVPropResultsWidget::on_toolButtonUpdateAvailableOutputs_clicked() {
	readResultsDir();
}


void SVPropResultsWidget::on_pushButton_clicked() {
	readCurrentResult(true);
}

