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

#ifndef SVNetworkImportDialogH
#define SVNetworkImportDialogH

#include <QDialog>
#include <QMap>

#include <VICUS_Network.h>

namespace Ui {
	class SVNetworkImportDialog;
}


/*! A dialog for importing pipe network data. */
class SVNetworkImportDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVNetworkImportDialog(QWidget *parent = nullptr);
	~SVNetworkImportDialog();

	/*! Always start the dialog with this function.
		\return Returns true if dialog was confirmed and data can be added to project.
	*/
	bool edit();

private slots:

	void on_radioButtonNewNetwork_clicked(bool checked);

	void on_radioButtonAddToExistingNetwork_clicked(bool checked);

	/*! Opens a dialog to select the available pipes and saves the ids in m_availablePipes*/
	void on_pushButtonSelectAvailablePipes_clicked();

	/*! Opens a dialog to select the default pipe and saves the selected id in m_defaultPipe*/
	void on_pushButtonSelectDefaultPipe_clicked();

	/*! triggers a check if the import button for pipelines should be enables*/
	void on_lineEditPipelineFileName_editingFinished();

	/*! triggers a check if the import button for substations should be enabled*/
	void on_lineEditMaxHeatingDemand_editingFinished();

	/*! triggers a check if the import button for substations should be enabled*/
	void on_lineEditSubStationFileName_editingFinished();

	void on_comboBoxNetworkSelectionBox_currentIndexChanged(int index);

private:

	/*! Imports the pipeline with the selected inputs*/
	void importPipeline();

	/*! Imports the substation with the selected inputs*/
	void importSubStations();

	//stores the selected default pipe for pipe network import
	unsigned int m_defaultPipeId = VICUS::INVALID_ID;

	/*! stores the available Pipes for the imprt that the user has selected */
	std::vector<unsigned int> m_availablePipes;

	enum ImportType {
		IT_Pipeline,
		IT_SubStation,
		NUM_IT
	};

	/*! Toggles enable state of line edit, combobox */
	void toggleExistingOrNewNetwork(bool readExisting);

	/*! reads the json contents of the filepath and stores them in obj */
	void convert2QJsonObject(const IBK::Path &filePath, QJsonObject &obj) const;

	/*! Reads either a geoJSON or csv file of either pipeline data or sub stations (based on given importType) and adds that to given network */
	void readNetworkData(const IBK::Path &fname, VICUS::Network &network, unsigned int nextId, ImportType importType) const;

	/*! reads csv-files from QGIS with multiple rows, containing "MULTILINESTRING"s and adds according nodes/edges to the network.
		Lines that share a common node (identical coordinates) are automatically connected.
	*/
	void readGridFromCSV(VICUS::Network & network, const IBK::Path &filePath, unsigned int nextId) const;

	/*! reads geoJson-file and adds according nodes/edges to the network.
		Lines that share a common node (identical coordinates) are automatically connected.
	*/
	void readGridFromGeoJson(VICUS::Network & network, const QJsonObject & jsonObj, unsigned int nextId) const;

	/*! reads csv-files from QGIS with multiple rows, containing "POINT"s and adds according nodes of type NT_BUILDING to the network.*/
	void readBuildingsFromCSV(VICUS::Network & network, const IBK::Path &filePath, const double &heatDemand, unsigned int nextId) const;

	/*! reads geoJson-file and adds according nodes of type NT_BUILDING to the network*/
	void readBuildingsFromGeoJson(VICUS::Network & network, const QJsonObject jsonObj, const double &heatDemand, unsigned int nextId) const;

	/*! checks if the file name is set and a default pipe is selected and enables/disables the import button*/
	void checkIfPipelineImportIsEnabled();

	/*! checks if the file name is set and the max heating demand is greater 0 and enables/disables the import button*/
	void checkIfSubStationImportIsEnabled();

	/*! updates combobox etc. */
	void updateUi();

	QString uniqueName(const QString & name);

	Ui::SVNetworkImportDialog		*m_ui;

	VICUS::Network					m_network;

	QString							m_lastPipelineFilePath;
	QString							m_lastSubstationFilePath;
};

#endif // SVNetworkImportDialogH
