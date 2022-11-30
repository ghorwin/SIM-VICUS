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

	void on_pushButtonSelectPipelineFile_clicked();

	void on_pushButtonImportPipeline_clicked();

	void on_pushButtonSelectSubStationFile_clicked();

	void on_pushButtonImportSubStation_clicked();

	void on_pushButtonSelectAvailablePipes_clicked();

	void on_pushButtonSelectDefaultPipe_clicked();

private:

	//stores the selected default pipe for pipe network import
	const VICUS::NetworkPipe * m_defaultPipe = nullptr;

	/*! hier die dokumentation */
	std::vector<unsigned int> m_availablePipes;

	// TODO Anton: enums haben bei uns immer das Bennungsschema "Akronym_Name" wobei Akronym den enum Namen abkürzt. In dem Fall: IT_Pipeline und IT_Substation (IT für ImportType)
	enum ImportType {
		Pipeline,
		SubStation,
	};

	/*! Toggles enable state of line edit, combobox */
	void toggleExistingOrNewNetwork(bool readExisting);

	/*! Reads either a geoJSON or csv file of either pipeline data or sub stations (based on given importType) and adds that to given network */
	void readNetworkData(const IBK::Path &fname, VICUS::Network &network, unsigned int nextId, ImportType importType) const;

	QString uniqueName(const QString & name);

	Ui::SVNetworkImportDialog		*m_ui;

	VICUS::Network					m_network;

	QMap<QString, unsigned>			m_existingNetworksMap;

	QString							m_lastPipelineFilePath;
	QString							m_lastSubstationFilePath;
};

#endif // SVNetworkImportDialogH
