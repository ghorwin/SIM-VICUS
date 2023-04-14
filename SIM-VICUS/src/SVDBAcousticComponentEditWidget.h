#ifndef SVDBAcoustucComponentEditWidgetH
#define SVDBAcoustucComponentEditWidgetH

#include <QWidget>
#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBAcousticComponentEditWidget;
}

class SVDBAcousticComponentTableModel;
class SVDatabase;

namespace VICUS {
	class AcousticComponent;
}


/*! Edit widget for components.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBAcousticComponentEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:

	explicit SVDBAcousticComponentEditWidget(QWidget *parent = nullptr);
	~SVDBAcousticComponentEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;

private slots:
	void on_pushButtonColor_colorChanged();

	void on_lineEditName_editingFinished();

	void on_lineEditISV_editingFinishedSuccessfully();

	void on_lineEditASRV_editingFinishedSuccessfully();

private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	Ui::SVDBAcousticComponentEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBAcousticComponentTableModel		*m_dbModel;

	/*! Pointer to currently edited acoustic component.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no component to edit.
	*/
	VICUS::AcousticComponent			*m_current;
};

#endif // SVDBAcoustucComponentEditWidgetH
