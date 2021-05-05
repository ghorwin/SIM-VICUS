#ifndef SVDBWindowEditWidgetH
#define SVDBWindowEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBWindowEditWidget;
}

class SVDBWindowTableModel;
class SVDatabase;

namespace VICUS {
	class Window;
}


/*! Edit widget for components (window).

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBWindowEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:



	explicit SVDBWindowEditWidget(QWidget *parent = nullptr);
	~SVDBWindowEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;


private slots:
	void on_lineEditName_editingFinished();

	void on_pushButtonWindowColor_colorChanged();

	void on_toolButtonSelectGlazingSystemName_clicked();

	void on_toolButtonSelectFrameMaterial_clicked();

	void on_toolButtonSelectDividerMaterial_clicked();

	void on_lineEditFrameMaterialThickness_editingFinished();

	void on_lineEditDividerMaterialThickness_editingFinished();

	void on_lineEditDividerInput_editingFinished();

	void on_lineEditFrameInput_editingFinished();

	void on_comboBoxFrameMethod_currentIndexChanged(int index);

	void on_comboBoxDividerMethod_currentIndexChanged(int index);

private:

	/*! Set up the modified variable of the model to true. */
	void modelModify();

	Ui::SVDBWindowEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase					*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBWindowTableModel		*m_dbModel;

	/*! Pointer to currently edited component (window).
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no component (window) to edit.
	*/
	VICUS::Window				*m_current;
};

#endif // SVDBWindowEditWidgetH

