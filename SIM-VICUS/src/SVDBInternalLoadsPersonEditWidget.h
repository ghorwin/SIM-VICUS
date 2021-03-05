#ifndef SVDBInternalLoadsPersonEditWidgetH
#define SVDBInternalLoadsPersonEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBInternalLoadsPersonEditWidget;
}

namespace VICUS {
	class InternalLoad;
}

class SVDBInternalLoadsTableModel;
class SVDatabase;

class SVDBInternalLoadsPersonEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBInternalLoadsPersonEditWidget(QWidget *parent = nullptr);
	~SVDBInternalLoadsPersonEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Sets up the widget for a Schedule with a given ID. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_comboBoxPersonMethod_currentIndexChanged(int index);
	void on_lineEditPersonCount_editingFinished();
	void on_lineEditConvectiveFactor_editingFinished();
	void on_pushButtonPersonColor_colorChanged();
	void on_toolButtonSelectOccupancy_clicked();
	void on_toolButtonSelectActivity_clicked();

private:
	Ui::SVDBInternalLoadsPersonEditWidget	*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase								*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBInternalLoadsTableModel				*m_dbModel;

	/*! Pointer to currently edited internal loads model.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no model to edit.
	*/
	VICUS::InternalLoad						*m_current;
};

#endif // SVDBInternalLoadsPersonEditWidgetH
