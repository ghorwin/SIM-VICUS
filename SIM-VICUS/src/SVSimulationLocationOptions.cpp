#include "SVSimulationLocationOptions.h"
#include "ui_SVSimulationLocationOptions.h"

#include <QFileInfo>

#include <NANDRAD_Location.h>
#include <NANDRAD_KeywordList.h>

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
	m_ui->verticalLayout->setMargin(0);

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

	m_ui->filepathClimateDataFile->setup("", true, true, tr("Climate data container files (*.c6b *.epw *.wac);;All files (*.*)"));
}


SVSimulationLocationOptions::~SVSimulationLocationOptions() {
	delete m_ui;
}


void SVSimulationLocationOptions::updateUi() {
	m_ui->tableViewClimateFiles->resizeColumnsToContents();

	if (m_location->m_para[NANDRAD::Location::P_Albedo].name.empty())
		m_location->m_para[NANDRAD::Location::P_Albedo].set("Albedo", 0.2, "---");

	m_ui->comboBoxAlbedo->setValue(m_location->m_para[NANDRAD::Location::P_Albedo].value);
	m_ui->checkBoxCustomLocation->blockSignals(true);
	// if both latitude and longitude are given, we have a custom climate location
	if (!m_location->m_para[NANDRAD::Location::P_Latitude].name.empty() &&
		!m_location->m_para[NANDRAD::Location::P_Longitude].name.empty())
	{
		m_ui->checkBoxCustomLocation->setChecked(true);
	}
	else {
		// no custom location? clear location and latitude parameters
		m_location->m_para[NANDRAD::Location::P_Latitude].clear();
		m_location->m_para[NANDRAD::Location::P_Longitude].clear();
		m_ui->checkBoxCustomLocation->setChecked(false);
	}
	m_ui->checkBoxCustomLocation->blockSignals(false);
	on_checkBoxCustomLocation_toggled(m_ui->checkBoxCustomLocation->isChecked());

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


void SVSimulationLocationOptions::updateLocationInfo(const SVClimateFileInfo * climateInfoPtr) {
	if (climateInfoPtr == nullptr)
		m_location->m_climateFileName.clear();
	else
		m_location->m_climateFileName = climateInfoPtr->m_file.absoluteFilePath().toStdString();

	// update info text on climate location
	m_ui->textBrowserDescription->clear();
	// default values for location
	m_ui->lineEditLatitude->setValue(51.1);
	m_ui->lineEditLongitude->setValue(13.1);
	m_ui->comboBoxTimeZone->setCurrentIndex(13);

	if (climateInfoPtr == nullptr) {
		return;
	}
	QString infoText;
	infoText = "<html><body>";
	if (!climateInfoPtr->m_file.isFile()) {
		infoText += "<p>" + tr("Invalid climate data file path.") + "</p>";
	}
	else {
		if (m_ui->radioButtonFromFile->isChecked()) {
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
	f.setPointSizeF(f.pointSizeF()*0.8);
	m_ui->textBrowserDescription->setFont(f);

	// also update the location line edits
	m_ui->lineEditLatitude->setValue(climateInfoPtr->m_latitudeInDegree);
	m_ui->lineEditLongitude->setValue(climateInfoPtr->m_longitudeInDegree);
	int index = climateInfoPtr->m_timeZone+12;
	if (index < 0 || index > 24)
		index = 12;
	m_ui->comboBoxTimeZone->setCurrentIndex(index);
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


void SVSimulationLocationOptions::storeCustomLocationInputs(){
	NANDRAD::KeywordList::setParameter(m_location->m_para, "Location::para_t",
									   NANDRAD::Location::P_Latitude, m_ui->lineEditLatitude->value());
	NANDRAD::KeywordList::setParameter(m_location->m_para, "Location::para_t",
									   NANDRAD::Location::P_Longitude, m_ui->lineEditLongitude->value());
}


void SVSimulationLocationOptions::on_radioButtonFromDB_toggled(bool checked) {
	m_ui->tableViewClimateFiles->setEnabled(checked);
	m_ui->labelTextFilter->setEnabled(checked);
	m_ui->lineEditTextFilter->setEnabled(checked);
	m_ui->filepathClimateDataFile->setEnabled(!checked);
	if (checked) {
		onCurrentIndexChanged(m_ui->tableViewClimateFiles->currentIndex(), QModelIndex());
	}
	else {
		updateUserClimateFileInfo();
		updateLocationInfo(&m_userClimateFile);
	}
}


void SVSimulationLocationOptions::on_lineEditTextFilter_editingFinished() {
	// update the filter text
	m_filterModel->setFilterWildcard(m_ui->lineEditTextFilter->text().trimmed());
}


void SVSimulationLocationOptions::on_lineEditTextFilter_textChanged(const QString &arg1) {
	m_filterModel->setFilterWildcard(arg1);
}


void SVSimulationLocationOptions::on_filepathClimateDataFile_editingFinished() {
	updateUserClimateFileInfo();
	updateLocationInfo(&m_userClimateFile);
}

void SVSimulationLocationOptions::on_checkBoxCustomLocation_toggled(bool checked) {
	m_ui->lineEditLatitude->setEnabled(checked);
	m_ui->lineEditLongitude->setEnabled(checked);
	m_ui->comboBoxTimeZone->setEnabled(checked);
	m_ui->labelLatitude->setEnabled(checked);
	m_ui->labelLongitude->setEnabled(checked);
	m_ui->labelTimeZone->setEnabled(checked);
	if (checked) {
		storeCustomLocationInputs();
	}
	else {
		// no custom location - clear custom inputs
		m_location->m_para[NANDRAD::Location::P_Altitude].clear();
		m_location->m_para[NANDRAD::Location::P_Latitude].clear();
		m_location->m_para[NANDRAD::Location::P_Longitude].clear();
	}
}
