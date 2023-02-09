#include "SVPropResultsWidget.h"
#include "ui_SVPropResultsWidget.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVStyle.h"
#include "SVViewStateHandler.h"

#include <QFileInfo>
#include <QTextStream>

#include <IBK_CSVReader.h>


SVPropResultsWidget::SVPropResultsWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropResultsWidget)
{
	m_ui->setupUi(this);

	m_ui->resultsDir->setup("", false, true, "", SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->tableWidgetAvailableResults->setColumnCount(3);
	m_ui->tableWidgetAvailableResults->setHorizontalHeaderLabels(QStringList() << tr("") << tr("name") << tr("unit") );
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetAvailableResults);
	m_ui->tableWidgetAvailableResults->setSortingEnabled(true);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->resizeSection(0,10);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->resizeSection(1,120);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->resizeSection(2,30);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetAvailableResults->setSelectionMode(QAbstractItemView::SingleSelection);
	m_ui->tableWidgetAvailableResults->setSelectionBehavior(QAbstractItemView::SelectRows);

	m_ui->comboBoxPipeType->clear();
	m_ui->comboBoxPipeType->addItem(tr("SupplyPipe"), "SupplyPipe");
	m_ui->comboBoxPipeType->addItem(tr("ReturnPipe"), "ReturnPipe");

	SVViewStateHandler::instance().m_resultColors = &this->m_resultColors;

	m_ui->doubleSpinBoxMaxValue->setMaximum(std::numeric_limits<double>::max());
	m_ui->doubleSpinBoxMaxValue->setMinimum(std::numeric_limits<double>::lowest());
	m_ui->doubleSpinBoxMinValue->setMaximum(std::numeric_limits<double>::max());
	m_ui->doubleSpinBoxMinValue->setMinimum(std::numeric_limits<double>::lowest());

	m_ui->pushButtonMaxColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->pushButtonMaxColor->setColor("#780000");
	m_ui->pushButtonMinColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->pushButtonMinColor->setColor("#669bbc");

	connect(m_ui->widgetTimeSlider, &SVTimeSliderWidget::cutValueChanged,
			this, &SVPropResultsWidget::onTimeSliderCutValueChanged);
}


SVPropResultsWidget::~SVPropResultsWidget() {
	delete m_ui;
}


void SVPropResultsWidget::on_pushButtonSetDefaultDirectory_clicked() {
	QFileInfo finfo(SVProjectHandler::instance().projectFile());
	m_resultsDir = QDir(finfo.dir().absoluteFilePath(finfo.completeBaseName()));
	m_ui->resultsDir->setFilename(m_resultsDir.absolutePath());

	updateTableWidgetAvailableOutputs();
}


void SVPropResultsWidget::updateTableWidgetAvailableOutputs() {
	FUNCID(SVPropResultsWidget::updateTableWidgetAvailableOutputs());
	m_availableOutputs.clear();

	if (m_resultsDir.absolutePath().endsWith("/results"))
		m_resultsDir.cdUp();

	// read substitution map
	m_substitutionName2Id.clear();
	QFile subsFile(m_resultsDir.absolutePath() + "/var/objectref_substitutions.txt");
	if (subsFile.open(QFile::ReadOnly)){
		QTextStream in(&subsFile);
		QString line;
		while (in.readLineInto(&line)) {
			QStringList fields = line.split("\t");
			if (fields.size()==2) {
				// extract the id
				int start = fields[1].indexOf("ID=")+3;
				int end = fields[1].lastIndexOf(")");
				unsigned int id = fields[1].mid(start, end-start).toUInt();
				m_substitutionName2Id[fields[1]] = id;
			}
		}
	}
	else {
		throw IBK::Exception(IBK::FormatString("Substitution file '%1' not found!").arg(subsFile.fileName().toStdString()), FUNC_ID);
	}

	m_resultsDir = QDir(m_resultsDir.absolutePath() + "/results");
	QStringList tsvFiles = m_resultsDir.entryList(QStringList() << "*.tsv", QDir::Files);
	for (const QString &fileName: tsvFiles) {
		QFile tsvFile(m_resultsDir.absoluteFilePath(fileName));
		if (!tsvFile.open(QFile::ReadOnly))
			continue;
		QTextStream in(&tsvFile);
		QString line;
		in.readLineInto(&line);

		// we go through the tsv header ...
		QStringList header = line.split("\t");
		for (const QString &field: header) {
			QStringList nameUnit = field.split(" ");
			QString outputProperty;
			if (nameUnit.size()==2) { // this is the case for regular tsv files
				// we look
				for (auto it=m_substitutionName2Id.begin(); it!=m_substitutionName2Id.end(); ++it) {
					if (nameUnit[0].contains(it->first)){
						outputProperty = nameUnit[0].remove(it->first + ".");
						m_availableOutputs[outputProperty].push_back(it->second);
						m_availableOutputUnits[outputProperty] = nameUnit[1];
						m_outputFile[outputProperty] = m_resultsDir.absoluteFilePath(fileName);
					}
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


void SVPropResultsWidget::on_pushButtonUpdateAvailableOutputs_clicked() {
	updateTableWidgetAvailableOutputs();
}


void SVPropResultsWidget::on_pushButtonReadResults_clicked() {

	m_resultColors.clear();

	QString pipeType = m_ui->comboBoxPipeType->currentData().toString();
	QTableWidgetItem *item = m_ui->tableWidgetAvailableResults->item(m_ui->tableWidgetAvailableResults->currentRow(), 1);
	QString outputName = item->text();

	// read entire file
	IBK::CSVReader reader;
	reader.read(IBK::Path(m_outputFile[outputName].toStdString()), false, true);
	if (reader.m_nColumns < 2 || reader.m_nRows < 5)
		return;

	std::vector<double> time = reader.colData(0);
	IBK::Unit timeUnit = IBK::Unit(reader.m_units[0]);

	if (timeUnit.base_unit() != IBK::Unit("s"))
		return;

	for (unsigned int i=0; i<reader.m_captions.size(); ++i) {
		const std::string &caption = reader.m_captions[i];
		for (auto it=m_substitutionName2Id.begin(); it!=m_substitutionName2Id.end(); ++it) {

			std::string objectName = it->first.toStdString();
			// find columns
			if (caption.find(objectName) != std::string::npos && caption.find(outputName.toStdString()) != std::string::npos){
				QString header = QString::fromStdString(caption);
				int start = header.indexOf("ID=")+3;
				int end = header.lastIndexOf(")");
				unsigned int id = header.mid(start, end-start).toUInt();
				m_currentResults[id] = NANDRAD::LinearSplineParameter(caption, NANDRAD::LinearSplineParameter::I_LINEAR,
																	  time, reader.colData(i), timeUnit, IBK::Unit(reader.m_units[i]));
			}
		}
	}

	qDebug() << "number current results: " << m_currentResults.size();
	qDebug() << "current results 0 size : " << m_currentResults.begin()->second.m_values.size();

	// set slider
	m_ui->widgetTimeSlider->setValues(IBK::UnitVector(time.begin(), time.end(), timeUnit));

	// determine max/min values
	setCurrentMinMaxValues();

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

	for (auto it=m_currentResults.begin(); it!=m_currentResults.end(); ++it) {
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


void SVPropResultsWidget::calculateColor(const double & yint, QColor & col) {
	double ys = (yint - m_currentMin) / (m_currentMax - m_currentMin);
	if (ys>1)
		ys=1;
	if (ys<0)
		ys=0;
	double hMax, sMax, vMax, hMin, sMin, vMin;
	m_maxColor.getHsvF(&hMax, &sMax, &vMax);
	m_minColor.getHsvF(&hMin, &sMin, &vMin);
	double hNew = hMin + ys * (hMax - hMin);
//	double sNew = sMin + ys * (sMax - sMin);
//	double vNew = vMin + ys * (vMax - vMin);
	col.setHsvF(hNew, sMax, vMax);
}


void SVPropResultsWidget::updateColors(const double &currentTime) {

	QColor col;
	const VICUS::Project &p = project();
	for (const VICUS::Network &net: p.m_geometricNetworks) {
		for (const VICUS::NetworkEdge &e: net.m_edges) {
			if (m_currentResults.find(e.m_id) != m_currentResults.end()) {
				double yint = m_currentResults[e.m_id].m_values.value(currentTime);
				calculateColor(yint, col);
				std::swap(e.m_color, col);
//				e.m_color = col;
//				m_resultColors[e.m_id] = col;
			}
		}
	}

	for (const VICUS::Building & b : p.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {

				// a room related property (e.g. AirTemperature)
				if ( m_currentResults.find(r.m_id) != m_currentResults.end() ) {
					double yint = m_currentResults[r.m_id].m_values.value(currentTime);
					calculateColor(yint, col);
					for (const VICUS::Surface & s : r.m_surfaces)
						std::swap(s.m_color, col);
				}
				// a wall related property (e.g. SurfaceTemperature)
				else {
					for (const VICUS::Surface & s : r.m_surfaces) {
						if (m_currentResults.find(s.m_id) != m_currentResults.end()) {
							double yint = m_currentResults[s.m_id].m_values.value(currentTime);
							calculateColor(yint, col);
							std::swap(s.m_color, col);
						}
					}
				}
			}
		}
	}

	SVViewStateHandler::instance().updateResultColors();
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

