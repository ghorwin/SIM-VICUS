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

#include "SVPostProcBindings.h"

#include <QDir>
#include <QFileDialog>

#include <fstream>

#include <VICUS_Project.h>

#include <IBK_StringUtils.h>
#include <IBK_QuantityManager.h>
#include <IBK_Path.h>
#include <IBK_FileUtils.h>

#include "SVSettings.h"
#include "SVProjectHandler.h"

// Forward declarations of templates. Definitions are below at end of file
extern const char * const DEFAULT_SESSION_TEMPLATE;
extern const char * const COLORMAP_MAPPER_TEMPLATE;
extern const char * const TEMPERATURE_COLOR_MAP;
extern const char * const MOISTURE_COLOR_MAP;



IBK::Path SVPostProcBindings::defaultSessionFilePath(const QString & projectFile) {
	QDir projectDir = QFileInfo(projectFile).absoluteDir();
	QFileInfoList files = projectDir.entryInfoList(QStringList() << "*.p2", QDir::Files);
	if(files.empty()) {
		IBK::Path parentPath = IBK::Path(projectDir.absolutePath().trimmed().toStdString());
		// if path is valid, use filename as session name, otherwise use generic name (i.e. root path selected)
		IBK::Path sessionFile;
		if (parentPath.isValid())
			sessionFile = parentPath.filename() + ".p2";
		else
			sessionFile = "analysis.p2";
		IBK::Path sessionFilePath = parentPath / sessionFile;
		return sessionFilePath;
	}
	else {
		if(files.size() == 1) {
			return IBK::Path(files.front().absoluteFilePath().trimmed().toStdString());
		}
		else {
			return IBK::Path();
		}
	}
}


void SVPostProcBindings::generateDefaultSessionFile(const QString & projectFile) {
	IBK::Path sessionFile = defaultSessionFilePath(projectFile);

	std::ofstream strm;
	IBK::open_ofstream(strm, sessionFile);

	// configure mappers
	std::string mappers;

//	const VICUS::Project & prj = project();

	/// \todo Andreas: populate "mappers" string with predefined mapper configurations

	// replace placeholders
	IBK::Path projectFilePath = IBK::Path(projectFile.trimmed().toStdString() );
	IBK::Path subdir = projectFilePath.filename().withoutExtension();
	std::string content = DEFAULT_SESSION_TEMPLATE;
	content = IBK::replace_string(content, "${MAPPERS}", mappers);
	content = IBK::replace_string(content, "${PROJECT_RESULT_BASE_DIR}", subdir.str());
	strm << content << std::endl;
}




const char * const DEFAULT_SESSION_TEMPLATE =
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<PostProc xmlns=\"http://www.bauklimatik-dresden.de\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:IBK=\"http://www.bauklimatik-dresden.de/IBK\" xsi:schemaLocation=\"http://www.bauklimatik-dresden.de PostProc.xsd\" fileVersion=\"1.0\">\n"
		"	<Directories>\n"
		"		<Directory>\n"
		"			<Path>.</Path>\n"
		"			<SubDir Color=\"#d02000\" Checked=\"1\">${PROJECT_RESULT_BASE_DIR}</SubDir>\n"
		"			<ExpandedSubDir>.</ExpandedSubDir>\n"
		"		</Directory>\n"
		"	</Directories>\n"
		"	<!--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~-->\n"
		"\n"
		"	<!--All mappers.-->\n"
		"	<Mappers>\n"
		"		${MAPPERS}\n"
		"	</Mappers>\n"
		"\n"
		"		<!--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~-->\n"
		"		\n"
		"</PostProc>\n";

