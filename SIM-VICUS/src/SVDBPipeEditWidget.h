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

#ifndef SVDBPipeEditWidgetH
#define SVDBPipeEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBPipeEditWidget;
}

namespace VICUS {
	class NetworkPipe;
}

class SVDBPipeTableModel;
class SVDatabase;

/*! Edit widget for network pipes. */
class SVDBPipeEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBPipeEditWidget(QWidget *parent = nullptr);
	~SVDBPipeEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_lineEditOuterDiameter_editingFinishedSuccessfully();
	void on_lineEditWallThickness_editingFinishedSuccessfully();
	void on_lineEditWallLambda_editingFinishedSuccessfully();
	void on_lineEditWallRoughness_editingFinishedSuccessfully();
	void on_lineEditInsulationThickness_editingFinishedSuccessfully();
	void on_lineEditInsulationLambda_editingFinishedSuccessfully();
	void on_pushButtonColor_colorChanged();
	void on_lineEditCategory_editingFinished();

	void on_lineEditWallHeatCapacity_editingFinished();

	void on_lineEditWallDensity_editingFinished();

private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	/*! Updates the pipe name from entered data, if auto naming is enabled. */
	void updateNameFromData();

	Ui::SVDBPipeEditWidget	*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase				*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBPipeTableModel		*m_dbModel;

	/*! Pointer to currently edited pipe.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no material to edit.
	*/
	VICUS::NetworkPipe		*m_current;

	/*! For newly created pipes we set this to true and generate the name automatically based on
		entered pipe dimensions. As soon as the user has entered a custom name, we set this to
		false so that the name is no longer updated.
	*/
	bool					m_autoNameGeneration = true;
};


#endif // SVDBPipeEditWidgetH
