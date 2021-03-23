#ifndef SVDBZoneControlThermostatEditWidgetH
#define SVDBZoneControlThermostatEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBZoneControlThermostatEditWidget;
}

namespace VICUS {
	class ZoneControlThermostat;
}

class SVDBZoneControlThermostatTableModel;
class SVDatabase;

class SVDBZoneControlThermostatEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBZoneControlThermostatEditWidget(QWidget *parent = nullptr);
	~SVDBZoneControlThermostatEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Sets up the widget for a Schedule with a given ID. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_comboBoxControlValue_currentIndexChanged(int index);
	void on_pushButtonColor_colorChanged();

	void on_lineEditToleranceHeating_editingFinished();
	void on_lineEditToleranceCooling_editingFinished();
	void on_toolButtonSelectHeatingSchedule_clicked();
	void on_toolButtonSelectCoolingSchedule_clicked();

private:

	/*! Set up the modified variable of the model to true. */
	void modelModify();

	Ui::SVDBZoneControlThermostatEditWidget				*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase											*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBZoneControlThermostatTableModel					*m_dbModel;

	/*! Pointer to currently edited zone control thermostat model.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no model to edit.
	*/
	VICUS::ZoneControlThermostat						*m_current;
};

#endif // SVDBZoneControlThermostatEditWidgetH
