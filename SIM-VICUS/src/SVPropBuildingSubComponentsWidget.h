#ifndef SVPropBuildingSubComponentsWidgetH
#define SVPropBuildingSubComponentsWidgetH

#include <QWidget>

#include <set>

namespace Ui {
	class SVPropBuildingSubComponentsWidget;
}

namespace VICUS {
	class SubSurfaceComponent;
	class SubSurface;
}

/*! Shows sub-surface component assignments (windows/doors).
	Implementation is basically a clone of the SVPropBuildingComponentsWidget, so see
	that code for explanation.
*/
class SVPropBuildingSubComponentsWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropBuildingSubComponentsWidget(QWidget *parent = nullptr);
	~SVPropBuildingSubComponentsWidget();

	/*! Updates user interface. */
	void updateUi();

private slots:
	void on_tableWidgetSubSurfaceComponents_itemSelectionChanged();
	void on_pushButtonEditSubSurfaceComponents_clicked();
	void on_pushButtonExchangeSubSurfaceComponents_clicked();
	void on_pushButtonSelectObjectsWithSubSurfaceComponent_clicked();
	void on_pushButtonAssignSubSurfaceComponent_clicked();
	void on_pushButtonAssignInsideSubSurfaceComponent_clicked();

private:
	/*! This function opens the sub-surface component DB dialog and lets the user select a sub-surface component.
		Then, it creates new sub-surface component instances for all selected surfaces.
		If insideWall is true, the two selected sub-surfaces are connected to each other with an inside-sub-surface-component.
	*/
	void assignSubSurfaceComponent(bool insideWall, bool fromSurfaceSelection);

	/*! Based on the current selection in the table widget, the corresponding component is looked up and returned.
		Only for valid component assignments.
	*/
	const VICUS::SubSurfaceComponent * currentlySelectedSubSurfaceComponent() const;

	/*! Data structure that holds the information displayed in the table. */
	struct ComponentLegendEntry {
		// 0 = not connected, 1 = connected without component, 2 - connected with component
		int												m_type = 0;
		/*! Pointer to the component associated with the table entry (only for type 3). */
		const VICUS::SubSurfaceComponent				*m_component = nullptr;
		/*! Associated surfaces with this table entry.
			Note: each visible surfaces should only appear once in one of the legend entries
		*/
		std::vector<const VICUS::SubSurface*>			m_surfaces;
	};

	/*! Stores list of all selected surfaces. */
	std::vector<const VICUS::SubSurface* >				m_selectedSurfaces;

	/*! Stores the data shown in the table. */
	std::vector<ComponentLegendEntry>					m_componentTable;

	std::map<const VICUS::SubSurfaceComponent *, std::set<const VICUS::SubSurface*> > m_surfaceComponentMap;

	Ui::SVPropBuildingSubComponentsWidget *m_ui;
};

#endif // SVPropBuildingSubComponentsWidgetH
