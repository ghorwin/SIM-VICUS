#ifndef SVClimateFileInfoH
#define SVClimateFileInfoH

#include <QFileInfo>
#include <QCoreApplication>

#include <array>

#include <CCM_ClimateDataLoader.h>

/*! Class contains informations from one climate data set read from a c6b, wac or epw file. */
class SVClimateFileInfo {
	Q_DECLARE_TR_FUNCTIONS(SVClimateFileInfo)
public:
	/*! Defines where file is located (used for relative path selection and for display properties in tree widget). */
	enum FileLocationType {
		FLT_ClimateDatabase,
		FLT_UserClimateDatabase,
		FLT_CustomPath
	};

	/*! Standard constructor.*/
	SVClimateFileInfo();

	/*! Constructor for basic initialisiation.*/
	SVClimateFileInfo(const QString& name, const QString& file, FileLocationType fileLocationType);

	/*! Read all the information from c6b file.
		Set m_file and m_filename but not m_name.
		\param file Complete file path without placeholder
		\param clearName If true m_name will be cleared.
		\param withData If true also the data set willbe read with update of checkBits.
		\note This function will throw an exception in case of error while reading given file.
	*/
	void readInfo(const QFileInfo& file, bool withData, bool clearName);

	/*! Return true if all climate components for shortwave solar radiation are valid.*/
	bool hasShortwave() const;

	/*! Return true if all climate components for longwave radiation are valid.*/
	bool hasLongwave() const;

	/*! Return true if all climate components for driving rain are valid.*/
	bool hasRain() const;

	/*! Return true if temperature is valid.*/
	bool hasTemperature() const;

	/*! Return true if relative humidity is valid.*/
	bool hasRelHum() const;

	/*! Set all data to default.*/
	void clear();

	/*! Return true if the class instance is not empty. It doesn't check if the given climate file is valid*/
	bool isEmpty() const { return m_filename.isEmpty(); }



	/*! Filename of the c6b file without path and extension.*/
	QString		m_name;
	/*! Filename of the c6b file without path.*/
	QString		m_filename;
	/*! Defines where the file is located. */
	FileLocationType	m_fileLocationType;
	/*! List of category strings which will be created from directory structure.*/
	QStringList	m_categories;
	/*! Complete description of c6b file.*/
	QFileInfo	m_file;
	/*! City. */
	QString		m_city;
	/*! Country. */
	QString		m_country;
	/*! Source. */
	QString		m_source;
	/*! Source. */
	QString		m_comment;
	/*! Store description of time behaviour (one year cyclic or not).*/
	QString		m_timeBehaviour;
	/*! Longitude in [deg] */
	double		m_longitudeInDegree;
	/*! Latitude in [deg] */
	double		m_latitudeInDegree;
	/*! Elevation/altitude in [m]. */
	double		m_elevation;
	/*! UTC - Time zone (-12..12) */
	int			m_timeZone;
	/*! Array with check bits for validity of climate components.*/
	std::array<unsigned int, CCM::ClimateDataLoader::NumClimateComponents> m_checkBits;
	/*! Climate data loader for caching data. */
	CCM::ClimateDataLoader m_loader;
};

#endif // SVClimateFileInfoH
