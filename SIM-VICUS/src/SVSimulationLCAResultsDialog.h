#ifndef SVSimulationLCAResultsDialogH
#define SVSimulationLCAResultsDialogH

#include <QDialog>
#include <QTreeWidgetItem>

#include "SVSettings.h"

#include <VICUS_Component.h>
#include <VICUS_ComponentInstance.h>
#include <VICUS_EpdDataset.h>
#include <VICUS_Surface.h>

struct AggregatedComponentData {

	AggregatedComponentData() {}

	/*! Construct AggregatedComponentData with pointer to VICUS::Component and
		empty Area. */
	AggregatedComponentData(const VICUS::ComponentInstance &compInst) :
		m_component(SVSettings::instance().m_db.m_components[compInst.m_idComponent]),
		m_area(0.0)
	{
		addArea(compInst);
		m_additionalComponents.insert(m_component);
	}

	/*! Adds area of an component instance to the aggregated component data. */
	void addArea(const VICUS::ComponentInstance &compInst) {
		const VICUS::Surface *surfA = compInst.m_sideASurface;
		const VICUS::Surface *surfB = compInst.m_sideASurface;

		if(surfA == nullptr && surfB == nullptr)
			return;

		if(surfA != nullptr) {
			m_area += surfA->geometry().area();
			for(unsigned int i=0; i<surfA->subSurfaces().size(); ++i) {
				m_area -= surfA->subSurfaces()[i].m_polygon2D.area();
			}
		}
		else if(surfB != nullptr) {
			m_area += surfB->geometry().area();
			for(unsigned int i=0; i<surfB->subSurfaces().size(); ++i) {
				m_area -= surfA->subSurfaces()[i].m_polygon2D.area();
			}
		}
	}

	/*! Combines Aggregated Component data.
		Adds Area, Aggregated LCA Data and inserts pointer to additional components.
	*/
	void addAggregatedData(const AggregatedComponentData &aggregatedData) {

		m_area += aggregatedData.m_area;

		for(unsigned int i=0; i<VICUS::EpdDataset::NUM_C; ++i) {
			m_totalEpdData[i] += aggregatedData.m_totalEpdData[i];
		}

		m_additionalComponents.insert(aggregatedData.m_component);
	}

	/*! Pointer to VICUS Component. */
	const VICUS::Component					*m_component = nullptr;

	/*! Vector with additional components. */
	std::set<const VICUS::Component *>		m_additionalComponents;

	/*! Area of all components used in VICUS project. */
	double									m_area;

	/*! Global EPD of used Component. */
	VICUS::EpdDataset						m_totalEpdData[VICUS::EpdDataset::NUM_C];

};


namespace VICUS {
class Component;
}

namespace Ui {
class SVSimulationLCAResultsDialog;
}

class SVSimulationLCAResultsDialog : public QDialog
{
	Q_OBJECT

	enum Cols {
		ColCategory,
		ColColor,
		ColComponentType,
		ColComponentName,
		ColArea,
		ColGWP,
		ColODP,
		ColPOCP,
		ColAP,
		ColEP,
		NumCol
	};

public:
	explicit SVSimulationLCAResultsDialog(QWidget *parent = nullptr);
	~SVSimulationLCAResultsDialog();

	void setLcaResults(const std::map<VICUS::Component::ComponentType, AggregatedComponentData> &lcaResultMap,
					   const std::map<unsigned int, AggregatedComponentData> compIdToAggregatedData,
					   const VICUS::EpdDataset::Category &category, const VICUS::LcaSettings &settings) const;


private slots:
	void on_treeWidgetLcaResults_itemExpanded(QTreeWidgetItem *item);

	void on_treeWidgetLcaResults_itemCollapsed(QTreeWidgetItem *item);

private:

	/*! Sets up Dialog with LCA Data. Needs to be called once before Dialog is beeing used. */
	void setup();

	/*! Set with ids of components with undefined data, that we have to skip.
	*/
	std::set<unsigned int>					m_idComponentEpdUndefined;

	/*! Pointer to UI. */
	Ui::SVSimulationLCAResultsDialog *m_ui;
};

#endif // SVSimulationLCAResultsDialogH
