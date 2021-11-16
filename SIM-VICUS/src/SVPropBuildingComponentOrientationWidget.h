#ifndef SVPropBuildingComponentOrientationWidgetH
#define SVPropBuildingComponentOrientationWidgetH

#include <QWidget>

namespace Ui {
	class SVPropBuildingComponentOrientationWidget;
}

namespace VICUS {
	class ComponentInstance;
}

/*! Widget shows orientation of assigned components.

	Content of the table depends on ComponentInstances. For each
	CI the orientation of the surface is shown.
*/
class SVPropBuildingComponentOrientationWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropBuildingComponentOrientationWidget(QWidget *parent = nullptr);
	~SVPropBuildingComponentOrientationWidget();

	/*! Updates user interface. */
	void updateUi();

private slots:
	void on_checkBoxShowAllComponentOrientations_toggled(bool checked);
	void on_pushButtonAlignComponentToSideA_clicked();
	void on_pushButtonAlignComponentToSideB_clicked();
	void on_comboBoxComponentSelection_currentIndexChanged(int);

private:
	/*! This function toggles side assignments in component instances for selected components. */
	void alignSelectedComponents(bool toSideA);

	Ui::SVPropBuildingComponentOrientationWidget *m_ui;

	/*! List of component instances with surfaces, that are currently visible and selected. */
	std::vector<const VICUS::ComponentInstance*> m_selectedComponentInstances;
};

#endif // SVPropBuildingComponentOrientationWidgetH
