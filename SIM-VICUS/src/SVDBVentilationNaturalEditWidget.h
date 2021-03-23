#ifndef SVDBVentilationNaturalEditWidgetH
#define SVDBVentilationNaturalEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBVentilationNaturalEditWidget;
}

namespace VICUS {
	class VentilationNatural;
}

class SVDBVentilationNaturalTableModel;
class SVDatabase;

class SVDBVentilationNaturalEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBVentilationNaturalEditWidget(QWidget *parent = nullptr);
	~SVDBVentilationNaturalEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Sets up the widget for a Schedule with a given ID. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_pushButtonColor_colorChanged();

	void on_lineEditAirChangeRate_editingFinished();
	void on_toolButtonSelectSchedule_clicked();


private:

	/*! Set up the modified variable of the model to true. */
	void modelModify();

	Ui::SVDBVentilationNaturalEditWidget				*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase											*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBVentilationNaturalTableModel					*m_dbModel;

	/*! Pointer to currently edited natural ventilation model.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no model to edit.
	*/
	VICUS::VentilationNatural						*m_current;
};

#endif // SVDBVentilationNaturalEditWidgetH
