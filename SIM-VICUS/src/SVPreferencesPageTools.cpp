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

#include "SVPreferencesPageTools.h"
#include "ui_SVPreferencesPageTools.h"

#include "SVSettings.h"

SVPreferencesPageTools::SVPreferencesPageTools(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPreferencesPageTools)
{
	m_ui->setupUi(this);

#ifdef Q_OS_WIN
	m_ui->filepathPostProc->setup("", true, true, tr("Executables (*.exe);;All files (*.*)"), SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->filepathTextEditor->setup("", true, true, tr("Executables (*.exe);;All files (*.*)"), SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->filePath7Zip->setup("", true, true, tr("Executables (*.exe);;All files (*.*)"), SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->filePathCCMEditor->setup("", true, true, tr("Executables (*.exe);;All files (*.*)"), SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->filePathDWDConverter->setup("", true, true, tr("Executables (*.exe);;All files (*.*)"), SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->filePathMasterSim->setup("", true, true, tr("All files (*)"), SVSettings::instance().m_dontUseNativeDialogs);
#else
	m_ui->filepathPostProc->setup("", true, true, tr("All files (*)"), SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->filepathTextEditor->setup("", true, true, tr("All files (*)"), SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->filePath7Zip->setup("", true, true, tr("All files (*)"), SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->filePathCCMEditor->setup("", true, true, tr("All files (*)"), SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->filePathDWDConverter->setup("", true, true, tr("All files (*)"), SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->filePathMasterSim->setup("", true, true, tr("All files (*)"), SVSettings::instance().m_dontUseNativeDialogs);
#endif


	// no auto-detect on Linux (programs should be in search path)
#ifdef Q_OS_LINUX
	m_ui->pushButtonAutoDetectPP2->setVisible(false);
	m_ui->pushButtonAutoDetect7zip->setVisible(false);
	m_ui->pushButtonAutoDetectCCM->setVisible(false);
	m_ui->pushButtonAutoDetectDWD->setVisible(false);
	m_ui->pushButtonAutoDetectTextEditor->setVisible(false);
	m_ui->pushButtonAutoDetectMasterSim->setVisible(false);
#endif

	connect(m_ui->filePathDWDConverter, &QtExt::BrowseFilenameWidget::editingFinished,
			this, &SVPreferencesPageTools::on_filePathDWDConverter_editingFinished);
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
	m_ui->filePathDWDConverter->setFilename(s.m_DWDConverterExecutable);
	m_ui->filePathMasterSim->setFilename(s.m_masterSimExecutable);


	// TODO : Refactor, let SVSettings auto-detect tool paths and just take them here

	// auto-detect postproc 2 and CCM
	QString postProc2Path, ccmPath, masterSimPath, dwdConverterPath;

#ifdef Q_OS_WIN

	if (!QFileInfo(s.m_postProcExecutable).exists())
		on_pushButtonAutoDetectPP2_clicked();

	if (!QFileInfo(s.m_textEditorExecutable).exists())
		on_pushButtonAutoDetectTextEditor_clicked();

	if (!QFileInfo(s.m_CCMEditorExecutable).exists())
		on_pushButtonAutoDetectCCM_clicked();

	if (!QFileInfo(s.m_DWDConverterExecutable).exists())
		on_pushButtonAutoDetectDWD_clicked();

	if (!QFileInfo(s.m_masterSimExecutable).exists())
		on_pushButtonAutoDetectMasterSim_clicked();




#else
	// no D5 PostProc on linux/mac
#if defined(Q_OS_MAC)
	// search for version in Applications dir
	const QString POST_PROC_INSTALL_LOC = "/Applications/PostProcApp.app";
	if (QFileInfo(POST_PROC_INSTALL_LOC).exists())
		postProc2Path = POST_PROC_INSTALL_LOC;

	// mastersim path on max?
#else
	// search for version in bin directory
	const QString POST_PROC_INSTALL_LOC = s.m_installDir + "/PostProcApp";
	if (QFileInfo(POST_PROC_INSTALL_LOC).exists())
		postProc2Path = POST_PROC_INSTALL_LOC;

	// search for version in bin directory
	const QString MASTER_SIM_INSTALL_LOC = "MasterSimulator";
	if (QFileInfo("usr/bin/" + MASTER_SIM_INSTALL_LOC).exists())
		masterSimPath = MASTER_SIM_INSTALL_LOC;

#endif

#endif // Q_OS_WIN

}




void SVPreferencesPageTools::on_filepathPostProc_editingFinished() {
	SVSettings & s = SVSettings::instance();
	s.m_postProcExecutable = m_ui->filepathPostProc->filename();
}


void SVPreferencesPageTools::on_filepathPostProc_returnPressed() {
	on_filepathPostProc_editingFinished();
}


void SVPreferencesPageTools::on_pushButtonAutoDetectPP2_clicked() {
	// TODO : refactor! let SVSetting auto-detect the tool and if it exists, set it in the user-interface
	SVSettings & s = SVSettings::instance();
	QString postProc2Path;
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

#elif defined(Q_OS_MAC)

	// search for version in Applications dir
	const QString POST_PROC_INSTALL_LOC = "/Applications/PostProcApp.app";
	if (QFileInfo(POST_PROC_INSTALL_LOC).exists())
		postProc2Path = POST_PROC_INSTALL_LOC;
#endif

	// if no file found, we set an empty string
	m_ui->filepathPostProc->setFilename(postProc2Path);
	s.m_postProcExecutable = postProc2Path;
}


void SVPreferencesPageTools::on_filepathTextEditor_editingFinished() {
	SVSettings & s = SVSettings::instance();
	s.m_textEditorExecutable = m_ui->filepathTextEditor->filename();
}


void SVPreferencesPageTools::on_filepathTextEditor_returnPressed() {
	on_filepathTextEditor_editingFinished();
}



void SVPreferencesPageTools::on_pushButtonAutoDetectTextEditor_clicked() {
	// TODO : refactor! let SVSetting auto-detect the tool and if it exists, set it in the user-interface
	SVSettings & s = SVSettings::instance();
	QString toolPath;
#ifdef Q_OS_WIN
	// search for installed x64 version of Notepad++
	QString x64ToolPath = "c:\\Program Files\\Notepad++\\notepad++.exe";
	if (QFileInfo(x64ToolPath).exists())
		toolPath = x64ToolPath;
	if (toolPath.isEmpty()) {
		// search for installed x86 version
		QString x86ToolPath = "c:\\Program Files (x86)\\Notepad++\\notepad++.exe";
		if (QFileInfo(x86ToolPath).exists())
			toolPath = x86ToolPath;
	}
#elif defined(Q_OS_MAC)

	// text editor on Mac?
#endif

	// if no file found, we set an empty string
	m_ui->filepathTextEditor->setFilename(toolPath);
	s.m_textEditorExecutable = toolPath;
}

void SVPreferencesPageTools::on_filePathMasterSim_editingFinished() {
	SVSettings & s = SVSettings::instance();
	s.m_masterSimExecutable = m_ui->filePathMasterSim->filename();
}


void SVPreferencesPageTools::on_filePathMasterSim_returnPressed() {
	on_filePathMasterSim_editingFinished();
}


void SVPreferencesPageTools::on_pushButtonAutoDetectMasterSim_clicked()
{
	SVSettings & s = SVSettings::instance();
	QString toolPath;
#ifdef Q_OS_WIN
	// search for installed x64 version of MasterSim
	const char * const MASTER_SIM_INSTALL_LOC = "c:\\Program Files\\IBK\\MasterSimulator 0.%1\\MasterSimulator.exe";
	for (int i=9; i>=0; --i) {
		QString masterSimLoc = QString(MASTER_SIM_INSTALL_LOC).arg(i);
		if (QFileInfo(masterSimLoc).exists()) {
			toolPath = masterSimLoc;
			break;
		}
	}
	if (toolPath.isEmpty()) {
		// search for installed x86 version
		const char * const MASTER_SIM_INSTALL_LOC2 = "c:\\Program Files (x86)\\IBK\\MasterSimulator 0.%1\\MasterSimulator.exe";
		for (int i=9; i>=0; --i) {
			QString masterSimLoc = QString(MASTER_SIM_INSTALL_LOC2).arg(i);
			if (QFileInfo(masterSimLoc).exists()) {
				toolPath = masterSimLoc;
				break;
			}
		}
	}
#elif defined(Q_OS_MAC)

	// master simulator on Mac?
#endif

	// if no file found, we set an empty string
	m_ui->filePathMasterSim->setFilename(toolPath);
	s.m_masterSimExecutable = toolPath;
}



void SVPreferencesPageTools::on_pushButtonAutoDetectDWD_clicked() {
	SVSettings & s = SVSettings::instance();
	QString toolPath;
#ifdef Q_OS_WIN
	const char * const DWD_CONVERTER_INSTALL_LOC = "c:\\Program Files\\IBK\\DWDWeatherDataConverter 1.%1\\DWDWeatherDataConverter.exe";
	for (int i=9; i>=0; --i) {
		QString dwdConvLoc = QString(DWD_CONVERTER_INSTALL_LOC).arg(i);
		if (QFileInfo(dwdConvLoc).exists()) {
			toolPath = dwdConvLoc;
			break;
		}
	}
	if (toolPath.isEmpty()) {
		// search for installed x86 version
		const char * const DWD_CONVERTER_INSTALL_LOC = "c:\\Program Files (x86)\\IBK\\DWDWeatherDataConverter 1.%1\\DWDWeatherDataConverter.exe";
		for (int i=9; i>=0; --i) {
			QString dwdConvLoc = QString(DWD_CONVERTER_INSTALL_LOC).arg(i);
			if (QFileInfo(dwdConvLoc).exists()) {
				toolPath = dwdConvLoc;
				break;
			}
		}
	}
#elif defined(Q_OS_MAC)

	// master simulator on Mac?
#endif

	// if no file found, we set an empty string
	m_ui->filePathDWDConverter->setFilename(toolPath);
	s.m_DWDConverterExecutable = toolPath;
}


void SVPreferencesPageTools::on_pushButtonAutoDetectCCM_clicked() {
	SVSettings & s = SVSettings::instance();
	QString toolPath;
#ifdef Q_OS_WIN
	const char * const CCM_INSTALL_LOC = "c:\\Program Files\\IBK\\CCMEditor 1.%1\\CCMEditor.exe";
	for (int i=9; i>=0; --i) {
		QString ccmLoc = QString(CCM_INSTALL_LOC).arg(i);
		if (QFileInfo(ccmLoc).exists()) {
			toolPath = ccmLoc;
			break;
		}
	}

#elif defined(Q_OS_MAC)

	// master simulator on Mac?
#endif

	// if no file found, we set an empty string
	m_ui->filePathCCMEditor->setFilename(toolPath);
	s.m_CCMEditorExecutable = toolPath;
}


void SVPreferencesPageTools::on_filePathDWDConverter_editingFinished() {
	SVSettings & s = SVSettings::instance();
	s.m_DWDConverterExecutable = m_ui->filePathDWDConverter->filename();
}


