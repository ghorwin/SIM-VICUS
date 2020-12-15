#ifndef SVDBMaterialsEditWidgetH
#define SVDBMaterialsEditWidgetH

#include <QWidget>


namespace VICUS {
class Material;
}

class SVSettings;

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

	/*! Update widget with this. */
	void update(int id);

	/*! Edit widget with this. */
	void edit();


protected:
	void closeEvent(QCloseEvent *event) override;


private slots:
	void editingFinishedSuccessfully();

	void on_toolButtonAdd_clicked();

	void on_toolButtonCopy_clicked();

	void onMaterialSelected(int);

	void on_lineEditConductivity_editingFinished();

private:




	Ui::SVDBMaterialsEditWidget *m_ui;

	std::map<unsigned int, VICUS::Material>	*m_dbMat;

	/*! Conductivity in W/mK. */
	double						m_conductivity;

	/*! Conductivity in W/mK. */
	double						m_density;

	/*! Conductivity in W/mK. */
	double						m_specHeat;

	int							m_actualId;


};

#endif // SVDBMaterialsEditWidgetH
