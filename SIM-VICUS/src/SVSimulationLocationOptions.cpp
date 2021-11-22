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

#include <NANDRAD_Location.h>
#include <NANDRAD_KeywordList.h>

#include "SVSettings.h"
#include "SVClimateDataTableModel.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVClimateDataSortFilterProxyModel.h"
#include "SVProjectHandler.h"

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

	SVDBModelDelegate * delegate = new SVDBModelDelegate(this, Role_BuiltIn, Role_Local, Role_Referenced);
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

	m_ui->filepathClimateDataFile->setup("", true, true, tr("Climate data container files (*.c6b *.epw *.wac);;All files (*.*)"),
										 SVSettings::instance().m_dontUseNativeDialogs);
}


SVSimulationLocationOptions::~SVSimulationLocationOptions() {
	delete m_ui;
}


void SVSimulationLocationOptions::updateUi() {
	m_ui->tableViewClimateFiles->resizeColumnsToContents();

	// *** input data check ***

	// user may have invalid units or value ranges in project file

	if (m_location->m_para[NANDRAD::Location::P_Albedo].name.empty() ||
		m_location->m_para[NANDRAD::Location::P_Albedo].IO_unit.base_id() != IBK::Unit("---").base_id())
	{
		// set/fix Albedo
		m_location->m_para[NANDRAD::Location::P_Albedo].set("Albedo", 0.2, "---");
	}
	m_ui->comboBoxAlbedo->setValue(m_location->m_para[NANDRAD::Location::P_Albedo].value);

	int index = m_location->m_timeZone+12;
	// fix invalid time zone in project file
	if (index < 0 || index > 24) {
		index = 13; // UTC+1 as default fall-back
		m_location->m_timeZone = index - 12;
	}
	m_ui->comboBoxTimeZone->setCurrentIndex(index);

	// Latitude is optional, but if given, must have the correct unit
	if (!m_location->m_para[NANDRAD::Location::P_Latitude].name.empty() &&
		m_location->m_para[NANDRAD::Location::P_Latitude].IO_unit.base_id() != IBK::Unit("Deg").base_id())
	{
		NANDRAD::KeywordList::setParameter(m_location->m_para, "Location::para_t", NANDRAD::Location::P_Latitude, 51.1);
	}

	// Longitude is optional, but if given, must have the correct unit
	if (!m_location->m_para[NANDRAD::Location::P_Longitude].name.empty() &&
		m_location->m_para[NANDRAD::Location::P_Longitude].IO_unit.base_id() != IBK::Unit("Deg").base_id())
	{
		NANDRAD::KeywordList::setParameter(m_location->m_para, "Location::para_t", NANDRAD::Location::P_Longitude, 13.1);
	}


	// *** now populate the user interface ***


	// update rules for location input widget:
	//
	// - if latitude and longitude are stored in location (the default), we
	//   initially assume a custom location; this gives us always defined and available
	//   location data for other calculations (like shading)
	// - if additionally a climate data file is given, we update the data in updateLocationInfo()
	// - there, if latitute and longitude from file match our current "custom" input, we toggle back
	//   to "from climate file"

	m_ui->checkBoxCustomLocation->blockSignals(true);
	// if both latitude and longitude are given, we have a custom climate location
	if (!m_location->m_para[NANDRAD::Location::P_Latitude].name.empty() &&
		!m_location->m_para[NANDRAD::Location::P_Longitude].name.empty())
	{
		m_ui->checkBoxCustomLocation->setChecked(true);
		// input data checks have been done already
		m_ui->lineEditLatitude->setValue(m_location->m_para[NANDRAD::Location::P_Latitude].get_value("Deg"));
		m_ui->lineEditLongitude->setValue(m_location->m_para[NANDRAD::Location::P_Longitude].get_value("Deg"));
		m_ui->comboBoxTimeZone->setCurrentIndex(m_location->m_timeZone+12);
	}
	else {
		m_ui->checkBoxCustomLocation->setChecked(false);
		// no custom location? clear location and latitude parameters
		m_location->m_para[NANDRAD::Location::P_Latitude].clear();
		m_location->m_para[NANDRAD::Location::P_Longitude].clear();
	}
	m_ui->checkBoxCustomLocation->blockSignals(false);
	on_checkBoxCustomLocation_toggled(m_ui->checkBoxCustomLocation->isChecked()); // only updates enabled/disabled states


	// do we have a climate file path given?
	if (m_location->m_climateFilePath.isValid()) {
		// is the referenced file in the climate database? If so, it has a placeholder "${Database}" in path or
		// ${User Database}
		QModelIndex idx;
		for (int i=0, count = m_climateDataModel->rowCount(QModelIndex()); i< count; ++i) {
			QModelIndex curIdx = m_climateDataModel->index(i, 0);
			// get the path including potential placeholders
			IBK::Path p(m_climateDataModel->data(curIdx, Role_FilePath).toString().toStdString());
			if (p == m_location->m_climateFilePath) {
				idx = curIdx;
				break;
			}
			// now try the absolute path, maybe that matches
			p = m_climateDataModel->data(curIdx, Role_AbsoluteFilePath).toString().toStdString();
			if (p == m_location->m_climateFilePath) {
				idx = curIdx;
				break;
			}
		}
		if (idx.isValid()) {
			m_ui->radioButtonFromDB->blockSignals(true);
			m_ui->radioButtonFromDB->setChecked(true);
			m_ui->radioButtonFromDB->blockSignals(false);
			m_ui->filepathClimateDataFile->blockSignals(true);
			m_ui->filepathClimateDataFile->setFilename("");
			m_ui->filepathClimateDataFile->blockSignals(false);

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

			// Note: update of location info is done through on_radioButtonFromDB_toggled() call below
		}
		else {
			// not a database file, might still contain a placeholder

			IBK::Path absPath = SVProjectHandler::instance().replacePathPlaceholders(m_location->m_climateFilePath);
			absPath.removeRelativeParts(); // remove any remaining ../.. in the middle

			m_ui->radioButtonFromDB->blockSignals(true); // slot is connected to first radio button, so block this
			m_ui->radioButtonFromFile->setChecked(true);
			m_ui->radioButtonFromDB->blockSignals(false);
			// file is not contained in the database or user database, assume absolute file path
			m_ui->filepathClimateDataFile->blockSignals(true);
			m_ui->filepathClimateDataFile->setFilename(QString::fromStdString(absPath.str()) );
			m_ui->filepathClimateDataFile->blockSignals(false);

			// select the correct radio button
			m_ui->radioButtonUserPathAbsolute->blockSignals(true);
			if (m_location->m_climateFilePath.str().find("${Project Directory}") != std::string::npos)
				m_ui->radioButtonUserPathRelative->setChecked(true);
			else
				m_ui->radioButtonUserPathAbsolute->setChecked(true);
			m_ui->radioButtonUserPathAbsolute->blockSignals(false);

			m_ui->lineEditUserFilePath->setText( QString::fromStdString(m_location->m_climateFilePath.str()));

			// Note: update of location info is done through on_radioButtonFromDB_toggled() call below
		}
	}
	else {
		// no climate file path, but we assume that user wants to use DB climate
		m_ui->radioButtonFromDB->blockSignals(true);
		m_ui->radioButtonFromDB->setChecked(true);
		m_ui->radioButtonFromDB->blockSignals(false);
	}
	on_radioButtonFromDB_toggled(m_ui->radioButtonFromDB->isChecked());
}


void SVSimulationLocationOptions::onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/)  {
	if (m_ui->radioButtonFromDB->isChecked()) {
		// get filename from current model and then update the climate station info text box
		const SVClimateFileInfo * p = (const SVClimateFileInfo *)current.data(Role_RawPointer).value<void*>();
		updateLocationInfo(p, true);
	}
}


void SVSimulationLocationOptions::updateLocationInfo(const SVClimateFileInfo * climateInfoPtr, bool databaseFile) {
	if (climateInfoPtr == nullptr)
		m_location->m_climateFilePath.clear();
	else {
		// store file with placeholder
		if (databaseFile)
			m_location->m_climateFilePath = climateInfoPtr->m_filename.toStdString();
		else
			m_location->m_climateFilePath = climateInfoPtr->m_absoluteFilePath.toStdString();
	}

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

	// check if location in DB file matches our custom location, and switch back to "from DB" mode in this case

	IBK::Parameter longFromDB("Longitude", climateInfoPtr->m_longitudeInDegree, "Deg");
	IBK::Parameter latFromDB("Latitude", climateInfoPtr->m_latitudeInDegree, "Deg");
	int timeZoneFromDB = climateInfoPtr->m_timeZone;

	// we compare with some rounding tolerance (deg->rad conversion)
	if (IBK::near_equal(longFromDB.value, m_location->m_para[NANDRAD::Location::P_Longitude].value) &&
		IBK::near_equal(latFromDB.value, m_location->m_para[NANDRAD::Location::P_Latitude].value) &&
		timeZoneFromDB == m_location->m_timeZone)
	{
		m_ui->checkBoxCustomLocation->setChecked(false);
	}

	// if we do not have custom climate, transfer data from selected climate location to our location object and UI
	if (!m_ui->checkBoxCustomLocation->isChecked()) {
		// we also want to update the NANDRAD::Location (needed for shading calculation)
		NANDRAD::KeywordList::setParameter(m_location->m_para, "Location::para_t", NANDRAD::Location::P_Longitude, climateInfoPtr->m_longitudeInDegree);
		NANDRAD::KeywordList::setParameter(m_location->m_para, "Location::para_t", NANDRAD::Location::P_Latitude, climateInfoPtr->m_latitudeInDegree);
		// guard against invalid data in climate data file
		int index = climateInfoPtr->m_timeZone + 12;
		// fix invalid time zone in project file
		if (index < 0 || index > 24) {
			index = 13; // UTC+1 as default fall-back
			m_location->m_timeZone = index - 12;
		}

		// Note : latitude and longitude may also be invalid in project file, but that will be checked in the solver
		m_ui->lineEditLatitude->setValue(climateInfoPtr->m_latitudeInDegree);
		m_ui->lineEditLongitude->setValue(climateInfoPtr->m_longitudeInDegree);
		m_ui->comboBoxTimeZone->setCurrentIndex(m_location->m_timeZone + 12);
	}
}


void SVSimulationLocationOptions::updateUserClimateFileInfo() {
	QString climateFile = m_ui->filepathClimateDataFile->filename().trimmed();
	m_userClimateFile = SVClimateFileInfo();
	if (climateFile.isEmpty()) {
		return;
	}

	try {
		m_userClimateFile.readInfo(QString(), climateFile, false, true);
	}
	catch (...) {
		m_userClimateFile = SVClimateFileInfo();
	}
}


void SVSimulationLocationOptions::storeCustomLocationInputs(){
	NANDRAD::KeywordList::setParameter(m_location->m_para, "Location::para_t",
									   NANDRAD::Location::P_Latitude, m_ui->lineEditLatitude->value());
	NANDRAD::KeywordList::setParameter(m_location->m_para, "Location::para_t",
									   NANDRAD::Location::P_Longitude, m_ui->lineEditLongitude->value());
	m_location->m_timeZone = m_ui->comboBoxTimeZone->currentData().toInt();
}


void SVSimulationLocationOptions::on_radioButtonFromDB_toggled(bool checked) {
	m_ui->tableViewClimateFiles->setEnabled(checked);
	m_ui->labelTextFilter->setEnabled(checked);
	m_ui->lineEditTextFilter->setEnabled(checked);
	m_ui->filepathClimateDataFile->setEnabled(!checked);
	m_ui->widgetUserPathOptions->setEnabled(!checked);
	if (checked) {
		onCurrentIndexChanged(m_ui->tableViewClimateFiles->currentIndex(), QModelIndex());
	}
	else {
		updateUserClimateFileInfo();
		updateLocationInfo(&m_userClimateFile, false);
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
	on_radioButtonUserPathAbsolute_toggled(m_ui->radioButtonUserPathAbsolute->isChecked());
	updateLocationInfo(&m_userClimateFile, false);
}


void SVSimulationLocationOptions::on_checkBoxCustomLocation_toggled(bool checked) {
	m_ui->lineEditLatitude->setEnabled(checked);
	m_ui->lineEditLongitude->setEnabled(checked);
	m_ui->comboBoxTimeZone->setEnabled(checked);
	m_ui->labelLatitude->setEnabled(checked);
	m_ui->labelLongitude->setEnabled(checked);
	m_ui->labelTimeZone->setEnabled(checked);
}


void SVSimulationLocationOptions::on_radioButtonUserPathAbsolute_toggled(bool checked) {
	// update local file path

	// we need a project file path for that
	QString p = SVProjectHandler::instance().projectFile();
	if (p.isEmpty())
		checked = true; // fall back to absolute path if project file hasn't been saved, yet

	if (checked) {
		m_ui->lineEditUserFilePath->setText( m_ui->filepathClimateDataFile->filename() );
	}
	else {
		IBK::Path proPath(p.toStdString());
		proPath = proPath.parentPath();
		IBK::Path climateFilePath(m_ui->filepathClimateDataFile->filename().toStdString());
		try {
			IBK::Path relPath = climateFilePath.relativePath(proPath);
			QString pathString = QString::fromStdString( (IBK::Path("${Project Directory}") / relPath).str());
			m_ui->lineEditUserFilePath->setText( pathString );
		} catch (...) {
			// can't relate paths... keep absolute
			m_ui->lineEditUserFilePath->setText( m_ui->filepathClimateDataFile->filename() );
		}
	}
	// finally update project data
	m_location->m_climateFilePath = IBK::Path(m_ui->lineEditUserFilePath->text().toStdString());
}
