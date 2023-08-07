#include "SVPropResultsWidget.h"
#include "ui_SVPropResultsWidget.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVStyle.h"
#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "SVColorLegend.h"
#include "SVUndoTreeNodeState.h"
#include "SVNavigationTreeWidget.h"

#include <QFileInfo>
#include <QTextStream>
#include <QProgressDialog>
#include <QFileDialog>

#include <IBK_CSVReader.h>

#include <QtExt_Directories.h>
#include <QtExt_BrowseFilenameWidget.h>

#include <fstream>

#include <VICUS_BTFReader.h>


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

	m_ui->lineEditMinValue->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Minimum value for coloring"), false, false);
	m_ui->lineEditMaxValue->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("Maximum value for coloring"), false, false);

	connect(m_ui->widgetTimeSlider, &SVTimeSliderWidget::cutValueChanged,
			this, &SVPropResultsWidget::onTimeSliderCutValueChanged);

	connect(m_ui->resultsDir, &QtExt::BrowseFilenameWidget::editingFinished,
			this, &SVPropResultsWidget::on_resultsDir_editingFinished);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropResultsWidget::onModified);

	m_ui->resultsDir->setFilename("");
	m_ui->tableWidgetAvailableResults->setRowCount(0);
	m_ui->lineEditMaxValue->setValue(1);
	m_ui->lineEditMinValue->setValue(0);
	m_ui->widgetTimeSlider->clear();

	on_pushButtonSetDefaultColormap_clicked();

	// set pointer for color legend
	SVViewStateHandler::instance().m_geometryView->colorLegend()->initialize(&m_currentMin, &m_currentMax, &m_colorMap);
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
		}
		[[clang::fallthrough]];
		case SVProjectHandler::NodeStateModified: {
			// update current value line edit
			onSelectionChanged();
			updateLineEditCurrentValue();
		} break;
		default: ; // just to make compiler happy
	}
}


void SVPropResultsWidget::onSelectionChanged() {
	std::set<const VICUS::Object *> objs;
	std::vector<const VICUS::Room *> rooms;
	project().selectObjects(objs, VICUS::Project::SG_All, true, true);
	project().selectedRooms(rooms);

	// only if we find a single object or a room, we store the id
	m_selectedObjectId = VICUS::INVALID_ID;
	if (objs.empty() && rooms.empty())
		m_ui->labelCurrentValue->setText(tr("None selected"));
	else if (rooms.size() == 1)
		m_selectedObjectId = rooms[0]->m_id;
	else if (objs.size()==1)
		m_selectedObjectId = (*objs.begin())->m_id;
	else
		m_ui->labelCurrentValue->setText(tr("Multiple objects selected"));

	// If we have a valid selection: update object label
	const VICUS::Object *obj = project().objectById(m_selectedObjectId);
	if (obj != nullptr) {
		if (obj->m_displayName.isEmpty())
			m_ui->labelCurrentValue->setText(QString("Object with id=%1: ").arg(obj->m_id));
		else
			m_ui->labelCurrentValue->setText(QString("%1: ").arg(obj->m_displayName));
	}
}


void SVPropResultsWidget::onTimeSliderCutValueChanged(double currentTime) {
	updateColors(currentTime);
	updateLineEditCurrentValue();
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
	m_currentOutputUnit.clear();
	// find out the selected row
	int row = m_ui->tableWidgetAvailableResults->currentRow();
	if (row == -1) {
		// selection cleared, disable false coloring
		m_currentOutputQuantity.clear();
	}
	else {
		// make quantity the current quantity
		m_currentOutputQuantity = m_ui->tableWidgetAvailableResults->item(row, 1)->text();
		m_currentOutputUnit = m_ui->tableWidgetAvailableResults->item(row, 2)->text();
		// check if the respective file is in cache
		Q_ASSERT(m_outputVariable2FileIndexMap.find(m_currentOutputQuantity) != m_outputVariable2FileIndexMap.end());
		unsigned int outputFileIndex = m_outputVariable2FileIndexMap[m_currentOutputQuantity];
		if (m_outputFiles[(int)outputFileIndex].m_status == ResultDataSet::FS_Unread) {
			m_currentOutputQuantity.clear(); // not cached yet, cannot display
			m_currentOutputUnit.clear();
		}
		else {
			// set slider
			Q_ASSERT(!m_allResults[m_currentOutputQuantity].empty());
			// get time points from first data set for this quantity
			IBK::UnitVector timePointVec;
			timePointVec.m_data = m_allResults[m_currentOutputQuantity].begin()->second.m_values.x();
			timePointVec.m_unit = m_allResults[m_currentOutputQuantity].begin()->second.m_xUnit;
			m_currentOutputUnit = QString::fromStdString( m_allResults[m_currentOutputQuantity].begin()->second.m_yUnit.name() );
			m_ui->widgetTimeSlider->setValues(timePointVec);
			m_ui->widgetTimeSlider->setCurrentValue(timePointVec.m_data.back());
			// determine max/min values
			setCurrentMinMaxValues(false);
		}
	}

	m_ui->groupBoxAnalysis->setEnabled(!m_currentOutputQuantity.isEmpty());
	m_ui->groupBoxCurrentSelection->setEnabled(!m_currentOutputQuantity.isEmpty());
	if (m_currentOutputQuantity.isEmpty())
		SVViewStateHandler::instance().m_geometryView->colorLegend()->setTitle("");
	else
		SVViewStateHandler::instance().m_geometryView->colorLegend()->setTitle(QString("%1 [%2]").arg(m_currentOutputQuantity, m_currentOutputUnit));

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
	readDataFile(m_outputFiles[(int)outputFileIndex].m_filename);
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

	QProgressDialog progDiag("", tr("Cancel"), 0, 100, this);
	progDiag.setWindowTitle(tr("Reading results directory"));
	progDiag.setValue(0);
	progDiag.setMinimumDuration(0);
	qApp->processEvents();

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
			qApp->processEvents();
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

	// now read result files
	QDir resultFilesDir = QDir(m_resultsDir.absolutePath() + "/results");
	QStringList tsvFiles = resultFilesDir.entryList(QStringList() << "*.tsv", QDir::Files);
	QStringList btfFiles = resultFilesDir.entryList(QStringList() << "*.btf", QDir::Files);
	QStringList files;
	QSet<QString> filesFound;

	// set file Mode
	if (!tsvFiles.empty()) {
		m_resultFileType = FT_TSV;
		files = tsvFiles;
	}
	else if (!btfFiles.empty()) {
		m_resultFileType = FT_BTF;
		files = btfFiles;
	}
	else {
		m_resultFileType = FT_None;
		return;
	}

	// update progress dialog
	progDiag.setMaximum(files.size()+1);
	int nProg = 0;

	for (const QString &fileName: qAsConst(files)) {

		progDiag.setLabelText(tr("Reading header of file '%1'").arg(fileName));
		progDiag.setValue(++nProg);
		qApp->processEvents();

		std::vector<std::string> captions;
		std::vector<std::string> units;
		QString absoluteFilePath = resultFilesDir.absoluteFilePath(fileName);

		// read tsv header
		if (m_resultFileType == FT_TSV) {
			IBK::CSVReader reader;
			IBK::Path tsvFilePath(absoluteFilePath.toStdString());
			try {
				reader.read(tsvFilePath, true, true);
			}
			catch (IBK::Exception & ex) {
				ex.writeMsgStackToError();
				IBK::IBK_Message(IBK::FormatString("Error parsing file '%1").arg(tsvFilePath), IBK::MSG_ERROR);
				continue; // skip this file
			}

			if (reader.m_captions.empty() || reader.m_units.empty()) {
				IBK::IBK_Message(IBK::FormatString("Empty result file '%1").arg(tsvFilePath), IBK::MSG_ERROR);
				continue;
			}

			// check captions, units, remove first column (which is the time)
			captions = reader.m_captions;
			units = reader.m_units;
			captions.erase(captions.begin());
			units.erase(units.begin());
			if (captions.size() != units.size() || captions.empty())
				continue;
		}
		// read btf header
		else if (m_resultFileType == FT_BTF) {
			VICUS::BTFReader btfReader;
			try {
				btfReader.parseHeaderData(absoluteFilePath, captions, units);
			}
			catch (IBK::Exception & ex) {
				ex.writeMsgStackToError();
				IBK::IBK_Message(IBK::FormatString("Error parsing file '%1").arg(absoluteFilePath.toStdString()), IBK::MSG_ERROR);
				continue; // skip this file
			}
			if (captions.empty())
				continue;
		}


		// remember this file so that we can find missing files later
		filesFound.insert(fileName);
		// update file status
		int outputFileIdx=0;
		for (; outputFileIdx<m_outputFiles.count(); ++outputFileIdx) {
			if (m_outputFiles[outputFileIdx].m_filename == fileName) {
				// previously read?
				if (m_outputFiles[outputFileIdx].m_status != ResultDataSet::FS_Unread) {
					// check time stamp on file
					if (QFileInfo(absoluteFilePath).lastModified() > m_outputFiles[outputFileIdx].m_timeStampLastUpdated)
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

		// we go through the header and look for our object names
		for (unsigned int i=0; i<captions.size(); ++i) {
			QString caption = QString::fromStdString(captions[i]);
			QString unit = QString::fromStdString(units[i]);
			QString outputName;
			for (auto it=m_objectName2Id.begin(); it!=m_objectName2Id.end(); ++it) {
				if (caption.contains(it->first)){
					// In case of network pipes: If we have "SupplyPipe" or "ReturnPipe" in the caption, we want to store that in our output name
					QString addOutputName;
					if (caption.contains("SupplyPipe"))
						addOutputName = "SupplyPipe-";
					else if (caption.contains("ReturnPipe"))
						addOutputName = "ReturnPipe-";
					outputName = addOutputName + caption.remove(it->first + ".");
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
	m_ui->groupBoxTime->setEnabled(validOutputFound);
	m_ui->groupBoxAnalysis->setEnabled(validOutputFound);
	m_ui->groupBoxCurrentSelection->setEnabled(validOutputFound);

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

	progDiag.close();
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

	m_ui->tableWidgetAvailableResults->selectRow(selectedRow);
}


void SVPropResultsWidget::readDataFile(const QString & filename) {
	FUNCID(SVPropResultsWidget::readDataFile);

	if (m_resultFileType == FT_None)
		return;

	QString fullFilePath = m_resultsDir.absoluteFilePath("results/" + filename);
	QProgressDialog progDiag(tr("Reading file '%1'").arg(filename), tr("Cancel"), 0, 100, this);
	progDiag.setWindowTitle(tr("Reading results file"));
	progDiag.setValue(0);
	progDiag.setAutoClose(true);
	progDiag.setMinimumDuration(0);
	qApp->processEvents();

	std::vector<std::string> captions;
	std::vector<IBK::Unit> units;
	IBK::CSVReader reader;
	VICUS::BTFReader btfReader;
	IBK::UnitVector timeSeconds;

	std::vector<std::vector<double> > dataColMajor;

	try {
		if (m_resultFileType == FT_TSV) {
			// read entire file
			reader.read(IBK::Path(fullFilePath.toStdString()), false, true);
			if (reader.m_nColumns < 2 || reader.m_nRows < 5)
				throw IBK::Exception("Missing data in file.", FUNC_ID);

			// check time unit
			IBK::Unit timeUnit = IBK::Unit(reader.m_units[0]); // may throw an exception
			if (timeUnit.base_unit() != IBK::Unit("s"))
				throw IBK::Exception("Invalid time unit.", FUNC_ID);
			// store captions, units
			for (unsigned int i=0; i<reader.m_captions.size(); ++i) {
				captions.push_back(reader.m_captions[i]);
				units.push_back(IBK::Unit(reader.m_units[i]));
			}
			std::vector<double> time = reader.colData(0);
			timeSeconds = IBK::UnitVector(time.begin(), time.end(), timeUnit);
			timeSeconds.convert(IBK::Unit("s"));
		}
		else if (m_resultFileType == FT_BTF) {
			// read header and data
			std::vector<std::string> ustrs;
			btfReader.readData(fullFilePath, timeSeconds, dataColMajor, captions, ustrs);
			for (const std::string & ustr: ustrs)
				units.push_back(IBK::Unit(ustr));
			timeSeconds.convert(IBK::Unit("s"));
		}
		else {
			throw IBK::Exception("Invalid result file type.", FUNC_ID);
		}
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
	progDiag.setMaximum((int)captions.size()-1);

	// process all columns of the tsv file, except time column
	for (unsigned int i=1; i<captions.size(); ++i) {

		progDiag.setValue((int)i);

		// check if this caption is among our recognized captions
		QString qCaption = QString::fromStdString(captions[i]);
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

		// In case of network pipes: If we have "SupplyPipe" or "ReturnPipe" in the caption, we want to store that in our output name
		QString addOutputName;
		if (qCaption.contains("SupplyPipe"))
			addOutputName = "SupplyPipe-";
		else if (qCaption.contains("ReturnPipe"))
			addOutputName = "ReturnPipe-";

		// remove the object reference, for example
		// "BuildingName.E0.WE0.0_Bath(ID=100)" from
		// caption "BuildingName.E0.WE0.0_Bath(ID=100).AirTemperature"  -> "AirTemperature"
		// caption "SupplyPipe(ID=100).FluidMassflux"					-> "SupplyPipe-FluidMassflux"
		QString outputName = addOutputName + qCaption.mid(dotpos+1);

		// and store in map with all outputs
		if (m_resultFileType == FT_TSV) {
			m_allResults[outputName][id] = NANDRAD::LinearSplineParameter(captions[i], NANDRAD::LinearSplineParameter::I_LINEAR,
																				   timeSeconds.m_data, reader.colData(i), IBK::Unit("s"), units[i]);
		}
		else if (m_resultFileType == FT_BTF) {
			m_allResults[outputName][id] = NANDRAD::LinearSplineParameter(captions[i], NANDRAD::LinearSplineParameter::I_LINEAR,
																				   timeSeconds.m_data, dataColMajor[i], IBK::Unit("s"), units[i]);
		}
	} // for captions in file

	progDiag.setValue((int)captions.size()-1); // fill and closes the dialog

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
	unsigned int maxIdx=0, minIdx=0;

	bool convertToAbs = m_ui->checkBoxConvertToAbsolute->isChecked();

	for (auto it=m_allResults[m_currentOutputQuantity].begin(); it!=m_allResults[m_currentOutputQuantity].end(); ++it) {

		IBK::LinearSpline vals = it->second.m_values;

		// local min/max
		if (localMinMax) {
			if (convertToAbs)
				max = std::abs( vals.nonInterpolatedValue(currentTime) );
			else
				max = vals.nonInterpolatedValue(currentTime);
			min = max;
		}
		// global min/max
		else {
			// the conversion to absolute values is a bit cumbersome, we need to iterate through the vector
			max = std::numeric_limits<double>::lowest();
			min = std::numeric_limits<double>::max();
			for (unsigned int i=0; i<vals.y().size(); ++i) {
				const double &val = vals.y()[i];

				if (convertToAbs) {
					if (std::abs(val) > max) {
						max = std::abs(val);
						maxIdx = i;
					}
					else if (std::abs(val) < min) {
						min = std::abs(val);
						minIdx = i;
					}
				}
				else if (val > max) {
					max = val;
					maxIdx = i;
				}
				else if (val < min) {
					min = val;
					minIdx = i;
				}
			}
		}

		if (max > m_currentMax) {
			m_currentMax = max;
			m_currentMaxIdx = maxIdx;
		}
		if (min < m_currentMin) {
			m_currentMin = min;
			m_currentMinIdx = minIdx;
		}
	}

	m_ui->lineEditMaxValue->setValue(m_currentMax);
	m_ui->lineEditMinValue->setValue(m_currentMin);
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
				double yint = m_allResults[m_currentOutputQuantity][e.m_id].m_values.nonInterpolatedValue(currentTime);
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
					double yint = m_allResults[m_currentOutputQuantity][r.m_id].m_values.nonInterpolatedValue(currentTime);
					interpolateColor(yint, col);
					for (const VICUS::Surface & s : r.m_surfaces) {
						s.m_color = col;
						// we dont color the subsurfaces
//						for (const VICUS::SubSurface &ss: s.subSurfaces())
//							ss.m_color = col;
					}
				}
				// a surface related property (e.g. SurfaceTemperature)
				else if (haveData) {
					for (const VICUS::Surface & s : r.m_surfaces) {
						if (m_allResults[m_currentOutputQuantity].find(s.m_id) != m_allResults[m_currentOutputQuantity].end()) {
							double yint = m_allResults[m_currentOutputQuantity][s.m_id].m_values.nonInterpolatedValue(currentTime);
							interpolateColor(yint, col);
							s.m_color = col;
						}
						for (const VICUS::SubSurface &ss: s.subSurfaces()) {
							if (m_allResults[m_currentOutputQuantity].find(ss.m_id) != m_allResults[m_currentOutputQuantity].end()) {
								double yint = m_allResults[m_currentOutputQuantity][ss.m_id].m_values.nonInterpolatedValue(currentTime);
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


bool SVPropResultsWidget::readColorMap(const QString & filename){
	const char * const FUNC_ID = "[SVPropResultsWidget::readColorMap]";

	if (filename.isEmpty())
		return false;

	TiXmlDocument doc( filename.toStdString() );
	if (!doc.LoadFile()) {
		QMessageBox::critical(this, tr("File error"), tr("Error in line %1 of file '%2':\n%3")
							  .arg(doc.ErrorRow())
							  .arg(filename)
							  .arg(doc.ErrorDesc()));
		return false;
	}

	// now we parse the different sections of the color map file
	try {
		// we use a handle so that NULL pointer checks are done during the query functions
		TiXmlHandle xmlHandleDoc(&doc);

		TiXmlElement * xmlElem = xmlHandleDoc.FirstChildElement().Element();
		if (!xmlElem)
			return false; // empty project, this means we are using only defaults
		std::string rootnode = xmlElem->Value();
		if (rootnode != "PostProcColorMap")
			throw IBK::Exception("Expected PostProcColorMap as root node in XML file.", FUNC_ID);

		TiXmlHandle xmlRoot = TiXmlHandle(xmlElem);

		xmlElem = xmlRoot.FirstChild( "ColorMap" ).Element();
		if (!xmlElem)
			throw IBK::Exception(IBK::FormatString("Expected top-level 'ColorMap' element."), FUNC_ID);

		m_colorMap.readXML(xmlElem);
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		QMessageBox::critical(this, tr("File error"), tr("Error reading color map from file. See logfile '%1' for details."));
		return false;
	}

	updateColors(m_ui->widgetTimeSlider->currentCutValue());

	return true;
}


void SVPropResultsWidget::interpolateColor(double y, QColor & col) const {
	y = (y-m_currentMin) / (m_currentMax-m_currentMin);
	if (m_ui->checkBoxConvertToAbsolute->isChecked())
		m_colorMap.interpolateColor(std::abs(y), col);
	else
		m_colorMap.interpolateColor(y, col);
}


void SVPropResultsWidget::on_pushButtonSetDefaultColormap_clicked() {
	QString dbDir = QtExt::Directories::databasesDir();
	QString filename = dbDir + "/colormaps/turbo.p2colormap";
	if (QFileInfo::exists(filename)) {
		if (readColorMap(filename))
			return;
	}
	// fall back - just use red and blue
	m_colorMap.m_linearColorStops.push_back(SVColorMap::ColorStop(0,"#669bbc"));
	m_colorMap.m_linearColorStops.push_back(SVColorMap::ColorStop(1,"#cb1b16"));
	updateColors(m_ui->widgetTimeSlider->currentCutValue());
}


void SVPropResultsWidget::on_pushButtonSetColormapViridis_clicked() {
	QString dbDir = QtExt::Directories::databasesDir();
	QString filename = dbDir + "/colormaps/viridis.p2colormap";
	if (QFileInfo::exists(filename)) {
		if (readColorMap(filename))
			return;
	}
	on_pushButtonSetDefaultColormap_clicked();
}


void SVPropResultsWidget::on_pushButtonSetColormapSpectral_clicked() {
	QString dbDir = QtExt::Directories::databasesDir();
	QString filename = dbDir + "/colormaps/spectral.p2colormap";
	if (QFileInfo(filename).exists()) {
		if (readColorMap(filename))
			return;
	}
	on_pushButtonSetDefaultColormap_clicked();
}



void SVPropResultsWidget::on_pushButtonReadColormap_clicked() {
	QString filename = QFileDialog::getOpenFileName( this,
			tr("Select Color Map File"), m_lastOpenFileLocation, tr("Color map files (*.p2colormap);;All files (*.*)") );
	readColorMap(filename);
}


void SVPropResultsWidget::on_pushButtonSaveColormap_clicked() {
	QString filename = QFileDialog::getSaveFileName( this,
			tr("Select Color Map File"), m_lastOpenFileLocation, tr("Color map files (*.p2colormap);;All files (*.*)") );

	if (filename.isEmpty()) return;

	// store open file location
	QFileInfo finfo(filename);
	m_lastOpenFileLocation = finfo.absoluteDir().absolutePath();
	if (finfo.suffix() != "p2colormap")
		filename += ".p2colormap";

	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "PostProcColorMap" );
	doc.LinkEndChild(root);

	root->SetAttribute("xmlns", "http://www.bauklimatik-dresden.de");
	root->SetAttribute("xmlns:IBK", "http://www.bauklimatik-dresden.de/IBK");
	root->SetAttribute("xsi:schemaLocation", "http://www.bauklimatik-dresden.de PostProcColorMap.xsd");
	root->SetAttribute("fileVersion", "1.0");

	TiXmlElement * e = new TiXmlElement("ColorMap");
	root->LinkEndChild(e);
	m_colorMap.writeXML(e);

	doc.SaveFile( filename.toStdString() );
}


void SVPropResultsWidget::on_checkBoxConvertToAbsolute_stateChanged(int /*arg1*/) {
	on_pushButtonSetGlobalMinMax_clicked();
}


void SVPropResultsWidget::on_pushButtonJumpToMax_clicked() {
	setCurrentMinMaxValues();
	m_ui->widgetTimeSlider->setCurrentIndex(m_currentMaxIdx);
}


void SVPropResultsWidget::on_pushButtonJumpToMin_clicked() {
	setCurrentMinMaxValues();
	m_ui->widgetTimeSlider->setCurrentIndex(m_currentMinIdx);
}


void SVPropResultsWidget::selectTargetObject(unsigned int targetId) {
	const VICUS::Object *obj = project().objectById(targetId);
	bool selectChildren = dynamic_cast<const VICUS::Room*>(obj) != nullptr;
	// create undo-action that toggles the selection
	SVUndoTreeNodeState * action = SVUndoTreeNodeState::createUndoAction(tr("Selection changed"), SVUndoTreeNodeState::SelectedState,
																		 targetId, selectChildren, true, true);
	action->push();
	// now signal the navigation tree view to scroll
	SVViewStateHandler::instance().m_navigationTreeWidget->scrollToObject(targetId);
	// and reset camera to the object
	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_FindSelection);
}


void SVPropResultsWidget::updateLineEditCurrentValue() {
	m_ui->lineEditCurrentValue->clear();
	if (m_selectedObjectId == VICUS::INVALID_ID)
		return;
	if (m_allResults.find(m_currentOutputQuantity) == m_allResults.end())
		return;
	auto it = m_allResults.at(m_currentOutputQuantity).find(m_selectedObjectId);
	if (it != m_allResults.at(m_currentOutputQuantity).end()) {
		double t = m_ui->widgetTimeSlider->currentCutValue();
		double val = it->second.m_values.nonInterpolatedValue(t);
		if (m_ui->checkBoxConvertToAbsolute->isChecked())
			val = std::abs(val);
		m_ui->lineEditCurrentValue->setText(QString("%1 %2").arg(val).arg(m_currentOutputUnit));
	}
	else
		m_ui->lineEditCurrentValue->setText(tr("output not available for selected object"));
}


void SVPropResultsWidget::on_pushButtonFindMaxObject_clicked() {
	double t = m_ui->widgetTimeSlider->currentCutValue();
	unsigned int targetId = VICUS::INVALID_ID;
	double maxVal = std::numeric_limits<double>::lowest();
	for (auto it=m_allResults[m_currentOutputQuantity].begin(); it!=m_allResults[m_currentOutputQuantity].end(); ++it) {
		const double &val = it->second.m_values.nonInterpolatedValue(t);
		if (val > maxVal) {
			maxVal = val;
			targetId = it->first;
		}
	}
	if (targetId != VICUS::INVALID_ID)
		selectTargetObject(targetId);
}


void SVPropResultsWidget::on_pushButtonFindMinObject_clicked() {
	double t = m_ui->widgetTimeSlider->currentCutValue();
	unsigned int targetId = VICUS::INVALID_ID;
	double minVal = std::numeric_limits<double>::max();
	for (auto it=m_allResults[m_currentOutputQuantity].begin(); it!=m_allResults[m_currentOutputQuantity].end(); ++it) {
		const double &val = it->second.m_values.nonInterpolatedValue(t);
		if (val < minVal) {
			minVal = val;
			targetId = it->first;
		}
	}
	if (targetId != VICUS::INVALID_ID)
		selectTargetObject(targetId);
}


void SVPropResultsWidget::on_resultsDir_editingFinished() {
	on_pushButtonRefreshDirectory_clicked();
}

