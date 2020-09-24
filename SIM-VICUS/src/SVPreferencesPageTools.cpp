#include "SVPreferencesPageTools.h"
#include "ui_SVPreferencesPageTools.h"

#include "SVSettings.h"

SVPreferencesPageTools::SVPreferencesPageTools(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPreferencesPageTools)
{
	m_ui->setupUi(this);

#ifdef Q_OS_WIN
	m_ui->filepathTextEditor->setup("", true, true, tr("Executables (*.exe);;All files (*.*)"));
	m_ui->filePath7Zip->setup("", true, true, tr("Executables (*.exe);;All files (*.*)"));
	m_ui->filePathCCMEditor->setup("", true, true, tr("Executables (*.exe);;All files (*.*)"));
#else
	m_ui->filepathTextEditor->setup("", true, true, tr("All files (*.*)"));
	m_ui->filePath7Zip->setup("", true, true, tr("All files (*.*)"));
	m_ui->filePathCCMEditor->setup("", true, true, tr("All files (*.*)"));
#endif

	m_ui->lineEditTerminal->setVisible(false);
	m_ui->checkBoxUseTerminal->setVisible(false);
	m_ui->labelUseTerminal->setVisible(false);
}


SVPreferencesPageTools::~SVPreferencesPageTools() {
	delete m_ui;
}


void SVPreferencesPageTools::updateUi() {
	SVSettings & s = SVSettings::instance();
	// transfer data to Ui
	m_ui->filepathTextEditor->setFilename(s.m_textEditorExecutable);
	m_ui->filepathPostProc->setFilename(s.m_postProcExecutable);
	m_ui->filePath7Zip->setFilename(s.m_7zExecutable);
	m_ui->filePathCCMEditor->setFilename(s.m_CCMEditorExecutable);

	// auto-detect postproc 2 and CCM
	QString postProc2Path, ccmPath;

#ifdef Q_OS_WIN
	// search for installed x64 version of PostProc2
	const char * const POST_PROC_INSTALL_LOC = "c:\\Program Files\\IBK\\PostProc 2.%1\\PostProcApp.exe";
	for (int i=9; i>=0; --i) {
		QString postProcLoc = QString(POST_PROC_INSTALL_LOC).arg(i);
		if (QFileInfo(postProcLoc).exists()) {
			postProc2Path = postProcLoc;
			break;
		}
	}
	if (postProc2Path.isEmpty()) {
		// search for installed x86 version
		const char * const POST_PROC_INSTALL_LOC2 = "c:\\Program Files (x86)\\IBK\\PostProc 2.%1\\PostProcApp.exe";
		for (int i=9; i>=0; --i) {
			QString postProcLoc = QString(POST_PROC_INSTALL_LOC2).arg(i);
			if (QFileInfo(postProcLoc).exists()) {
				postProc2Path = postProcLoc;
				break;
			}
		}
	}

	QFileInfo ccmEditor = s.m_CCMEditorExecutable;
	if (!ccmEditor.exists()) {
		// search for installed x86 version
		const char * const CCM_INSTALL_LOC2 = "c:\\Program Files (x86)\\IBK\\CCMEditor 1.%1\\CCMEditor.exe";
		for (int i=9; i>=0; --i) {
			QString ccmLoc = QString(CCM_INSTALL_LOC2).arg(i);
			if (QFileInfo(ccmLoc).exists()) {
				ccmPath = ccmLoc;
				break;
			}
		}

		m_ui->filePathCCMEditor->setFilename(ccmPath);
	}

#else
	// no D5 PostProc on linux/mac
#if defined(Q_OS_MAC)
	// search for version in Applications dir
	const QString D6_POST_PROC_INSTALL_LOC = "/Applications/PostProcApp.app";
	if (QFileInfo(D6_POST_PROC_INSTALL_LOC).exists())
		postProc2Path = D6_POST_PROC_INSTALL_LOC;
#else
	// search for version in D6 binary directory
	const QString D6_POST_PROC_INSTALL_LOC = s.m_installDir + "/PostProcApp";
	if (QFileInfo(D6_POST_PROC_INSTALL_LOC).exists())
		postProc2Path = D6_POST_PROC_INSTALL_LOC;
#endif

#endif // Q_OS_WIN

}


bool SVPreferencesPageTools::storeConfig() {
	// no checks necessary
	SVSettings & s = SVSettings::instance();
	s.m_textEditorExecutable = m_ui->filepathTextEditor->filename();
#if defined(Q_OS_MAC)
	s.m_postProcExecutable = m_ui->filepathPostProc->filename();
	if (s.m_postProcExecutable.endsWith(".app"))
		s.m_postProcExecutable += "/Contents/MacOS/PostProcApp";
#else
	s.m_postProcExecutable = m_ui->filepathPostProc->filename();
#endif
	s.m_7zExecutable = m_ui->filePath7Zip->filename();
	s.m_CCMEditorExecutable = m_ui->filePathCCMEditor->filename();

	return true;
}


