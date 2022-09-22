#ifndef SVPropAddWindowWidgetH
#define SVPropAddWindowWidgetH

#include <QWidget>

#include "Vic3DNewSubSurfaceObject.h"

namespace Ui {
class SVPropAddWindowWidget;
}

class QSpinBox;

class ModificationInfo;

/*! Property widget for adding sub-surfaces. */
class SVPropAddWindowWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropAddWindowWidget(QWidget *parent = nullptr);
	~SVPropAddWindowWidget();

	/*! Call this function before switching to this property widget.
		This will setup the "new subsurface preview" object and fix any invalid
		inputs with defaults.
	*/
	void setup();

	/*! Updates the list of sub-surface components to select.
		This function is called from DB edit widgets when either sub-surface or regular component instances
		have been edited.
	*/
	void updateSubSurfaceComponentList();

public slots:

	void onModified(int modificationType, ModificationInfo * /*data*/);

private slots:
	void onSpinBoxValueChanged(int arg1);

	void on_lineEditWindowWidth_editingFinishedSuccessfully();

	void on_lineEditWindowHeight_editingFinishedSuccessfully();

	void on_lineEditWindowSillHeight_editingFinishedSuccessfully();

	void on_lineEditWindowDistance_editingFinishedSuccessfully();

	void on_lineEditWindowOffset_editingFinishedSuccessfully();

	void on_lineEditWindowPercentage_editingFinishedSuccessfully();

	void on_spinBoxMaxHoleCount_valueChanged(int arg1);

	void on_radioButtonSubSurfaceTypeWindow_toggled(bool checked);

	void on_tabWidgetWindow_currentChanged(int index);


	void on_pushButtonCreate_clicked();

	void on_pushButtonChangeLocalOrigin_clicked();

private:
	/*! Updates widget to current project state. */
	void updateUi();

	/*! Collects input data from widget and updates data in window geometry object. */
	void updateGeometryObject();

	Ui::SVPropAddWindowWidget							*m_ui;

	/*! Caches currently selected surfaces. Updated in updateUi(). */
	std::vector<const VICUS::Surface*>					m_currentSelection;

	Vic3D::NewSubSurfaceObject::WindowComputationData	m_windowInputData;

	QSpinBox											*m_prioritySpinBoxes[4];
};

#endif // SVPropAddWindowWidgetH
