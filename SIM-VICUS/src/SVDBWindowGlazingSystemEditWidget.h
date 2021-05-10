#ifndef SVDBWindowGlazingSystemEditWidgetH
#define SVDBWindowGlazingSystemEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBWindowGlazingSystemEditWidget;
}

class SVDBWindowGlazingSystemTableModel;
class SVDatabase;
class QwtPlotCurve;

namespace VICUS {
	class WindowGlazingSystem;
}


/*! Edit widget for window glazing systems.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBWindowGlazingSystemEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:



	explicit SVDBWindowGlazingSystemEditWidget(QWidget *parent = nullptr);
	~SVDBWindowGlazingSystemEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;


private slots:
	void on_lineEditName_editingFinished();

	void on_lineEditSHGC_editingFinished();
	void on_pushButtonWindowColor_colorChanged();

	void on_lineEditUValue_editingFinished();

	void on_comboBoxType_currentIndexChanged(int index);


private:

	/*! Set up the modified variable of the model to true. */
	void modelModify();

	Ui::SVDBWindowGlazingSystemEditWidget 	*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase								*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBWindowGlazingSystemTableModel		*m_dbModel;

	/*! Pointer to currently edited window glazing system.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no window glazing system to edit.
	*/
	VICUS::WindowGlazingSystem				*m_current;

	/*! Diagram curve for SHGC-plot. */
	QwtPlotCurve							*m_shgcCurve;
};

#endif // SVDBWindowGlazingSystemEditWidgetH

