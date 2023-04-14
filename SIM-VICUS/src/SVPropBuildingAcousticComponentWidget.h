#ifndef SVPropBuildingAcousticComponentWidgetH
#define SVPropBuildingAcousticComponentWidgetH

#include <QWidget>

#include <VICUS_Constants.h>

namespace Ui {
class SVPropBuildingAcousticComponentWidget;
}

namespace VICUS {
	class Surface;
	class AcousticComponent;
}

class SVPropBuildingAcousticComponentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SVPropBuildingAcousticComponentWidget(QWidget *parent = nullptr);
    ~SVPropBuildingAcousticComponentWidget();

	/*! Updates user interface. */
	void updateUi();


private slots:
	/*! Triggered when user switches acoustic component in table. */
	void on_tableWidgetAcousticComponents_itemSelectionChanged();
	/*! Triggers the openEditAcousticComponentDialog function */
	void on_pushButtonEditAcousticComponents_clicked();
	/*! Triggers the openEditAcousticComponentDialog function */
	void on_tableWidgetAcousticComponents_cellDoubleClicked(/*int row, int column*/);
	/*! Action to swap all occurances of currently selected acoustic component with newly selected acoustic component. */
	void on_pushButtonExchangeAcousticComponents_clicked();
	/*! The user selects an acoustic component from the database and this will be assigned to all selected surfaces,
		possibly overwriting previously assigned acoustic components.
		If a surface does not yet have an association to a ComponentInstance, a new one will be created
		where the selected surface is assigned to side A.
	*/
	void on_pushButtonAssignAcousticComponent_clicked();
	/*! Assigns an acoustic component to two simultaneously selected surfaces. */
	void on_pushButtonAssignInsideAcousticComponent_clicked();
	/*! Assigns acoustic component from table to selected surfaces. */
	void on_pushButtonAssignAcousticComponentFromTable_clicked();
	/*! All surfaces that reference the currently selected component (in the table) will be selected. */
	void on_pushButtonSelectObjectsWithAcousticComponent_clicked();
private:
	/*! Launches component db edit dialog. */
	void openEditComponentDialog();

	/*! This function opens the acoustic component DB dialog and lets the user select an acoustic component.
		Then, it creates new component instances for all selected surfaces.
		If insideWall is true, the two selected surfaces are connected to each other with an inside-wall-component.

		\param fromSurfaceSelection If false, the surfaces matching the currently selected acoustic component in the acoustic component
			table are modified, rather than the currently selected surfaces in the scene (used for Exchange Component).
		\param componentID If INVALID_ID, the user is prompted to select the acoustic component to be assigned, otherwise
			the function works silently and assigns the given component.
	*/
	void assignAcousticComponent(bool insideWall, bool fromSurfaceSelection, unsigned int componentID = VICUS::INVALID_ID);

	/*! Based on the current selection in the table widget, the corresponding component is looked up and returned.
		Only for valid component assignments.
	*/
	const VICUS::AcousticComponent * currentlySelectedComponent() const;

	/*! Data structure that holds the information displayed in the table. */
	struct AcousticComponentLegendEntry {
		// 0 = not connected, 1 = connected without component, 2 - connected with component
		int									m_type = 0;
		/*! Pointer to the acoustic component associated with the table entry (only for type 3). */
		const VICUS::AcousticComponent		*m_acousticComponent = nullptr;
		/*! Associated surfaces with this table entry.
			Note: each visible surfaces should only appear once in one of the legend entries
		*/
		std::vector<const VICUS::Surface*>	m_surfaces;
	};

	/*! Stores the data shown in the table. */
	std::vector<AcousticComponentLegendEntry>		m_acousticComponentTable;

	/*! Stores list of all selected surfaces. */
	std::vector<const VICUS::Surface * >	m_selectedSurfaces;

	Ui::SVPropBuildingAcousticComponentWidget *m_ui;
};

#endif // SVPropBuildingAcousticComponentWidgetH
