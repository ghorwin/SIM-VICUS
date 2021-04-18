#ifndef SVDBInfiltrationEditWidgetH
#define SVDBInfiltrationEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBInfiltrationEditWidget;
}

namespace VICUS {
	class Infiltration;
}

class SVDBInfiltrationTableModel;
class SVDatabase;

class SVDBInfiltrationEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBInfiltrationEditWidget(QWidget *parent = nullptr);
	~SVDBInfiltrationEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Sets up the widget for a Schedule with a given ID. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	//void on_comboBoxControlValue_currentIndexChanged(int index);
	void on_comboBoxMethod_currentIndexChanged(int index);
	void on_pushButtonColor_colorChanged();

	void on_lineEditShieldCoefficient_editingFinished();
	void on_lineEditAirChangeRate_editingFinished();


private:

	/*! Set up the modified variable of the model to true. */
	void modelModify();

	Ui::SVDBInfiltrationEditWidget				*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase									*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBInfiltrationTableModel					*m_dbModel;

	/*! Pointer to currently edited zone control infiltration model.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no model to edit.
	*/
	VICUS::Infiltration							*m_current;
};

#endif // SVDBZoneControlThermostatEditWidgetH
