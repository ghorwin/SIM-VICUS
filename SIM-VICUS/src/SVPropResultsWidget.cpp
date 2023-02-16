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

	m_ui->tableWidgetAvailableResults->setColumnCount(4);
	m_ui->tableWidgetAvailableResults->setHorizontalHeaderLabels(QStringList() << tr("") << tr("Name") << tr("Unit") << tr("Status"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetAvailableResults);
	m_ui->tableWidgetAvailableResults->setSortingEnabled(false);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->resizeSection(0,10);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->resizeSection(1,300);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->resizeSection(2,30);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->resizeSection(3,70);
	m_ui->tableWidgetAvailableResults->horizontalHeader()->setStretchLastSection(false);
	m_ui->tableWidgetAvailableResults->setSelectionMode(QAbstractItemView::SingleSelection);
	m_ui->tableWidgetAvailableResults->setSelectionBehavior(QAbstractItemView::SelectRows);

	m_ui->comboBoxPipeType->clear();
	m_ui->comboBoxPipeType->addItem(tr("SupplyPipe"), "SupplyPipe");
	m_ui->comboBoxPipeType->addItem(tr("ReturnPipe"), "ReturnPipe");

	m_ui->lineEditMinValue->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Minimum value for coloring"), false, false);
	m_ui->lineEditMaxValue->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Maximum value for coloring"), false, false);

	m_ui->pushButtonMaxColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);
	if (SVSettings::instance().m_theme == SVSettings::TT_Dark)
		m_ui->pushButtonMaxColor->setColor("#ff1d1b"); // on dark mode use a bit more vibrant colors
	else
		m_ui->pushButtonMaxColor->setColor("#8f1d1b");
	m_maxColor = m_ui->pushButtonMaxColor->color();
	m_ui->pushButtonMinColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->pushButtonMinColor->setColor("#669bbc");
	m_minColor = m_ui->pushButtonMinColor->color();

	connect(m_ui->widgetTimeSlider, &SVTimeSliderWidget::cutValueChanged,
			this, &SVPropResultsWidget::onTimeSliderCutValueChanged);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropResultsWidget::onModified);

	m_ui->resultsDir->setFilename("");
	m_ui->tableWidgetAvailableResults->setRowCount(0);
	m_ui->lineEditMaxValue->setValue(1);
	m_ui->lineEditMinValue->setValue(0);
	m_ui->widgetTimeSlider->clear();

	// set pointer for color legend
	SVViewStateHandler::instance().m_geometryView->colorLegend()->initialize(&m_currentMin, &m_currentMax, &m_minColor, &m_maxColor);
}


SVPropResultsWidget::~SVPropResultsWidget() {
	delete m_ui;
}


void SVPropResultsWidget::refreshDirectory() {
	Q_ASSERT(SVProjectHandler::instance().isValid());
	Q_ASSERT(SVViewStateHandler::instance().viewState().m_objectColorMode == SVViewState::OCM_ResultColorView);

	if (m_ui->resultsDir->filename().isEmpty())
		on_toolButtonSetDefaultDirectory_clicked();
	else
		on_pushButtonRefreshDirectory_clicked();
}


void SVPropResultsWidget::onModified(int /*modificationType*/, ModificationInfo * /*data*/) {
	// Output data and VICUS project on file and current VICUS data model are unrelated.
	// We could check for consistency of vicus, nandrad and output files,
	// i.e. time stamps must be vicus <= nandrad <= outputs.
	// Also, we could set a flag if user has modified anything - however, we never know
	// if those changes affect outputs or not.

	// Hence, in the result view we must always assume that we show mismatching data from
	// outdated files. But, as long as the UI doesn't crash, we can just keep it simple.

//	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
//	switch (modType) {
//		case SVProjectHandler::AllModified:
//		case SVProjectHandler::NetworkGeometryChanged:
//		case SVProjectHandler::BuildingGeometryChanged: {
//			clearUi();
//		} break;
//		default: ; // just to make compiler happy
//	}
}


void SVPropResultsWidget::readResultsDir() {

	std::map<QString, std::vector<unsigned int> >	availableOutputs;
	std::map<QString, QString>						availableOutputUnits;
	m_outputFiles.clear();
	m_allResults.clear();

	// make sure we have a proper path
	Q_ASSERT(!m_resultsDir.absolutePath().endsWith("/results"));

	// read substitution file to get object name and VICUS ID
	m_objectName2Id.clear();
	QFile subsFile(m_resultsDir.absolutePath() + "/var/objectref_substitutions.txt");
	// File contains tab-separated ID names - first is native object name of SIM-VICUS object,
	// second is name appearing in tsv files:
	//			ConstructionInstance(id=9)	WE0.0_Floor_1 (ID=1)
	//			Zone(id=100)	BuildingName.E0.WE0.0_Bath(ID=100)
	if (subsFile.open(QFile::ReadOnly)){
		QTextStream in(&subsFile);
		in.setCodec("UTF-8");
		QString line;
		while (in.readLineInto(&line)) {
			QStringList fields = line.split("\t");
			if (fields.size()==2) {
				// for network elements, we read only pipe elements currently
				if (fields[0].contains("NetworkElement") && !(fields[1].contains("SupplyPipe") || fields[1].contains("ReturnPipe")))
					continue;
				// extract the id
				// TODO : protected against invalid naming and invalid IDs
				int start = fields[1].indexOf("ID=")+3;
				int end = fields[1].lastIndexOf(")");
				unsigned int id = fields[1].mid(start, end-start).toUInt(); // TODO : invalid ID numbers?
				m_objectName2Id[fields[1]] = id;
			}
		}
	}

	// now read result tsv files
	QDir resultFilesDir = QDir(m_resultsDir.absolutePath() + "/results");
	QStringList tsvFiles = resultFilesDir.entryList(QStringList() << "*.tsv", QDir::Files);
	QSet<QString> filesFound;
	for (const QString &fileName: qAsConst(tsvFiles)) {
		// read tsv header
		IBK::CSVReader reader;
		QString absoluteTsvFilePath = resultFilesDir.absoluteFilePath(fileName);
		IBK::Path tsvFilePath(absoluteTsvFilePath.toStdString());
		try {
			reader.read(tsvFilePath, true, true);
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			IBK::IBK_Message(IBK::FormatString("Error parsing file '%1").arg(tsvFilePath), IBK::MSG_ERROR);
			continue; // skip this file
		}

		std::vector<std::string> captions = reader.m_captions;
		std::vector<std::string> units = reader.m_units;
		if (captions.size() != units.size() || captions.empty())
			continue;

		// remember this file so that we can find missing files later
		filesFound.insert(fileName);
		// update file status
		int outputFileIdx=0;
		for (; outputFileIdx<m_outputFiles.count(); ++outputFileIdx) {
			if (m_outputFiles[outputFileIdx].m_filename == fileName) {
				// previously read?
				if (m_outputFiles[outputFileIdx].m_status != ResultDataSet::FS_Unread) {
					// check time stamp on file
					if (QFileInfo(absoluteTsvFilePath).lastModified() > m_outputFiles[outputFileIdx].m_timeStampLastUpdated)
						m_outputFiles[outputFileIdx].m_status = ResultDataSet::FS_Outdated;
					else
						m_outputFiles[outputFileIdx].m_status = ResultDataSet::FS_Current;
				}
				break;
			}
		}
		// check if this is a new file
		if (outputFileIdx == m_outputFiles.count()) {
			ResultDataSet ds;
			ds.m_filename = fileName;
			m_outputFiles.append(ds);
		}

		// we go through the tsv header and look for our object names
		for (unsigned int i=0; i<captions.size(); ++i) {
			QString caption = QString::fromStdString(captions[i]);
			QString unit = QString::fromStdString(units[i]);
			QString outputName;
			for (auto it=m_objectName2Id.begin(); it!=m_objectName2Id.end(); ++it) {
				if (caption.contains(it->first)){
					outputName = caption.remove(it->first + ".");
					availableOutputs[outputName].push_back(it->second); // store output name to ids
					availableOutputUnits[outputName] = unit;
					// remember output quantity to file association
					m_outputVariables[outputName] = outputFileIdx;
				}
			}
		}
	}

	// if we have found an output of a minimum size: enable Ui
	bool validOutputFound = !availableOutputs.empty() && availableOutputs.begin()->second.size() > 0;
	m_ui->groupBoxAvailableOutputs->setEnabled(validOutputFound);
	m_ui->groupBoxColormap->setEnabled(validOutputFound);
	m_ui->groupBoxNetworkOptions->setEnabled(validOutputFound && !project().m_geometricNetworks.empty());
	m_ui->groupBoxTime->setEnabled(validOutputFound);

	if (!validOutputFound)
		return;

	// update table widget
	m_ui->tableWidgetAvailableResults->clearContents();
	m_ui->tableWidgetAvailableResults->setRowCount(availableOutputs.size());
	int row=0;
	for (auto it=availableOutputUnits.begin(); it!=availableOutputUnits.end(); ++it) {

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

	QProgressDialog progDiag(tr("Reading file"), tr("Cancel"), 0, 100, this);
	progDiag.setWindowTitle(tr("Read results"));
	progDiag.setValue(0);
	progDiag.setAutoClose(true);
	progDiag.setMinimumDuration(0);
	qApp->processEvents();

	// now set current output
	QTableWidgetItem *item = m_ui->tableWidgetAvailableResults->item(m_ui->tableWidgetAvailableResults->currentRow(), 1);
	m_currentOutput = item->text();

	QString filter;
	if (!project().m_geometricNetworks.empty())
		filter = m_ui->comboBoxPipeType->currentData().toString();

	if (filter != m_currentFilter)
		forceToRead = true;
	m_currentFilter = filter;
#if 0
	// we only read results which have not yet been read and stored
	if (forceToRead || m_allResults.find(m_currentOutput) == m_allResults.end()) {

		progDiag.setValue(1);
		qApp->processEvents();

		// read entire file
		IBK::CSVReader reader;
		reader.read(IBK::Path(m_outputFiles[m_currentOutput].toStdString()), false, true);
		if (reader.m_nColumns < 2 || reader.m_nRows < 5)
			return;

		progDiag.setLabelText(tr("Processing results"));
		progDiag.setMaximum((int)reader.m_captions.size()-1);

		// we convert time to seconds so its in accordance with time slider
		std::vector<double> time = reader.colData(0);
		IBK::Unit timeUnit = IBK::Unit(reader.m_units[0]);
		if (timeUnit.base_unit() != IBK::Unit("s"))
			return;
		IBK::UnitVector timeSeconds(time.begin(), time.end(), timeUnit);
		timeSeconds.convert(IBK::Unit("s"));

		for (unsigned int i=0; i<reader.m_captions.size(); ++i) {

			progDiag.setValue((int)i);
			qApp->processEvents();

			// go through all objects
			for (auto it=m_objectName2Id.begin(); it!=m_objectName2Id.end(); ++it) {
				std::string objectName = it->first.toStdString();
				// look if object name is in this caption
				if (reader.m_captions[i].find(objectName) != std::string::npos){
					// additional filter
					if (filter.isEmpty() || reader.m_captions[i].find(filter.toStdString()) != std::string::npos) {
						qDebug() << "found" << QString::fromStdString( reader.m_captions[i] );
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

	// set current row bold
	item = m_ui->tableWidgetAvailableResults->item(m_ui->tableWidgetAvailableResults->currentRow(), 1);
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

	// set icon
	for (int row=0; row<m_ui->tableWidgetAvailableResults->rowCount(); ++row) {
		const QString &property = m_ui->tableWidgetAvailableResults->item(row, 1)->text();
		QTableWidgetItem *item = new QTableWidgetItem();
		if (m_allResults.find(property) != m_allResults.end())
			item->setIcon(QIcon(":/gfx/actions/16x16/ok.png"));
		m_ui->tableWidgetAvailableResults->setItem(row, 0, item);
	}

	// determine max/min values
	setCurrentMinMaxValues(false);

	// set legend title
	item = m_ui->tableWidgetAvailableResults->item(m_ui->tableWidgetAvailableResults->currentRow(), 2);
	Q_ASSERT(item!=nullptr);
	QString currentUnit = item->text();
	SVViewStateHandler::instance().m_geometryView->colorLegend()->setTitle(QString("%1 [%2]").arg(m_currentOutput).arg(currentUnit));
#endif
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
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

	m_ui->lineEditMaxValue->setValue(m_currentMax);
	m_ui->lineEditMinValue->setValue(m_currentMin);
}


void SVPropResultsWidget::interpolateColor(const double & yint, QColor & col) {
	double ys = (yint - m_currentMin) / (m_currentMax - m_currentMin);
	if (ys>1)
		ys=1;
	else if (ys<0)
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
					for (const VICUS::Surface & s : r.m_surfaces) {
						s.m_color = col;
						for (const VICUS::SubSurface &ss: s.subSurfaces())
							ss.m_color = col;
					}
				}
				// a surface related property (e.g. SurfaceTemperature)
				else {
					for (const VICUS::Surface & s : r.m_surfaces) {
						if (m_allResults[m_currentOutput].find(s.m_id) != m_allResults[m_currentOutput].end()) {
							double yint = m_allResults[m_currentOutput][s.m_id].m_values.value(currentTime);
							interpolateColor(yint, col);
							s.m_color = col;
						}
						for (const VICUS::SubSurface &ss: s.subSurfaces()) {
							if (m_allResults[m_currentOutput].find(ss.m_id) != m_allResults[m_currentOutput].end()) {
								double yint = m_allResults[m_currentOutput][ss.m_id].m_values.value(currentTime);
								interpolateColor(yint, col);
								ss.m_color = col;
							}
						}
					}
				}
			}
		}
	}

	// update color legend
	SVViewStateHandler::instance().m_geometryView->colorLegend()->update();

	// signal main scene to regenerate color buffers and render view
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_objectColorMode = SVViewState::OCM_ResultColorView;
	SVViewStateHandler::instance().setViewState(vs); // this will inform all widgets that monitor the coloring mode
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
	on_pushButtonRefreshDirectory_clicked();
}


void SVPropResultsWidget::on_pushButtonRefreshDirectory_clicked() {
	QString resultsDirPath = QFileInfo(m_ui->resultsDir->filename()).absolutePath();
	if (resultsDirPath.endsWith("/results"))
		resultsDirPath = resultsDirPath.left(resultsDirPath.count()-8);
	QDir resultsDir(resultsDirPath);
	// if user has selected a different directory, clear our cached results
	if (resultsDir != m_resultsDir) {
		// clear cached results
		m_outputFiles.clear();
		m_outputVariables.clear();
		m_objectName2Id.clear();
		m_allResults.clear();
	}
	readResultsDir();
}


void SVPropResultsWidget::on_lineEditMaxValue_editingFinishedSuccessfully() {
	double val = m_ui->lineEditMaxValue->value();
	if (val*0.95 < m_currentMin) {
		m_ui->lineEditMaxValue->setValue(m_currentMax);
		return;
	}
	m_currentMax = val;
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


void SVPropResultsWidget::on_lineEditMinValue_editingFinishedSuccessfully() {
	double val = m_ui->lineEditMinValue->value();
	if (val*1.05 > m_currentMax) {
		m_ui->lineEditMinValue->setValue(m_currentMin);
		return;
	}
	m_currentMin = val;
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


void SVPropResultsWidget::on_comboBoxPipeType_activated(int) {
	// TODO : clarify what should happen here... just update the colors using a different filter? Or refresh entire directory?
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


