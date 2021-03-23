#ifndef SVDBZoneControlVentilationNaturalEditWidgetH
#define SVDBZoneControlVentilationNaturalEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBZoneControlVentilationNaturalEditWidget;
}

namespace VICUS {
	class ZoneControlNaturalVentilation;
}

class SVDBZoneControlVentilationNaturalTableModel;
class SVDatabase;

class SVDBZoneControlVentilationNaturalEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBZoneControlVentilationNaturalEditWidget(QWidget *parent = nullptr);
	~SVDBZoneControlVentilationNaturalEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Sets up the widget for a Schedule with a given ID. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_pushButtonColor_colorChanged();

	void on_lineEditTemperatureAirOutsideMaximum_editingFinished();
	void on_lineEditTemperatureAirOutsideMinimum_editingFinished();
	void on_lineEditTemperatureAirRoomMaximum_editingFinished();
	void on_lineEditTemperatureAirRoomMinimum_editingFinished();
	void on_lineEditTemperatureDifference_editingFinished();
	void on_lineEditWindSpeedMax_editingFinished();

private:

	/*! Set up the modified variable of the model to true. */
	void modelModify();

	Ui::SVDBZoneControlVentilationNaturalEditWidget		*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase											*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBZoneControlVentilationNaturalTableModel			*m_dbModel;

	/*! Pointer to currently edited zone control natural ventilation model.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no model to edit.
	*/
	VICUS::ZoneControlNaturalVentilation				*m_current;
};

#endif // SVDBZoneControlVentilationNaturalEditWidgetH
