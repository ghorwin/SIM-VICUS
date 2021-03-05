#ifndef SVDBMaterialEditWidgetH
#define SVDBMaterialEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBMaterialEditWidget;
}

class SVDBMaterialTableModel;
class SVDatabase;

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
	void on_pushButtonOpaqueMaterialColor_colorChanged();

private:

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
};

#endif // SVDBMaterialEditWidgetH
