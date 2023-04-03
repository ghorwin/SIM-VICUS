#ifndef SVPropBuildingStructuralUnitWidgetH
#define SVPropBuildingStructuralUnitWidgetH

#include <QWidget>

namespace Ui {
class SVPropBuildingStructuralUnitWidget;
}


namespace VICUS {
class Room;
class StructuralUnit;
}

class ModificationInfo;

class SVPropBuildingStructuralUnitWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVPropBuildingStructuralUnitWidget(QWidget *parent = nullptr);
	~SVPropBuildingStructuralUnitWidget();

	/*! Updates user interface. */
	void updateUi();

private slots:
	/*! Triggered when user switches structural unit in table. */
	void on_tableWidgetStructuralUnit_itemSelectionChanged();

	void on_pushButtonAddStructuralUnit_clicked();

	/*! Assigns structural unit from table to selected rooms. */
	void on_pushButtonAssignStructuralUnit_clicked();

	/*! Triggers the edit dialog. */
	void on_tableWidgetStructuralUnit_cellDoubleClicked(/*int row, int column*/);

	/*! Deletes the selected structural unit from the project. */
	void on_pushButtonRemoveStructuralUnit_clicked();

	/*! Removes the all assignments from the selected rooms. */
	void on_pushButtonRemoveAssignment_clicked();

private:
	const VICUS::StructuralUnit * currentlySelectedStructuralUnit() const;

	Ui::SVPropBuildingStructuralUnitWidget *m_ui;
};

#endif // SVPropBuildingStructuralUnitWidgetH
