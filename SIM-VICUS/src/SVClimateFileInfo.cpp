#include "SVClimateFileInfo.h"

#include <QtExt_LanguageHandler.h>

#include "SVSettings.h"

SVClimateFileInfo::SVClimateFileInfo() :
	m_fileLocationType(FLT_CustomPath),
	m_longitudeInDegree(13.737),
	m_latitudeInDegree(51.05),
	m_elevation(0),
	m_timeZone(1)
{
	m_checkBits.fill(CCM::ClimateDataLoader::ALL_DATA_MISSING);
}


SVClimateFileInfo::SVClimateFileInfo(const QString& name, const QString& file, FileLocationType fileLocationType) :
	m_name(name),
	m_filename(file),
	m_fileLocationType(fileLocationType),
	m_longitudeInDegree(13.737),
	m_latitudeInDegree(51.05),
	m_elevation(0),
	m_timeZone(1)
{
	m_checkBits.fill(CCM::ClimateDataLoader::ALL_DATA_MISSING);
}


void SVClimateFileInfo::readInfo(const QFileInfo& file, bool withData, bool clearName) {
	bool sameFile = m_file == file;
	if(!sameFile) {
		m_file = file;
		m_filename = file.fileName();
	}
	if(clearName)
		m_name.clear();

	// read data
	IBK::Path cliPath(file.absoluteFilePath().toUtf8().data());
	// read only in case of Only header, new file or not already read
	if(!withData || !sameFile || m_loader.m_data[0].empty()) {
		try {
			m_loader.readClimateData(cliPath, !withData);
		}
		catch (IBK::Exception& ex) {
			m_file = QFileInfo();
			m_filename.clear();
			m_name.clear();
			throw IBK::Exception(ex, "Could not read climate file", "[SVClimateFileInfo::readInfo]");
		}
	}
	if(withData)
		m_checkBits = m_loader.m_checkBits;
	else {
		m_checkBits.fill(CCM::ClimateDataLoader::ALL_DATA_MISSING);
	}

	QString langID = QtExt::LanguageHandler::instance().langId();

	IBK::MultiLanguageString cityName(m_loader.m_city);
	IBK::MultiLanguageString countryName(m_loader.m_country);
	m_city = QString::fromStdString(cityName.string(langID.toStdString()));
	if (m_city.isEmpty())
		m_city = QString::fromStdString(cityName.string("en"));
	m_country = QString::fromStdString(countryName.string(langID.toStdString()));
	if (m_country.isEmpty())
		m_country = QString::fromStdString(countryName.string("en"));
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

