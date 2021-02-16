#ifndef SVFloorManagerWidgetH
#define SVFloorManagerWidgetH

#include <QWidget>

namespace Ui {
class SVFloorManagerWidget;
}

namespace VICUS {
	class Building;
	class BuildingLevel;
}

class ModificationInfo;

/*! A widget to edit buildings/building levels and associate rooms with building levels. */
class SVFloorManagerWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVFloorManagerWidget(QWidget *parent = nullptr);
	~SVFloorManagerWidget();


public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

protected:
	void showEvent(QShowEvent * event) override;
	void resizeEvent(QResizeEvent * event) override;

private slots:
	void on_treeWidget_itemSelectionChanged();

	void on_lineEditBuildingName_editingFinished();

	void on_lineEditLevelName_editingFinished();

	void on_pushButtonAddBuilding_clicked();

	void on_pushButtonAddLevel_clicked();

	void on_pushButtonRemoveBuilding_clicked();

private:
	Ui::SVFloorManagerWidget	*m_ui;

	/*! Pointer to currently selected building, updated in on_treeWidget_itemSelectionChanged(). */
	const VICUS::Building		*m_currentBuilding;
	/*! Pointer to currently selected building level, updated in on_treeWidget_itemSelectionChanged(). */
	const VICUS::BuildingLevel	*m_currentBuildingLevel;


};

#endif // SVFloorManagerWidgetH
