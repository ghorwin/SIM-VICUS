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
	m_mainHeaderTextFormat(QtExt::FT_Header1),
	m_outerTableFrameWidth(1.5),
	m_innerTableFrameWidth(0.7)
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
	m_frames.clear();
	if(!activatedFrames.isEmpty()) {
		QStringList lst = activatedFrames.split(" ", QString::SkipEmptyParts);
		for (int i=0; i<lst.count(); ++i) {
			bool ok;
			int activated = lst[i].toInt(&ok);
			if (ok)
				m_frames.insert(activated);
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
