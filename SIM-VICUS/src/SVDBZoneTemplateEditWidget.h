#ifndef SVDBZoneTemplateEditWidgetH
#define SVDBZoneTemplateEditWidgetH

#include <QWidget>

namespace Ui {
	class SVDBZoneTemplateEditWidget;
}

namespace VICUS {
	class ZoneTemplate;
}

class SVDBZoneTemplateTreeModel;
class SVDatabase;

/*! Edit widget for zone template.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBZoneTemplateEditWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVDBZoneTemplateEditWidget(QWidget *parent = nullptr);
	~SVDBZoneTemplateEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVDBZoneTemplateTreeModel * dbModel);

	/*! Update widget with this. */
	void updateInput(int id, int subTemplateId, int subTemplateType);

private slots:
	void on_lineEditName_editingFinished();
	void on_pushButtonColor_colorChanged();

	void on_toolButtonSelectSubComponent_clicked();

	void on_toolButtonRemoveSubComponent_clicked();

	void on_pushButtonAddSubTemplate_clicked();

private:
	Ui::SVDBZoneTemplateEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBZoneTemplateTreeModel			*m_dbModel;

	/*! Pointer to currently edited element.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is nothing to edit.
	*/
	VICUS::ZoneTemplate			*m_current;
};

#endif // SVDBZoneTemplateEditWidgetH
