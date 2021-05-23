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

	if (m_location->m_climateFilePath.isValid()) {
		// is the referenced file in the climate database?
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
			// file is not contained in the database or user database, assume absolute file path
			m_ui->filepathClimateDataFile->blockSignals(true);
			m_ui->filepathClimateDataFile->setFilename(QString::fromStdString(m_location->m_climateFilePath.str()));
			// try to read the user-defined climate data file - if this fails, the m_userClimateFile info structure
			// will be empty
			updateUserClimateFileInfo();
			// now update the
			updateLocationInfo(&m_userClimateFile, false); // false = no database file
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

	// also update the location line edits
	m_ui->lineEditLatitude->setValue(climateInfoPtr->m_latitudeInDegree);
	m_ui->lineEditLongitude->setValue(climateInfoPtr->m_longitudeInDegree);

	// we also want to update the NANDRAD::Location (needed for shading calculation)
	NANDRAD::KeywordList::setParameter(m_location->m_para, "Location::para_t", NANDRAD::Location::P_Longitude, climateInfoPtr->m_longitudeInDegree);
	NANDRAD::KeywordList::setParameter(m_location->m_para, "Location::para_t", NANDRAD::Location::P_Latitude, climateInfoPtr->m_latitudeInDegree);
	m_location->m_timeZone = climateInfoPtr->m_timeZone;

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
	updateLocationInfo(&m_userClimateFile, false);
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
