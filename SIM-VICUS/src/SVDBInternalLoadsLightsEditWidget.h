#ifndef SVDBInternalLoadsLightsEditWidgetH
#define SVDBInternalLoadsLightsEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBInternalLoadsLightsEditWidget;
}

namespace VICUS {
	class InternalLoad;
}

class SVDBInternalLoadsTableModel;
class SVDatabase;

class SVDBInternalLoadsLightsEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBInternalLoadsLightsEditWidget(QWidget *parent = nullptr);
	~SVDBInternalLoadsLightsEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Sets up the widget for a Schedule with a given ID. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_comboBoxMethod_currentIndexChanged(int index);
	void on_lineEditPower_editingFinished();
	void on_pushButtonColor_colorChanged();
	void on_toolButtonSelectSchedule_clicked();
	void on_lineEditConvectiveFactor_editingFinished();
	//void on_lineEditLatentFactor_editingFinished();
	//void on_lineEditLossFactor_editingFinished();

private:

	/*! Update the unit label of input method. */
	void updateLabel();

	Ui::SVDBInternalLoadsLightsEditWidget	*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase											*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBInternalLoadsTableModel							*m_dbModel;

	/*! Pointer to currently edited internal loads model.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no model to edit.
	*/
	VICUS::InternalLoad									*m_current;
};

#endif // SVDBInternalLoadsLightsEditWidgetH
