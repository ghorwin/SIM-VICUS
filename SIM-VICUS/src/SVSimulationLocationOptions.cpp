#include "SVSimulationLocationOptions.h"
#include "ui_SVSimulationLocationOptions.h"

#include <QFileInfo>

#include <NANDRAD_Location.h>

#include "SVSettings.h"
#include "SVClimateDataTableModel.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVClimateDataSortFilterProxyModel.h"

SVSimulationLocationOptions::SVSimulationLocationOptions(QWidget *parent, NANDRAD::Location & location) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationLocationOptions),
	m_location(&location)
{
	m_ui->setupUi(this);

	// source model
	m_climateDataModel = SVSettings::instance().climateDataTableModel();

	// proxy model
	m_filterModel = new SVClimateDataSortFilterProxyModel(this);
	m_filterModel->setSourceModel(m_climateDataModel);

	// set proxy model into table
	m_ui->tableViewClimateFiles->setModel(m_filterModel);

	SVDBModelDelegate * delegate = new SVDBModelDelegate(this, Role_BuiltIn);
	m_ui->tableViewClimateFiles->setItemDelegate(delegate);
	SVStyle::formatDatabaseTableView(m_ui->tableViewClimateFiles);

	connect(m_ui->tableViewClimateFiles->selectionModel(), &QItemSelectionModel::currentChanged,
			this, &SVSimulationLocationOptions::onCurrentIndexChanged);
}


SVSimulationLocationOptions::~SVSimulationLocationOptions() {
	delete m_ui;
}


void SVSimulationLocationOptions::updateUi() {
	m_ui->tableViewClimateFiles->resizeColumnsToContents();

	if (m_location->m_climateFileName.isValid()) {
		// is the referenced file in the climate database?
		QModelIndex idx;
		for (int i=0, count = m_climateDataModel->rowCount(QModelIndex()); i< count; ++i) {
			QModelIndex curIdx = m_climateDataModel->index(i, 0);
			IBK::Path p(m_climateDataModel->data(curIdx, Role_FilePath).toString().toStdString());
			if (p == m_location->m_climateFileName) {
				idx = curIdx;
				break;
			}
		}
		if (idx.isValid()) {
			m_ui->radioButtonFromDB->setChecked(true);
			m_ui->filepathClimateDataFile->blockSignals(true);
			m_ui->filepathClimateDataFile->setFilename("");
			m_ui->filepathClimateDataFile->blockSignals(false);

			// convert to proxy-index
			m_ui->tableViewClimateFiles->blockSignals(true);
			QModelIndex proxy = m_filterModel->mapFromSource(idx);
			// if not visible, reset all filters and convert to proxy index again
			if (!proxy.isValid()) {
				m_filterModel->setFilterText("");
			}
			proxy = m_filterModel->mapFromSource(idx);
			// select row
			m_ui->tableViewClimateFiles->setCurrentIndex(proxy);
			m_ui->tableViewClimateFiles->blockSignals(false);

			// manually trigger the currentChanged() signal
			onCurrentIndexChanged(proxy, QModelIndex());

		}
		else {
			m_ui->filepathClimateDataFile->blockSignals(true);
			m_ui->filepathClimateDataFile->setFilename(QString::fromStdString(m_location->m_climateFileName.str()));
			// try to read the user-defined climate data file - if this fails, the m_userClimateFile info structure
			// will be empty
			updateUserClimateFileInfo();
			// now update the
			updateLocationInfo(&m_userClimateFile);
			m_ui->filepathClimateDataFile->blockSignals(false);
		}
	}
	else {
		m_ui->radioButtonFromDB->setChecked(true);
	}
	on_radioButtonFromDB_toggled(m_ui->radioButtonFromDB->isChecked());
}



void SVSimulationLocationOptions::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/)  {
	if (m_ui->radioButtonFromDB->isChecked()) {
		// get filename from current model and then update the climate station info text box
		const SVClimateFileInfo * p = (const SVClimateFileInfo *)current.data(Role_RawPointer).value<void*>();
		updateLocationInfo(p);
	}
}


void SVSimulationLocationOptions::updateLocationInfo(const SVClimateFileInfo * dataPtr) {
	// update info text on climate location
	m_ui->textBrowserDescription->clear();
	if (dataPtr == nullptr) {
		return;
	}
	QString infoText;
	infoText = "<html><body>";
	if (!dataPtr->m_file.isFile()) {
		infoText += "<p>" + tr("Invalid climate data file path.") + "</p>";
	}
	else {
		if (m_ui->radioButtonFromFile->isChecked()) {
			infoText += "<p>" + tr("User climate data file.");
		}
		else {
			if (!dataPtr->m_builtIn)
				infoText += "<p>" + tr("Climate data from user database.");
			else
				infoText += "<p>" + tr("Climate data from standard database.");
		}
		infoText += "<br>" + dataPtr->m_timeBehaviour + "</p>";
		infoText += "<p>" + tr("City/Country") + ": <b>" + dataPtr->m_city + "</b>/";
		infoText += "<b>" + dataPtr->m_country + "</b>, ";
		infoText += tr("Source") + ": <b>" + dataPtr->m_source + "</b><br>";
		infoText += tr("Longitude") + ": <b>" + QString("%L1 Deg").arg(dataPtr->m_longitudeInDegree, 0, 'f', 2) + " </b>, ";
		infoText += tr("Latitude") + ": <b>" + QString("%L1 Deg").arg(dataPtr->m_latitudeInDegree, 0, 'f', 2) + " </b>, ";
		infoText += tr("Elevation") + ": <b>" + QString("%L1 m").arg(dataPtr->m_elevation, 0, 'f', 0) + " </b></p>";
		infoText += "<p>" + dataPtr->m_comment + "</p>";
	}
	infoText += "</body></html>";

	m_ui->textBrowserDescription->setHtml(infoText);
}


void SVSimulationLocationOptions::updateUserClimateFileInfo() {
	QString climateFile = m_ui->filepathClimateDataFile->filename().trimmed();
	m_userClimateFile = SVClimateFileInfo();
	if (climateFile.isEmpty()) {
		return;
	}

	try {
		m_userClimateFile.readInfo(QFileInfo(climateFile), false, true);
	} catch (...) {
		m_userClimateFile = SVClimateFileInfo();
	}
}


void SVSimulationLocationOptions::on_radioButtonFromDB_toggled(bool checked) {
	m_ui->tableViewClimateFiles->setEnabled(checked);
	m_ui->labelTextFilter->setEnabled(checked);
	m_ui->lineEditTextFilter->setEnabled(checked);
	m_ui->filepathClimateDataFile->setEnabled(!checked);
}


void SVSimulationLocationOptions::on_lineEditTextFilter_editingFinished() {
	// update the filter text
	m_filterModel->setFilterWildcard(m_ui->lineEditTextFilter->text().trimmed());
}


void SVSimulationLocationOptions::on_lineEditTextFilter_textChanged(const QString &arg1) {
	m_filterModel->setFilterWildcard(arg1);
}
