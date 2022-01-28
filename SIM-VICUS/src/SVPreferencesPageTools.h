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

#ifndef SVPreferencesPageToolsH
#define SVPreferencesPageToolsH

#include <QWidget>

namespace Ui {
	class SVPreferencesPageTools;
}

/*! The configuration page with external tool settings. */
class SVPreferencesPageTools : public QWidget {
	Q_OBJECT
	Q_DISABLE_COPY(SVPreferencesPageTools)
public:
	/*! Default constructor. */
	explicit SVPreferencesPageTools(QWidget *parent = nullptr);
	/*! Destructor. */
	~SVPreferencesPageTools() override;

	/*! Updates the user interface with values in Settings object.*/
	void updateUi();

private slots:
	void on_filepathPostProc_editingFinished();
	void on_filepathPostProc_returnPressed();

	void on_pushButtonAutoDetectPP2_clicked();

	void on_filepathTextEditor_editingFinished();
	void on_filepathTextEditor_returnPressed();

	void on_pushButtonAutoDetectTextEditor_clicked();

	void on_pushButtonAutoDetectMasterSim_clicked();

	void on_filePathMasterSim_editingFinished();
	void on_filePathMasterSim_returnPressed();
private:
	Ui::SVPreferencesPageTools *m_ui;
};


#endif // SVPreferencesPageToolsH
