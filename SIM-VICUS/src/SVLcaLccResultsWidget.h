#ifndef SVLcaLccResultsWidgetH
#define SVLcaLccResultsWidgetH

#include <QDialog>
#include <QTreeWidgetItem>

#include "SVSettings.h"

#include <VICUS_Component.h>
#include <VICUS_Construction.h>
#include <VICUS_Database.h>
#include <VICUS_ComponentInstance.h>
#include <VICUS_EpdDataset.h>
#include <VICUS_Surface.h>

struct AggregatedComponentData {

	AggregatedComponentData() {}

	/*! Construct AggregatedComponentData with pointer to VICUS::Component and
		empty Area. */
	AggregatedComponentData(const VICUS::ComponentInstance &compInst) :
		m_component(SVSettings::instance().m_db.m_components[compInst.m_idComponent]),
		m_area(0.0),
		m_totalCost(IBK::IntPara("TotalCost", 0))
	{
		FUNCID(AggregatedComponentData::AggregatedComponentData);

		addArea(compInst);
		m_additionalComponents.insert(m_component);

		const SVDatabase &db = SVSettings::instance().m_db;
		const VICUS::Component *comp = db.m_components[compInst.m_idComponent];

		if(comp == nullptr) {
			IBK::IBK_Message(IBK::FormatString("Component #%1 has not been found.").arg(comp->m_id), IBK::MSG_ERROR, FUNC_ID);
			return;
		}

		const VICUS::Construction *con = db.m_constructions[comp->m_idConstruction];

		if(con == nullptr) {
			IBK::IBK_Message(IBK::FormatString("Construction #%1 has not been found.").arg(comp->m_idConstruction), IBK::MSG_ERROR, FUNC_ID);
			return;
		}

		for(const VICUS::MaterialLayer &ml : con->m_materialLayers) {
			m_totalCost.value += ml.m_cost.value;
		}
	}

	/*! Adds area of an component instance to the aggregated component data. */
	void addArea(const VICUS::ComponentInstance &compInst) {
		const VICUS::Surface *surfA = compInst.m_sideASurface;
		const VICUS::Surface *surfB = compInst.m_sideBSurface;

		if(surfA == nullptr && surfB == nullptr)
			return;

		if(surfA != nullptr) {
			m_area += surfA->geometry().area();
			for(unsigned int i=0; i<surfA->subSurfaces().size(); ++i) {
				m_area -= surfA->subSurfaces()[i].m_polygon2D.area();
			}
			return;
		}
		else if(surfB != nullptr) {
			m_area += surfB->geometry().area();
			for(unsigned int i=0; i<surfB->subSurfaces().size(); ++i) {
				m_area -= surfB->subSurfaces()[i].m_polygon2D.area();
			}
			return;
		}
	}

	/*! Combines Aggregated Component data.
		Adds Area, Aggregated LCA Data and inserts pointer to additional components.
	*/
	void addAggregatedData(const AggregatedComponentData &aggregatedData) {

		m_area += aggregatedData.m_area;

		m_totalCost.value += aggregatedData.m_totalCost.value;

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

	/*! Total Cost of Components in EuroCent. */
	IBK::IntPara							m_totalCost;

	/*! Pointer to Database. */
	SVDatabase								*m_db;

	/*! Global EPD of used Component. */
	VICUS::EpdDataset						m_totalEpdData[VICUS::EpdDataset::NUM_C];

};


namespace VICUS {
class Component;
}

namespace Ui {
class SVLcaLccResultsWidget;
}

class SVLcaLccResultsWidget : public QWidget {
	Q_OBJECT

public:
	enum ResultType {
		RT_LCA,
		RT_LCC,
		NUM_RT
	};

	enum Cols {
		ColCategory,
		ColComponentType,
		ColColor,
		ColComponentName,
		ColMaterialName,
		ColEpdName,
		ColArea,
		ColInvestCost,
		ColGWP,
		ColODP,
		ColPOCP,
		ColAP,
		ColEP,
		NumCol
	};

	enum LCCColumns {
		LCC_Year,
		LCC_PriceElectricity,
		LCC_PriceGas,
		LCC_PriceCoal,
		LCC_PriceEnergyTotal,
		LCC_DiscountingRate,
		LCC_PriceInvestEnergy,		// Present Value - Energy
		LCC_PriceMaterialCosts,
		LCC_PriceInvestMaterial,	// Present Value - Material
		NUM_LCCColumns
	};

	enum LCCSummaryColumns {
		LCCS_Title,
		LCCS_Production,
		LCCS_Energy,
		LCCS_Total,
		NUM_LCCS
	};


	explicit SVLcaLccResultsWidget(QWidget *parent = nullptr);
	~SVLcaLccResultsWidget() override;

	/*! Sets up Dialog with LCA Data. Needs to be called once before Dialog is beeing used. */
	void setup();

	/*! Sets category specific results in the TreeWidget. Only Categories A,C,D should be used with this function.
		Since category B (Usage) is set differently in LCA Settings widget.
	*/
	void setLcaResults(const std::map<VICUS::Component::ComponentType, AggregatedComponentData> &lcaResultMap,
					   const std::map<unsigned int, AggregatedComponentData> compIdToAggregatedData,
					   const VICUS::EpdDataset::Category &category, const VICUS::LcaSettings &settings,
					   std::vector<double> &investCost) const;

	/*! Sets all Usage specific (Category B) results inside the tree widget. */
	void setUsageResults(const VICUS::LcaSettings &settings, const double &gasConsumption, const double &electricityConsumption,
						 const double &coalConsumption);

	/*! Utility costs are given for first year. Material costs are given based on todays costs for each year. */
	void setCostResults(const VICUS::LccSettings &lccSettings, const VICUS::LcaSettings &lcaSettings,
						double electricityCost,
						double coalCost,
						double gasCost,
						const std::vector<double> &totalMaterialCost);

	/*! Converts the material to the referenced reference quantity from the epd.
		\param layerThickness Thickness of layer in m
		\param layerArea Area of layer in m
	*/
	static double conversionFactorEpdReferenceUnit(const IBK::Unit & refUnit, const VICUS::Material &layerMat,
																double layerThickness, double layerArea);

private slots:
	void on_treeWidgetLcaResults_itemExpanded(QTreeWidgetItem *item);

	void on_treeWidgetLcaResults_itemCollapsed(QTreeWidgetItem *item);

private:


	/*! Set with ids of components with undefined data, that we have to skip.
	*/
	std::set<unsigned int>					m_idComponentEpdUndefined;

	/*! Pointer to UI. */
	Ui::SVLcaLccResultsWidget *m_ui;
};

#endif // SVLcaLccResultsWidgetH
