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

#ifndef SVDBMaterialEditWidgetH
#define SVDBMaterialEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBMaterialEditWidget;
}

class SVDBMaterialTableModel;
class SVDatabase;
class QTableWidgetItem;
class QLineEdit;

namespace VICUS {
	class Material;
}

/*! Edit widget for materials.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBMaterialEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

	enum ColCatA {
		CA_A1,
		CA_A2,
		CA_A3,
		CA_A4,
		CA_A5,
		NUM_CA
	};

	enum ColCatB {
		CB_B1,
		CB_B2,
		CB_B3,
		CB_B4,
		CB_B5,
		CB_B6,
		CB_B7,
		NUM_CB
	};

	enum ColCatC {
		CC_C1,
		CC_C2,
		CC_C3,
		CC_C4,
		NUM_CC
	};

	enum ColCatD {
		CD_D,
		NUM_CD
	};

public:
	explicit SVDBMaterialEditWidget(QWidget *parent = nullptr);
	~SVDBMaterialEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;

	/*! Activates a tab by index. */
	void setCurrentTabIndex(int idx);

private slots:
	void on_lineEditName_editingFinished();
	void on_lineEditDataSource_editingFinished();
	void on_lineEditManufacturer_editingFinished();
	void on_lineEditNotes_editingFinished();
	void on_lineEditConductivity_editingFinished();
	void on_lineEditDensity_editingFinished();
	void on_lineEditSpecHeatCapacity_editingFinished();

	void on_comboBoxCategory_currentIndexChanged(int index);

	void on_pushButtonColor_colorChanged();

	void on_toolButtonSelectCatA_clicked();

	void on_toolButtonSelectCatB_clicked();

	void on_toolButtonSelectCatC_clicked();

	void on_toolButtonSelectCatD_clicked();

	void on_pushButton_clicked();

private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	/*! Updates all category specific data and sets all subcategories such as A1,A2,... .*/
	void updateCategory(unsigned int idCategory, VICUS::Material &mat, const VICUS::LcaSettings::LcaCategory category, QLineEdit *lineEdit);

	/*! Updates tab states of all categories such A,B,C,D to see whether all sub-categories are set properly. */
	void updateCategoryValidity(unsigned int idCategory, const VICUS::LcaSettings::LcaCategory category);

	Ui::SVDBMaterialEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase					*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBMaterialTableModel		*m_dbModel;

	/*! Pointer to currently edited material.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no material to edit.
	*/
	VICUS::Material				*m_current;

	/*! Pointer to LCA Settings .*/
	const VICUS::LcaSettings	*m_lcaSettings;

	/*! Map with all table widget items. Key is module enum.
	*/
	std::map<VICUS::EpdModuleDataset::Module, QTableWidgetItem*> m_moduleToTableItem;

	QWidgetList					m_catACheckboxes;
	QWidgetList					m_catBCheckboxes;
	QWidgetList					m_catCCheckboxes;
	QWidgetList					m_catDCheckboxes;
};

#endif // SVDBMaterialEditWidgetH
