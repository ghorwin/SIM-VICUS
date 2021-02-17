#ifndef SVFloorManagerWidgetH
#define SVFloorManagerWidgetH

#include <QWidget>

namespace Ui {
class SVPropFloorManagerWidget;
}

namespace VICUS {
	class Building;
	class BuildingLevel;
}

class ModificationInfo;
class QTreeWidgetItem;

/*! A widget to edit buildings/building levels and associate rooms with building levels. */
class SVPropFloorManagerWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropFloorManagerWidget(QWidget *parent = nullptr);
	~SVPropFloorManagerWidget();


public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

private slots:
	void on_treeWidget_itemSelectionChanged();
	void on_lineEditBuildingName_editingFinished();
	void on_lineEditLevelName_editingFinished();
	void on_lineEditLevel_editingFinishedSuccessfully();
	void on_lineEditHeight_editingFinishedSuccessfully();
	void on_pushButtonAddBuilding_clicked();
	void on_pushButtonAddLevel_clicked();
	void on_pushButtonRemoveBuilding_clicked();
	void on_pushButtonRemoveLevel_clicked();

	void on_pushButtonAssignRooms_clicked();

	void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

private:
	Ui::SVPropFloorManagerWidget	*m_ui;

	/*! Pointer to currently selected building, updated in on_treeWidget_itemSelectionChanged(). */
	const VICUS::Building		*m_currentBuilding;
	/*! Pointer to currently selected building level, updated in on_treeWidget_itemSelectionChanged(). */
	const VICUS::BuildingLevel	*m_currentBuildingLevel;


};

#endif // SVFloorManagerWidgetH
