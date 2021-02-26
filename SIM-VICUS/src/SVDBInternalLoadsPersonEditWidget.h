#ifndef SVDBInternalLoadsPersonEditWidgetH
#define SVDBInternalLoadsPersonEditWidgetH

#include <QWidget>

namespace Ui {
class SVDBInternalLoadsPersonEditWidget;
}

namespace VICUS {
class InternalLoad;
}

class SVDBInternalLoadTableModel;
class SVDatabase;

class SVDBInternalLoadsPersonEditWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVDBInternalLoadsPersonEditWidget(QWidget *parent = nullptr);
	~SVDBInternalLoadsPersonEditWidget();

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVDBInternalLoadTableModel * dbModel);

	/*! Sets up the widget for a Schedule with a given ID. */
	void updateInput(int id);

private slots:
	void on_lineEditName_editingFinished();

	void on_comboBoxPersonMethod_currentIndexChanged(int index);

	void on_lineEditPersonCount_editingFinished();

	void on_lineEditConvectiveFactor_editingFinished();

	void on_pushButtonPersonColor_colorChanged();

private:
	Ui::SVDBInternalLoadsPersonEditWidget	*m_ui;


	/*! Pointer to currently edited internal loads model.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no model to edit.
	*/
	VICUS::InternalLoad						*m_current;

	/*! Cached pointer to database object. */
	SVDatabase								*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBInternalLoadTableModel				*m_dbModel;
};

#endif // SVDBInternalLoadsPersonEditWidgetH
