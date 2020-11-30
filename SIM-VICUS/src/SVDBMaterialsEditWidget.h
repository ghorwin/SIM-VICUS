#ifndef SVDBMaterialsEditWidgetH
#define SVDBMaterialsEditWidgetH

#include <QWidget>

namespace Ui {
class SVDBMaterialsEditWidget;
}

/*! This is a non-modal dialog/widget and stays open parallel to the UI.
	When a new project is created/read or any other action is performed
	that changes the content of the materials DB externally, this
	widget has to be closed.

	Call edit() to show this widget, since this updates the widget to the
	current setting's data.
*/
class SVDBMaterialsEditWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVDBMaterialsEditWidget(QWidget *parent = nullptr);
	~SVDBMaterialsEditWidget();

	/*! Start widget with this. */
	void edit();

private slots:
	void on_toolButtonAdd_clicked();

	void on_toolButtonCopy_clicked();

private:
	Ui::SVDBMaterialsEditWidget *m_ui;
};

#endif // SVDBMaterialsEditWidgetH
