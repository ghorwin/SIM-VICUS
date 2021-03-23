#ifndef SVDBZoneControlShadingEditWidgetH
#define SVDBZoneControlShadingEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBZoneControlShadingEditWidget;
}

namespace VICUS {
	class ZoneControlShading;
}

class SVDBZoneControlShadingTableModel;
class SVDatabase;

class SVDBZoneControlShadingEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBZoneControlShadingEditWidget(QWidget *parent = nullptr);
	~SVDBZoneControlShadingEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Sets up the widget for a Schedule with a given ID. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_comboBoxMethod_currentIndexChanged(int index);
	void on_pushButtonColor_colorChanged();

	void on_lineEditNorth_editingFinished();
	void on_lineEditEast_editingFinished();
	void on_lineEditWest_editingFinished();
	void on_lineEditSouth_editingFinished();
	void on_lineEditHorizontal_editingFinished();
	void on_lineEditDeadBand_editingFinished();

private:

	/*! Set up the modified variable of the model to true. */
	void modelModify();

	Ui::SVDBZoneControlShadingEditWidget				*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase											*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBZoneControlShadingTableModel					*m_dbModel;

	/*! Pointer to currently edited zone control Shading model.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no model to edit.
	*/
	VICUS::ZoneControlShading						*m_current;
};

#endif // SVDBZoneControlShadingEditWidgetH
