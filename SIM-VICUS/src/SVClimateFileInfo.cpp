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

#include "SVClimateFileInfo.h"

#include <QtExt_LanguageHandler.h>
#include <QtExt_Directories.h>

#include "SVSettings.h"

SVClimateFileInfo::SVClimateFileInfo() :
	m_builtIn(false),
	m_longitudeInDegree(13.737),
	m_latitudeInDegree(51.05),
	m_elevation(0),
	m_timeZone(1)
{
	m_checkBits.fill(CCM::ClimateDataLoader::ALL_DATA_MISSING);
}


void SVClimateFileInfo::readInfo(const QString& databaseDir, const QString & absoluteFilePath, bool withData, bool builtIn) {
	FUNCID(SVClimateFileInfo::readInfo);
	QFileInfo finfo(absoluteFilePath);
	m_absoluteFilePath = absoluteFilePath;
	// store relative path to database dir
	if (!databaseDir.isEmpty()) {
		QDir dbDir(databaseDir);
		m_filename = dbDir.relativeFilePath(absoluteFilePath); // does not have placeholders, yet
		if (builtIn)
			m_filename = QString("${%1}/DB_climate/%2").arg(VICUS::DATABASE_PLACEHOLDER_NAME, m_filename );
		else
			m_filename = QString("${%1}/DB_climate/%2").arg(VICUS::USER_DATABASE_PLACEHOLDER_NAME, m_filename );
	}
	m_name = finfo.baseName();

	// read data
	IBK::Path cliPath(m_absoluteFilePath.toStdString());
	try {
		m_loader.readClimateData(cliPath, !withData); // function expects "header only" flag, which is "!withData"
	}
	catch (IBK::Exception& ex) {
		m_absoluteFilePath.clear();
		m_filename.clear();
		m_name.clear();
		throw IBK::Exception(ex, "Could not read climate file", FUNC_ID);
	}
	if (withData)
		m_checkBits = m_loader.m_checkBits;
	else
		m_checkBits.fill(CCM::ClimateDataLoader::ALL_DATA_MISSING);

	QString langID = QtExt::LanguageHandler::instance().langId();

	IBK::MultiLanguageString cityName(m_loader.m_city);
	IBK::MultiLanguageString countryName(m_loader.m_country);
	m_city = QString::fromStdString(cityName.string(langID.toStdString()));
	if (m_city.isEmpty())
		m_city = QString::fromStdString(cityName.string("en", true));
	m_country = QString::fromStdString(countryName.string(langID.toStdString()));
	if (m_country.isEmpty())
		m_country = QString::fromStdString(countryName.string("en", true));
	m_source = QString::fromStdString(m_loader.m_source);
	m_comment = QString::fromStdString(m_loader.m_comment);
	m_longitudeInDegree = m_loader.m_longitudeInDegree;
	m_latitudeInDegree = m_loader.m_latitudeInDegree;
	m_elevation = m_loader.m_elevation;
	m_timeZone = m_loader.m_timeZone;
	if(m_loader.m_dataTimePoints.empty())
		m_timeBehaviour = tr("One year, cyclic use.");
	else
		m_timeBehaviour = tr("%1 years, non-cyclic use.").
						  arg(int((m_loader.m_dataTimePoints.back() - m_loader.m_dataTimePoints.front()) / 3600.0 / 8760.0));
	m_builtIn = builtIn;
}


bool SVClimateFileInfo::hasShortwave() const {
	return m_checkBits[CCM::ClimateDataLoader::DirectRadiationNormal] == 0 &&
			m_checkBits[CCM::ClimateDataLoader::DiffuseRadiationHorizontal] == 0;
}


bool SVClimateFileInfo::hasLongwave() const {
	return m_checkBits[CCM::ClimateDataLoader::LongWaveCounterRadiation] == 0 ;
}


bool SVClimateFileInfo::hasRain() const {
	return m_checkBits[CCM::ClimateDataLoader::WindDirection] == 0 &&
			m_checkBits[CCM::ClimateDataLoader::WindVelocity] == 0 &&
			m_checkBits[CCM::ClimateDataLoader::Rain] == 0;
}


bool SVClimateFileInfo::hasTemperature() const {
	return m_checkBits[CCM::ClimateDataLoader::Temperature] == 0;
}


bool SVClimateFileInfo::hasRelHum() const {
	return m_checkBits[CCM::ClimateDataLoader::RelativeHumidity] == 0;
}


void SVClimateFileInfo::clear() {
	*this = SVClimateFileInfo();
}


