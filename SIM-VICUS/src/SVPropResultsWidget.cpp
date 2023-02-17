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



// *** Private Slots ***


void SVPropResultsWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	// Output data and VICUS project on file and current VICUS data model are unrelated.
	// We could check for consistency of vicus, nandrad and output files,
	// i.e. time stamps must be vicus <= nandrad <= outputs.
	// Also, we could set a flag if user has modified anything - however, we never know
	// if those changes affect outputs or not.

	// Hence, in the result view we must always assume that we show mismatching data from
	// outdated files. But, as long as the UI doesn't crash, we can just keep it simple.

	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {
		case SVProjectHandler::AllModified: {
			on_toolButtonSetDefaultDirectory_clicked();
		} break;
//		case SVProjectHandler::NetworkGeometryChanged:
//		case SVProjectHandler::BuildingGeometryChanged: {
//			refreshDirectory();
//		} break;
		default: ; // just to make compiler happy
	}
}


void SVPropResultsWidget::onTimeSliderCutValueChanged(double currentTime) {
	updateColors(currentTime);
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


void SVPropResultsWidget::on_tableWidgetAvailableResults_itemSelectionChanged() {
	QString currentOutputUnit;
	// find out the selected row
	int row = m_ui->tableWidgetAvailableResults->currentRow();
	if (row == -1) {
		// selection cleared, disable false coloring
		m_currentOutputQuantity.clear();
	}
	else {
		// make quantity the current quantity
		m_currentOutputQuantity = m_ui->tableWidgetAvailableResults->item(row, 1)->text();
		currentOutputUnit = m_ui->tableWidgetAvailableResults->item(row, 2)->text();
		// check if the respective file is in cache
		Q_ASSERT(m_outputVariable2FileIndexMap.find(m_currentOutputQuantity) != m_outputVariable2FileIndexMap.end());
		unsigned int outputFileIndex = m_outputVariable2FileIndexMap[m_currentOutputQuantity];
		if (m_outputFiles[outputFileIndex].m_status == ResultDataSet::FS_Unread) {
			m_currentOutputQuantity.clear(); // not cached yet, cannot display
			currentOutputUnit.clear();
		}
		else {
			// set slider
			Q_ASSERT(!m_allResults[m_currentOutputQuantity].empty());
			// get time points from first data set for this quantity
			IBK::UnitVector timePointVec;
			timePointVec.m_data = m_allResults[m_currentOutputQuantity].begin()->second.m_values.x();
			timePointVec.m_unit = m_allResults[m_currentOutputQuantity].begin()->second.m_xUnit;
			if (timePointVec.m_unit.base_unit() == IBK::Unit("s"))
				timePointVec.convert(IBK::Unit("s"));
			m_ui->widgetTimeSlider->setValues(timePointVec);
			m_ui->widgetTimeSlider->setCurrentValue(timePointVec.m_data.back());
			// determine max/min values
			setCurrentMinMaxValues(false);
		}
	}

	SVViewStateHandler::instance().m_geometryView->colorLegend()->setTitle( QString("%1 [%2]").arg(m_currentOutputQuantity, currentOutputUnit));
	// trigger recoloring (or if no data - make all grey)
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


void SVPropResultsWidget::on_tableWidgetAvailableResults_cellDoubleClicked(int row, int /*column*/) {
	// get selected quantity
	QString requestedQuantity = m_ui->tableWidgetAvailableResults->item(row, 1)->text();
	// find respective filename
	Q_ASSERT(m_outputVariable2FileIndexMap.find(requestedQuantity) != m_outputVariable2FileIndexMap.end());
	unsigned int outputFileIndex = m_outputVariable2FileIndexMap[requestedQuantity];
	// read data file
	readDataFile(m_outputFiles[outputFileIndex].m_filename);
	// and finally trigger recoloring
	on_tableWidgetAvailableResults_itemSelectionChanged();
}


void SVPropResultsWidget::on_toolButtonSetDefaultDirectory_clicked() {
	QFileInfo finfo(SVProjectHandler::instance().projectFile());
	QDir defaultResultsDir = QDir(finfo.dir().absoluteFilePath(finfo.completeBaseName()));
	m_ui->resultsDir->setFilename(defaultResultsDir.absolutePath());
	on_pushButtonRefreshDirectory_clicked(); // transfers new directory to m_resultsDir
}


void SVPropResultsWidget::on_pushButtonRefreshDirectory_clicked() {
	QString resultsDirPath = m_ui->resultsDir->filename();
	if (resultsDirPath.endsWith("/results"))
		resultsDirPath = resultsDirPath.left(resultsDirPath.count()-8);
	QDir resultsDir(resultsDirPath);
	// if user has selected a different directory, clear our cached results
	if (resultsDir != m_resultsDir) {
		// clear cached results
		m_outputFiles.clear();
		m_outputVariable2FileIndexMap.clear();
		m_objectName2Id.clear();
		m_allResults.clear();
		m_currentOutputQuantity.clear(); // = nothing selected, yet
		m_resultsDir = resultsDir;

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


// *** Private Functions ***

bool extractID(const QString & objectName, unsigned int & id) {
	// extract the id
	int start = objectName.indexOf("ID=")+3;
	int end = objectName.lastIndexOf(")");
	if (start == -1 || end == -1 || end < start)
		return false; // invalid format of caption; maybe missing name substitution in NANDRAD project generator?
	bool ok;
	unsigned int idval = objectName.midRef(start, end-start).toUInt(&ok);
	if (ok)
		id = idval;
	return ok;
}


void SVPropResultsWidget::readResultsDir() {

	// key = quantity name without ID suffix, for example: "IdealHeatingLoad-average" or "AirTemperature"
	// value = vector of object IDs that we have outputs for
	std::map<QString, std::vector<unsigned int> >	availableOutputs;
	// key = quantity name (same as above)
	// value = unit string
	std::map<QString, QString>						availableOutputUnits;

	// NOTE: we do not clear cached data here!

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
			if (fields.size()!=2)
				continue; // invalid format of line/empty line?
			// for network elements, currently we only recognize pipe elements
			if (fields[0].contains("NetworkElement") && !(fields[1].contains("SupplyPipe") || fields[1].contains("ReturnPipe")))
				continue;
			// extract the id
			unsigned int id;
			if (!extractID(fields[1], id))
				continue;
			// remember association of mapped quantity (as it appears in the caption of a tsv file) and the unique object ID
			m_objectName2Id[fields[1]] = id;
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
		// Note: we always skip the first time column
		for (unsigned int i=1; i<captions.size(); ++i) {
			QString caption = QString::fromStdString(captions[i]);
			QString unit = QString::fromStdString(units[i]);
			QString outputName;
			for (auto it=m_objectName2Id.begin(); it!=m_objectName2Id.end(); ++it) {
				if (caption.contains(it->first)){
					outputName = caption.remove(it->first + ".");
					availableOutputs[outputName].push_back(it->second); // store output name to ids
					availableOutputUnits[outputName] = unit;
					// remember output quantity to file association
					m_outputVariable2FileIndexMap[outputName] = (unsigned int)outputFileIdx;
				}
			}
		}
	}

	// finally update the state of files that have been lost, i.e. are no longer in the directory
	for (ResultDataSet & rds : m_outputFiles) {
		if (!filesFound.contains(rds.m_filename))
			rds.m_status = ResultDataSet::FS_Missing;
	}

	// if we have found an output of a minimum size: enable Ui
	bool validOutputFound = !availableOutputs.empty();
	m_ui->groupBoxAvailableOutputs->setEnabled(validOutputFound);
	m_ui->groupBoxColormap->setEnabled(validOutputFound);
	m_ui->groupBoxNetworkOptions->setEnabled(validOutputFound && !project().m_geometricNetworks.empty());
	m_ui->groupBoxTime->setEnabled(validOutputFound);

	if (!validOutputFound)
		return;

	// update table widget - we only create the table widget items and fill in the quantities - formatting and icons
	// is done in updateTableWidget()
	m_ui->tableWidgetAvailableResults->blockSignals(true);
	m_ui->tableWidgetAvailableResults->clearContents();
	m_ui->tableWidgetAvailableResults->setRowCount(availableOutputs.size());
	int row=0;
	int selectedRow = -1;
	for (auto it=availableOutputUnits.begin(); it!=availableOutputUnits.end(); ++it, ++row) {
		QString outputVariable = it->first;
		if (m_currentOutputQuantity == outputVariable)
			selectedRow = row;

		QTableWidgetItem * item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetAvailableResults->setItem(row, 0, item);

		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setText(outputVariable);
		m_ui->tableWidgetAvailableResults->setItem(row, 1, item);

		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setText(it->second);
		m_ui->tableWidgetAvailableResults->setItem(row, 2, item);

		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetAvailableResults->setItem(row, 3, item);
	}
	// apply font styles, colors and icons
	updateTableWidgetFormatting();
	// now that the table was populated, reselect the row of the current quantity
	if (selectedRow != -1)
		m_ui->tableWidgetAvailableResults->selectRow(selectedRow); // signals blocked, no side effect here

	m_ui->tableWidgetAvailableResults->blockSignals(false);

	// Now trigger the recoloring based on the selected quantity.
	// If this is the first time this function was called than there won't be a selection in the table and
	// all geometry will be greyed out.
	on_tableWidgetAvailableResults_itemSelectionChanged();
}


void SVPropResultsWidget::updateTableWidgetFormatting() {
	int selectedRow = -1;
	for (int row=0; row<m_ui->tableWidgetAvailableResults->rowCount(); ++row) {
		QString outputVariable = m_ui->tableWidgetAvailableResults->item(row, 1)->text();
		if (m_currentOutputQuantity == outputVariable) {
			selectedRow = row;
		}
		// check state of output file
		unsigned int outputFileIndex = m_outputVariable2FileIndexMap[outputVariable];

		const ResultDataSet & rds = m_outputFiles[(int)outputFileIndex];
		QFont f;
		QColor textColor;
		if (SVSettings::instance().m_theme == SVSettings::TT_Dark)
			textColor = Qt::white;
		else
			textColor = Qt::black;
		QString statusLabel;
		QIcon availableIcon;
		switch (rds.m_status) {
			case SVPropResultsWidget::ResultDataSet::FS_Unread:
				statusLabel = tr("unread");
				textColor = Qt::gray;
			break;
			case SVPropResultsWidget::ResultDataSet::FS_Current:
				availableIcon = QIcon(":/gfx/actions/16x16/ok.png");
				statusLabel = tr("current");
			break;
			case SVPropResultsWidget::ResultDataSet::FS_Outdated:
				f.setItalic(true);
				availableIcon = QIcon(":/gfx/actions/16x16/ok.png");
				textColor = QColor(196,128,0);
				statusLabel = tr("outdated");
			break;
			case SVPropResultsWidget::ResultDataSet::FS_Missing:
				f.setItalic(true);
				textColor = QColor(192,32,32);
				statusLabel = tr("missing");
			break;
		}

		m_ui->tableWidgetAvailableResults->item(row, 0)->setIcon(availableIcon);

		m_ui->tableWidgetAvailableResults->item(row, 1)->setFont(f);
		m_ui->tableWidgetAvailableResults->item(row, 1)->setForeground(textColor);

		m_ui->tableWidgetAvailableResults->item(row, 2)->setFont(f);
		m_ui->tableWidgetAvailableResults->item(row, 2)->setForeground(textColor);

		m_ui->tableWidgetAvailableResults->item(row, 3)->setFont(f);
		m_ui->tableWidgetAvailableResults->item(row, 3)->setForeground(textColor);
		m_ui->tableWidgetAvailableResults->item(row, 3)->setText(statusLabel);
	}
}


void SVPropResultsWidget::readDataFile(const QString & filename) {
	FUNCID("SVPropResultsWidget::readDataFile");

	QString fullFilePath = m_resultsDir.absoluteFilePath("results/" + filename);
	QProgressDialog progDiag(tr("Reading file '%1'").arg(filename), tr("Cancel"), 0, 100, this);
	progDiag.setWindowTitle(tr("Reading results file"));
	progDiag.setValue(0);
	progDiag.setAutoClose(true);
	progDiag.setMinimumDuration(0);
	qApp->processEvents();


	IBK::CSVReader reader;
	IBK::Unit timeUnit;
	try {
		// read entire file
		reader.read(IBK::Path(fullFilePath.toStdString()), false, true);
		if (reader.m_nColumns < 2 || reader.m_nRows < 5)
			throw IBK::Exception("Missing data in file.", FUNC_ID);

		// check time unit
		timeUnit = IBK::Unit(reader.m_units[0]); // may throw an exception
		if (timeUnit.base_unit() != IBK::Unit("s"))
			throw IBK::Exception("Invalid time unit.", FUNC_ID);
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		progDiag.close();
		QMessageBox::critical(this, QString(), tr("Invalid/missing content in result file '%1'.").arg(filename));
		for (ResultDataSet & rds : m_outputFiles) {
			if (rds.m_filename == filename) {
				rds.m_status = ResultDataSet::FS_Missing;
				refreshDirectory();
				break;
			}
		}
		return;
	}


	// the rest of the parsing may not throw, except for value unit conversion handled invidiually

	progDiag.setLabelText(tr("Processing results"));
	progDiag.setMaximum((int)reader.m_captions.size()-1);

	// we convert time to seconds so its in accordance with time slider
	std::vector<double> time = reader.colData(0);

	IBK::UnitVector timeSeconds(time.begin(), time.end(), timeUnit);
	timeSeconds.convert(IBK::Unit("s"));

	// process all columns of the tsv file, except time column
	for (unsigned int i=1; i<reader.m_captions.size(); ++i) {

		progDiag.setValue((int)i);
//		qApp->processEvents();

		// check if this caption is among our recognized captions
		QString qCaption = QString::fromStdString(reader.m_captions[i]);
		// extract output name - everything past the last .
		int dotpos = qCaption.lastIndexOf('.');
		if (dotpos == -1)
			continue;
		QString objectName = qCaption.left(dotpos);
		std::map<QString, unsigned int>::const_iterator it = m_objectName2Id.find(objectName);
		if (it == m_objectName2Id.end())
			continue; // not recognized, skip
		// extract the id
		unsigned int id;
		if (!extractID(qCaption, id))
			continue;
		// remove the object reference, for example
		// "BuildingName.E0.WE0.0_Bath(ID=100)" from
		// caption "BuildingName.E0.WE0.0_Bath(ID=100).AirTemperature"
		QString outputName = qCaption.mid(dotpos+1); // -> AirTemperature
		// and store in map with all outputs
		m_allResults[outputName][id] = NANDRAD::LinearSplineParameter(reader.m_captions[i], NANDRAD::LinearSplineParameter::I_LINEAR,
																			   timeSeconds.m_data, reader.colData(i), timeUnit, IBK::Unit("s"));
	} // for captions in file

	progDiag.setValue((int)reader.m_captions.size()-1); // fill and closes the dialog

	// update status in filelist
	for (ResultDataSet & rds : m_outputFiles) {
		if (rds.m_filename == filename) {
			rds.m_status = ResultDataSet::FS_Current;
			rds.m_timeStampLastUpdated = QFileInfo(fullFilePath).lastModified();
			break;
		}
	}

	// now cache was updated, update table widget status
	updateTableWidgetFormatting(); // this does not cause any recoloring or selection change, just updates fonts and icons
}


void SVPropResultsWidget::setCurrentMinMaxValues(bool localMinMax) {

	// determine overall max/min
	m_currentMin = std::numeric_limits<double>::max();
	m_currentMax = std::numeric_limits<double>::lowest();
	double currentTime = m_ui->widgetTimeSlider->currentCutValue();
	double max, min;

	for (auto it=m_allResults[m_currentOutputQuantity].begin(); it!=m_allResults[m_currentOutputQuantity].end(); ++it) {
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

	bool haveData = !m_currentOutputQuantity.isEmpty();

	const QColor GREY(64,64,64);

	QColor col;
	const VICUS::Project &p = project();
	for (const VICUS::Network &net: p.m_geometricNetworks) {
		for (const VICUS::NetworkNode &no: net.m_nodes)
			no.m_color = GREY;
		for (const VICUS::NetworkEdge &e: net.m_edges) {
			e.m_color = GREY; // initialize with grey
			if (haveData && m_allResults[m_currentOutputQuantity].find(e.m_id) != m_allResults[m_currentOutputQuantity].end()) {
				double yint = m_allResults[m_currentOutputQuantity][e.m_id].m_values.value(currentTime);
				interpolateColor(yint, col);
				e.m_color = col;
			}
		}
	}

	for (const VICUS::Building & b : p.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				// initialize with grey
				for (const VICUS::Surface & s : r.m_surfaces) {
					s.m_color = GREY;
					for (const VICUS::SubSurface &ss: s.subSurfaces())
						ss.m_color = GREY;
				}
				// a room related property (e.g. AirTemperature)
				if (haveData && m_allResults[m_currentOutputQuantity].find(r.m_id) != m_allResults[m_currentOutputQuantity].end() ) {
					double yint = m_allResults[m_currentOutputQuantity][r.m_id].m_values.value(currentTime);
					interpolateColor(yint, col);
					for (const VICUS::Surface & s : r.m_surfaces) {
						s.m_color = col;
						for (const VICUS::SubSurface &ss: s.subSurfaces())
							ss.m_color = col;
					}
				}
				// a surface related property (e.g. SurfaceTemperature)
				else if (haveData) {
					for (const VICUS::Surface & s : r.m_surfaces) {
						if (m_allResults[m_currentOutputQuantity].find(s.m_id) != m_allResults[m_currentOutputQuantity].end()) {
							double yint = m_allResults[m_currentOutputQuantity][s.m_id].m_values.value(currentTime);
							interpolateColor(yint, col);
							s.m_color = col;
						}
						for (const VICUS::SubSurface &ss: s.subSurfaces()) {
							if (m_allResults[m_currentOutputQuantity].find(ss.m_id) != m_allResults[m_currentOutputQuantity].end()) {
								double yint = m_allResults[m_currentOutputQuantity][ss.m_id].m_values.value(currentTime);
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
	SVViewStateHandler::instance().setViewState(vs); // this will inform all widgets that monitor the coloring mode and redraw the scene
}



