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

#include "SVClimateDataTableModel.h"

#include <QDir>

#include <QtExt_Directories.h>

#include <IBK_Path.h>
#include <IBK_messages.h>

#include <VICUS_Constants.h>

#include "SVSettings.h"
#include "SVConstants.h" // for the custom roles

SVClimateDataTableModel::SVClimateDataTableModel(QObject * parent) :
	QAbstractTableModel(parent)
{
}


int SVClimateDataTableModel::rowCount(const QModelIndex &) const {
	return m_climateFiles.size();
}


int SVClimateDataTableModel::columnCount(const QModelIndex &) const {
	return NUM_C;
}


QVariant SVClimateDataTableModel::data(const QModelIndex & index, int role) const {
	// get the requested climate data file
	Q_ASSERT(index.row() < m_climateFiles.size());
	const SVClimateFileInfo & f = m_climateFiles[index.row()];

	switch (role) {
		case Qt::DisplayRole : {
			switch ((Columns)index.column()) {
				case SVClimateDataTableModel::C_Region:
					if (f.m_categories.size() >= 1)
						return f.m_categories[0];
					else
						return "---";
				case SVClimateDataTableModel::C_Country:
					return f.m_country;
				case SVClimateDataTableModel::C_Sub:
					if (f.m_categories.size() >= 3)
						return f.m_categories[2];
					else
						return "---";
				case SVClimateDataTableModel::C_City:
					return f.m_city;
				case SVClimateDataTableModel::C_Longitude:
					return QString("%L1").arg(f.m_longitudeInDegree);
				case SVClimateDataTableModel::C_Latitude:
					return QString("%L1").arg(f.m_latitudeInDegree);
				case SVClimateDataTableModel::C_TimeZone:
					if (f.m_timeZone >= 0)
						return QString("UTC + %1").arg(f.m_timeZone);
					else
						return QString("UTC - %1").arg(-f.m_timeZone);
				case SVClimateDataTableModel::C_Elevation:
					return QString("%L1").arg(f.m_elevation);
				case SVClimateDataTableModel::NUM_C:
					break;
			}
		} break;

		case Qt::ToolTipRole :
			return f.m_filename;

		case Role_BuiltIn :
			return f.m_builtIn;

		case Role_FileName :
			return f.m_name; // just the basename

		case Role_FilePath :
			return f.m_filename; // relative path to DB including path placeholders

		case Role_AbsoluteFilePath :
			return f.m_absoluteFilePath;

		case Role_Value : {
			switch ((Columns)index.column()) {
				case SVClimateDataTableModel::C_Longitude:
					return f.m_longitudeInDegree;
				case SVClimateDataTableModel::C_Latitude:
					return f.m_latitudeInDegree;
				case SVClimateDataTableModel::C_TimeZone:
					return f.m_timeZone;
				case SVClimateDataTableModel::C_Elevation:
					return f.m_elevation;
				default:;
			}
		} break;

		case Role_RawPointer :
			return QVariant::fromValue<void*>((void*)&f);
	}
	return QVariant();
}


QVariant SVClimateDataTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole : {
			switch ((Columns)section) {
				case SVClimateDataTableModel::C_Region:
					return tr("Continent/Region");
				case SVClimateDataTableModel::C_Country:
					return tr("Country");
				case SVClimateDataTableModel::C_Sub:
					return tr("State/Province/...");
				case SVClimateDataTableModel::C_City:
					return tr("City/Station");
				case SVClimateDataTableModel::C_Longitude:
					return tr("Longitude [deg]");
				case SVClimateDataTableModel::C_Latitude:
					return tr("Latitude [deg]");
				case SVClimateDataTableModel::C_TimeZone:
					return tr("Time zone");
				case SVClimateDataTableModel::C_Elevation:
					return tr("Elevation [m]");
				default:;
			}
		} break;

		case Qt::TextAlignmentRole :
			return Qt::AlignCenter;

		case Qt::FontRole : {
			QFont f;
			f.setPointSizeF(f.pointSizeF()*0.8);
			f.setBold(true);
			return f;
		}

	}
	return QVariant();
}


const SVClimateFileInfo * SVClimateDataTableModel::infoForFilename(const QString & climateFilePath) const {
	for (const SVClimateFileInfo & info : m_climateFiles) {
		if (info.m_filename == climateFilePath)
			return &info;
	}
	return nullptr;
}


void SVClimateDataTableModel::updateClimateFileList() {
	beginResetModel();

	FUNCID(SVSettings::updateClimateFileList);

	m_climateFiles.clear();

	QStringList ccFileExtensions = QStringList() << "c6b" << "epw" << "wac";
	QString ccDir(QtExt::Directories::databasesDir() + "/DB_climate");
	QDir builtInDir(ccDir);
	QStringList defaultFiles;
	if (builtInDir.exists()) {
		SVSettings::recursiveSearch(builtInDir, defaultFiles, ccFileExtensions);
	}
	QString uccDir(QtExt::Directories::userDataDir() + "/DB_climate");
	QDir userCCDir(uccDir);
	QStringList userFiles;
	if (userCCDir.exists()) {
		SVSettings::recursiveSearch(userCCDir, userFiles, ccFileExtensions);
	}

	IBK::Path ccPath(ccDir.toStdString());
	for (const QString& file : defaultFiles) {
		// file is now the full file path to built-in database files
		try {
			SVClimateFileInfo item;
			item.readInfo(ccDir, file, false, true);
			IBK::Path itemPath(file.toStdString());
			itemPath = itemPath.relativePath(ccPath);
			QStringList categories = QString::fromStdString(itemPath.str()).split('/');
			categories.removeLast();
			item.m_categories = categories;
			m_climateFiles.push_back(item);
		}
		catch(IBK::Exception& ex) {
			ex.writeMsgStackToError();
			IBK::IBK_Message(IBK::FormatString("Error in climate data set '%1'.\n")
							 .arg(file.toStdString()), IBK::MSG_WARNING, FUNC_ID);
		}
	}

	IBK::Path uccPath(uccDir.toStdString());
	for (const QString& file : userFiles) {
		// file is now the full file path to user database files
		try {
			SVClimateFileInfo item;
			item.readInfo(ccDir, file, false, false);
			IBK::Path itemPath(file.toStdString());
			itemPath = itemPath.relativePath(uccPath);
			QStringList categories = QString::fromStdString(itemPath.str()).split('/');
			categories.removeLast();
			item.m_categories = categories;
			m_climateFiles.push_back(item);
		}
		catch(IBK::Exception& ex) {
			IBK::IBK_Message(IBK::FormatString("Error in climate data set '%1'.\n%2")
							 .arg(file.toStdString())
							 .arg(ex.what()), IBK::MSG_WARNING, FUNC_ID);
		}
	}

	endResetModel();
}

