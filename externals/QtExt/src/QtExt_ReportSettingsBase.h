#ifndef ReportSettingsBaseH
#define ReportSettingsBaseH

#include <set>
#include <QFont>

#include <QtExt_ReportUtilities.h>

class QSettings;

namespace QtExt {


class ReportSettingsBase
{
public:
	enum BaseItems {
		ReportHeader = 0,
		ReportFooter = 1
	};

	ReportSettingsBase();

	/*! Reads the report settings from the registry/system config.*/
	virtual void readSettings(QSettings & settings);

	/*! Stores the reprot settings in the registry/system config.*/
	virtual void writeSettings(QSettings & settings) const;

	/*! Return true if the given frame id is in the internal frame list.*/
	bool hasFrameId(int id);

	/*! Should return the number of possible frame types additionally to header and footer.
		Function must be reimplemented in derived class.
	*/
	virtual int frameTypeNumber() const {
		return 0;
	}

	/*! Set with frames that can be shown in the report.*/
	std::set<int>			m_frames;

	/*! Saves state of checkbox page numbers*/
	bool					m_showPageNumbers;

	/*! Defines the global font family to be used for diagram and report.*/
	QString					m_fontFamily;

	/*! Default font for report.*/
	QFont					m_defaultFont;

	/*! Default font size in points for report.*/
	int						m_defaultFontPointSize;

	/*! Text format used for main header.*/
	QtExt::FormatTags		m_mainHeaderTextFormat;

	static const int		m_frameIdStart = 2;

};

} // end namespace

#endif // ReportSettingsBaseH