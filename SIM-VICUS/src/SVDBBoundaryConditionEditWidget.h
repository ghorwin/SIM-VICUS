#ifndef SVDBBoundaryConditionEditWidgetH
#define SVDBBoundaryConditionEditWidgetH

#include <QWidget>


namespace VICUS {
class BoundaryCondition;
}

class SVDBBoundaryConditionTableModel;
class SVDatabase;


namespace Ui {
class SVDBBoundaryConditionEditWidget;
}
/*! Edit widget for boundary condition.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBBoundaryConditionEditWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVDBBoundaryConditionEditWidget(QWidget *parent = nullptr);
	~SVDBBoundaryConditionEditWidget();

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVDBBoundaryConditionTableModel * dbModel);

	/*! Update widget with this. */
	void updateInput(int id);

signals:
	/*! Emitted, whenever model data has been changed that is shown in the table
		and may have an effect on sorting.
	*/
	void tableDataChanged();


private slots:
	void on_lineEditName_editingFinished();
	void on_lineEditHeatTransferCoefficient_editingFinished();
	void on_lineEditSolarAbsorption_editingFinished();
	void on_lineEditThermalAbsorption_editingFinished();


private:
	Ui::SVDBBoundaryConditionEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBBoundaryConditionTableModel		*m_dbModel;

	/*! Pointer to currently edited boundary condition.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no material to edit.
	*/
	VICUS::BoundaryCondition			*m_current;
};

#endif // SVDBBoundaryConditionEditWidgetH
