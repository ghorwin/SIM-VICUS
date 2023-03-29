#ifndef SVPROPBUILDINGSTRUCTURALUNITWIDGET_H
#define SVPROPBUILDINGSTRUCTURALUNITWIDGET_H

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

	void on_tableWidgetStructuralUnit_itemSelectionChanged();

	void onModified(int modificationType, ModificationInfo * /*data*/);

	void on_pushButtonAddStructuralUnit_clicked();

	void on_pushButtonAssignStructuralUnit_clicked();

	void on_tableWidgetStructuralUnit_cellDoubleClicked(/*int row, int column*/);

	void on_pushButtonRemoveStructuralUnit_clicked();

	void on_pushButtonRemoveAssignment_clicked();

private:
	const VICUS::StructuralUnit * currentlySelectedStructuralUnit() const;

	Ui::SVPropBuildingStructuralUnitWidget *m_ui;
};

#endif // SVPROPBUILDINGSTRUCTURALUNITWIDGET_H
