#ifndef SVDBComponentEditWidgetH
#define SVDBComponentEditWidgetH

#include <QWidget>



namespace VICUS {
class Component;
}

class SVDBComponentTableModel;
class SVDatabase;

namespace Ui {
class SVDBComponentEditWidget;
}

/*! Edit widget for components.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBComponentEditWidget : public QWidget {
	Q_OBJECT

public:

	explicit SVDBComponentEditWidget(QWidget *parent = nullptr);
	~SVDBComponentEditWidget();

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVDBComponentTableModel * dbModel);

	/*! Update widget with this. */
	void updateInput(int id);

signals:
	/*! Emitted, whenever model data has been changed that is shown in the table
		and may have an effect on sorting.
	*/
	void tableDataChanged();


private slots:
	void on_lineEditName_editingFinished();
	void on_comboBoxComponentType_currentIndexChanged(int index);

private:
	Ui::SVDBComponentEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase					*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBComponentTableModel		*m_dbModel;

	/*! Pointer to currently edited component.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no component to edit.
	*/
	VICUS::Component				*m_current;
};

#endif // SVDBComponentEditWidgetH
