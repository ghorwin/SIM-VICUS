#ifndef SVDBNETWORKCONTROLLEREDITWIDGET_H
#define SVDBNETWORKCONTROLLEREDITWIDGET_H

#include "SVAbstractDatabaseEditWidget.h"

class SVDBNetworkControllerTableModel;

namespace VICUS {
	class NetworkController;
}

namespace Ui {
class SVDBNetworkControllerEditWidget;
}

class SVDBNetworkControllerEditWidget :  public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:

	explicit SVDBNetworkControllerEditWidget(QWidget *parent = nullptr);
	~SVDBNetworkControllerEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();

	void on_lineEditSetpoint_editingFinished();

	void on_lineEditKp_editingFinished();

	void on_lineEditKi_editingFinished();

	void on_radioButtonSchedule_clicked(bool checked);

	void on_radioButtonFixedSetPoint_clicked(bool checked);

	void on_toolButtonSchedule_clicked();

	void on_comboBoxProperty_activated(int index);

	void on_comboBoxControllerType_activated(int index);

	void on_pushButtonColor_clicked();

	void on_lineEditMaxControllerResultValue_editingFinished();

private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	void toggleLineEdits();

	Ui::SVDBNetworkControllerEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBNetworkControllerTableModel		*m_dbModel;

	/*! Pointer to currently edited controller.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no component to edit.
	*/
	VICUS::NetworkController			*m_current;
};

#endif // SVDBNETWORKCONTROLLEREDITWIDGET_H
