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

#include <NANDRAD_Location.h>

#include "SVClimateFileInfo.h"

#include <qwt_plot.h>

namespace Ui {
	class SVSimulationLocationOptions;
}

class SVClimateDataTableModel;
class SVClimateDataSortFilterProxyModel;
class ModificationInfo;
class QwtPlotZoomer;


/*! Widget with settings related to location. */
class SVSimulationLocationOptions : public QWidget {
	Q_OBJECT

public:
	SVSimulationLocationOptions(QWidget *parent);
	~SVSimulationLocationOptions();

	/*! Updates user interface with properties from the project data structure.
		This function is called whenever the dialog is first shown.
	*/
	void updateUi(bool updatePlots);

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified(int modificationType, ModificationInfo *);

	void onScreenChanged(const QScreen *screen);

private slots:
	void on_lineEditTextFilter_editingFinished();
	void on_lineEditTextFilter_textChanged(const QString &arg1);
	void on_filepathClimateDataFile_editingFinished();

	void on_radioButtonUserPathAbsolute_toggled(bool checked);

	void on_lineEditLatitude_editingFinishedSuccessfully();

	void on_lineEditLongitude_editingFinishedSuccessfully();

	void on_comboBoxTimeZone_activated(int);

	void on_comboboxAlbedoEditingFinishedSuccessfully();

	void on_tableViewClimateFiles_clicked(const QModelIndex &index);

	void on_radioButtonCustomFilePath_toggled(bool checked);

	void on_pushButtonOpenDWDConverter_clicked();

private:

	void updateLocationInfoText(const SVClimateFileInfo * dataPtr);

	void readUserClimateFileInfo();

	void modifyClimateFileAndLocation(const SVClimateFileInfo * climateInfoPtr);

	void modifyLocationFromLineEdits();

	void updatePlots(const SVClimateFileInfo * climateInfoPtr);

	void formatPlots(const QDateTime & start, const QDateTime &end, bool init);

	void formatQwtPlot(bool init, QwtPlot & plot, QDateTime start, QDateTime end, QString title, QString leftYAxisTitle, double yLeftMin, double yLeftMax, unsigned int yNumSteps=5,
					   bool hasRightAxis = false, QString rightYAxisTitle = "", double yRightMin = 0, double yRightMax = 100, double yRightStepSize = 0);

	void minMaxValuesInPlot(const QwtPlot & plot, double & minY, double & maxY);

	void resizeEvent(QResizeEvent * event) override;

	void setPlotHeights(int totalHeight);

	Ui::SVSimulationLocationOptions		*m_ui;
	SVClimateDataTableModel				*m_climateDataModel = nullptr;
	SVClimateDataSortFilterProxyModel	*m_filterModel		= nullptr;

	/*! Climate data file info, used to store the data when user climate was selected. */
	SVClimateFileInfo					m_userClimateFileInfo;

	/*! Holds climate data */
	CCM::ClimateDataLoader				m_loader;

	std::vector<double>					m_climateTimePoints;

	QwtPlotZoomer						*m_zoomerRelHum = nullptr;
	QwtPlotZoomer						*m_zoomerTemp = nullptr;
	QwtPlotZoomer						*m_zoomerWind = nullptr;
	QwtPlotZoomer						*m_zoomerRadShortWave = nullptr;
	QwtPlotZoomer						*m_zoomerRadLongWave = nullptr;

};

#endif // SVSimulationLocationOptionsH
