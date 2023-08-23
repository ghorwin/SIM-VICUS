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

#ifndef SVDBZoneTemplateEditWidgetH
#define SVDBZoneTemplateEditWidgetH

#include <QWidget>
#include <VICUS_ZoneTemplate.h>

namespace Ui {
	class SVDBZoneTemplateEditWidget;
}

//namespace VICUS {
//	class ZoneTemplate;
//}

class SVDBZoneTemplateTreeModel;
class SVDatabase;

/*! Edit widget for zone template.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBZoneTemplateEditWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVDBZoneTemplateEditWidget(QWidget *parent = nullptr);
	~SVDBZoneTemplateEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVDBZoneTemplateTreeModel * dbModel);

	/*! Update widget with this. */
	void updateInput(int id, int subTemplateId, VICUS::ZoneTemplate::SubTemplateType subTemplateType);

public slots:
	void on_toolButtonRemoveSubComponent_clicked();

	void on_toolButtonSelectSubComponent_clicked();

signals:
	/*! Emitted when the action in the widget shall cause the tree view to change selection.
		In case of removal of a sub-template, the subTemplateType matches ZoneTemplate::NUM_ST and
		in this case the top-level ZoneTemplate node shall be selected.
		The selection change in the tree must cause a call to updateInput() afterwards, even if
		the same item was re-selected.
	*/
	void selectSubTemplate(unsigned int id, VICUS::ZoneTemplate::SubTemplateType subTemplateType);

private slots:
	void on_lineEditName_editingFinished();
	void on_pushButtonColor_colorChanged();

	void on_pushButtonAddPersonLoad_clicked();
	void on_pushButtonAddElectricLoad_clicked();
	void on_pushButtonAddLightLoad_clicked();

	void on_pushButtonAddInfiltration_clicked();

	void on_pushButtonAddVentilationNatural_clicked();

	void on_pushButtonAddThermostat_clicked();
	void on_pushButtonAddIdealHeatingCooling_clicked();


	void on_pushButtonAddShading_clicked();

	void on_pushButtonAddVentilationNaturalControl_clicked();

private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	/*! Refresh ui buttons and content. */
	void refreshUi();

	Ui::SVDBZoneTemplateEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBZoneTemplateTreeModel			*m_dbModel;

	/*! Pointer to currently edited element.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is nothing to edit.
	*/
	VICUS::ZoneTemplate					*m_current;

	VICUS::ZoneTemplate::SubTemplateType	m_currentSubTemplateType;
};

#endif // SVDBZoneTemplateEditWidgetH
