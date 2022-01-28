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

#ifndef SVClimateDataTableModelH
#define SVClimateDataTableModelH

#include <QAbstractTableModel>

#include "SVClimateFileInfo.h"

/*! A table model that provides a table/list of available climate data files.
	Use the Role_FileName to get just the filename. Use Role_FilePath to get the
	file path including a database placeholder (this should be used to store the location in the project file).
	Finally, use Role_AbsoluteFilePath to get the absolute file path, in case the climate data file needs to be
	read for diagram display.
*/
class SVClimateDataTableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	/*! Different columns provided by the model.
		Country and City are stored as meta data in the climate data containers.
		Region and sub are from the file hierarchy:
		DB_climate/<region or continent>/<country>/<sub>/climateFile.c6b

		<sub> is optional. <region> can be "generic"
	*/
	enum Columns {
		C_Region,
		C_Country,
		C_Sub,
		C_City,
		C_Longitude,
		C_Latitude,
		C_TimeZone,
		C_Elevation,
		NUM_C
	};

	SVClimateDataTableModel(QObject * parent);

	// QAbstractItemModel interface

	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;
	QVariant data(const QModelIndex & index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	/*! Returns access to the raw climate data files used by the model. */
	QList<SVClimateFileInfo> climateFiles() const {	return m_climateFiles; }

	/*! Looks up the climate file path (potentially with 'Database' or 'User Database' placeholders and returns
		the matching climate file info. If not found, a nullptr is returned.
	*/
	const SVClimateFileInfo * infoForFilename(const QString & climateFilePath) const;

	/*! Parses the climate data base directories and refreshs the list of climate data files.
		This also resets the model.
	*/
	void updateClimateFileList();

private:
	/*! Available climate data files (updated in updateClimateFileList()). */
	QList<SVClimateFileInfo>	m_climateFiles;

};

#endif // SVClimateDataTableModelH
