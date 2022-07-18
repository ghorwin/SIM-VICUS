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

#ifndef SVPropAddGeometryH
#define SVPropAddGeometryH

#include <QWidget>

//#include <IBKMK_Vector3D.h>

//#include <VICUS_Surface.h>

//#include <Vic3DTransform3D.h>
//#include <Vic3DCoordinateSystemObject.h>

namespace Ui {
	class SVPropAddGeometry;
}

class SVProjectHandler;
class ModificationInfo;


/*! This widget is shown when the scene is put into geometry "addition" mode.
	This widget only shows several buttons that trigger individual add operations.
*/
class SVPropAddGeometry : public QWidget {
	Q_OBJECT
public:
	explicit SVPropAddGeometry(QWidget *parent = nullptr);
	~SVPropAddGeometry() override;

public slots:

	/*! Connected to SVProjectHandler::modified().
		Enables/disables button states based on current selection.
	*/
	void onModified(int modificationType, ModificationInfo * );

private slots:
	void on_pushButtonAddPolygon_clicked();
	void on_pushButtonAddRect_clicked();
	void on_pushButtonAddZone_clicked();
	void on_pushButtonAddRoof_clicked();
	void on_pushButtonAddWindow_clicked();

	void on_pushButtonAddPipeline_clicked();

	void on_pushButtonAddSubStation_clicked();

private:
	/*! Updates the property widget regarding to all geometry data.
		This function is called whenever the selection has changed, and when surface geometry (of selected surfaces)
		has changed.

		This function switches also between AddGeometry and EditGeometry mode, when first selection is made or
		everything is deselected.
	*/
	void updateUi();

	/*! Pointer to UI */
	Ui::SVPropAddGeometry				*m_ui;
};

#endif // SVPropAddGeometryH
