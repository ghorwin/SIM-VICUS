#include "QtExt_ReportSettingsBase.h"

#include <QSettings>
#include <QStringList>
#include <QTextDocument>

namespace QtExt {

ReportSettingsBase::ReportSettingsBase() :
	m_showPageNumbers(true),
	#ifdef Q_OS_WIN
		m_fontFamily("Verdana"),
	#else
		m_fontFamily("SansSerif"),
	#endif
	m_defaultFontPointSize(8),
	m_mainHeaderTextFormat(QtExt::FT_Header1)
{
	m_frames.insert(ReportHeader);
	m_frames.insert(ReportFooter);

	QTextDocument dummyDoc;
	m_defaultFont = dummyDoc.defaultFont();
	m_defaultFont.setFamily(m_fontFamily);
	m_defaultFont.setPointSize(8);

	// By default header and footer is switched on
	m_frames.insert(0);
	m_frames.insert(1);
}

void ReportSettingsBase::readSettings(QSettings & settings) {
	settings.beginGroup("ReportSettings");
	QString activatedFrames = settings.value("ActivatedFrames").toString();
	if(!activatedFrames.isEmpty()) {
		QStringList lst = activatedFrames.split(" ");
		for (int i=0; i<lst.count(); ++i) {
			bool ok;
			bool activated = lst[i].toInt(&ok);
			if (ok && activated)
				m_frames.insert(i);
			else
				m_frames.erase(i);
		}
	}

	m_showPageNumbers = settings.value("CurrentPageNumbersVisible", false ).toBool();
	m_defaultFont = settings.value("ReportFont").value<QFont>();

	settings.endGroup();
}

void ReportSettingsBase::writeSettings(QSettings & settings) const {
	settings.beginGroup("ReportSettings");
	QString frameString;
	for (int i : m_frames) {
		frameString += QString("%1 ").arg(i);
	}
	settings.setValue("ActivatedFrames", frameString);

	settings.setValue("CurrentPageNumbersVisible", m_showPageNumbers );
	settings.setValue("ReportFont", m_defaultFont );

	settings.endGroup();
}

bool ReportSettingsBase::hasFrameId(int id) {
	return m_frames.find(id) != m_frames.end();
}


} // end namespace
