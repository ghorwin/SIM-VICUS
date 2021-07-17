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

#ifndef SVSimulationLocationOptionsH
#define SVSimulationLocationOptionsH

#include <QWidget>

namespace Ui {
	class SVSimulationLocationOptions;
}

namespace NANDRAD {
	class Location;
}

class SVClimateDataTableModel;
class SVClimateDataSortFilterProxyModel;

#include "SVClimateFileInfo.h"


/*! Widget with settings related to location. */
class SVSimulationLocationOptions : public QWidget {
	Q_OBJECT

public:
	SVSimulationLocationOptions(QWidget *parent, NANDRAD::Location & location);
	~SVSimulationLocationOptions();

	/*! Updates user interface with properties from the project data structure.
		This function is called whenever the dialog is first shown.
	*/
	void updateUi();

private slots:
	void on_radioButtonFromDB_toggled(bool checked);
	void on_lineEditTextFilter_editingFinished();
	void on_lineEditTextFilter_textChanged(const QString &arg1);
	void onCurrentIndexChanged(const QModelIndex &current, const QModelIndex & /*previous*/);
	void on_checkBoxCustomLocation_toggled(bool checked);
	void on_filepathClimateDataFile_editingFinished();

	void on_radioButtonUserPathAbsolute_toggled(bool checked);

private:
	void updateLocationInfo(const SVClimateFileInfo * dataPtr, bool databaseFile);
	void updateUserClimateFileInfo();

	void storeCustomLocationInputs();

	Ui::SVSimulationLocationOptions		*m_ui;
	SVClimateDataTableModel				*m_climateDataModel = nullptr;
	SVClimateDataSortFilterProxyModel	*m_filterModel		= nullptr;
	NANDRAD::Location					*m_location;

	/*! Climate data file info, used to store the data when user climate was selected. */
	SVClimateFileInfo					m_userClimateFile;
};

#endif // SVSimulationLocationOptionsH
