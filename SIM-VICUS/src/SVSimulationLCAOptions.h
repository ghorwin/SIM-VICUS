#ifndef SVSIMULATIONLCAOPTIONS_H
#define SVSIMULATIONLCAOPTIONS_H

#include <QWidget>

#include <SVMainWindow.h>
#include <SVDBEPDEditWidget.h>

#include <SVSettings.h>

#include <VICUS_LCASettings.h>
#include <VICUS_Project.h>

namespace Ui {
class SVSimulationLCAOptions;
}

class SVSimulationLCAOptions : public QWidget {
	Q_OBJECT

public:

	struct AggregatedComponentData {

		enum Category {
			C_CategoryA,
			C_CategoryB,
			C_CategoryC,
			C_CategoryD,
			NUM_C
		};


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

		void addArea(const VICUS::ComponentInstance &compInst) {
			const VICUS::Surface *surfA = compInst.m_sideASurface;
			const VICUS::Surface *surfB = compInst.m_sideASurface;

			if(surfA == nullptr && surfB == nullptr)
				return;

			if(surfA != nullptr)
				m_area += surfA->geometry().area();
			else if(surfB != nullptr)
				m_area += surfB->geometry().area();
		}

		void addAggregatedData(const AggregatedComponentData &aggregatedData) {

			m_area += aggregatedData.m_area;

			for(unsigned int i=0; i<NUM_C; ++i)
				m_totalEpdData[i] += aggregatedData.m_totalEpdData[i];

			m_additionalComponents.insert(aggregatedData.m_component);
		}

		/*! Pointer to VICUS Component. */
		const VICUS::Component		*m_component = nullptr;

		std::set<const VICUS::Component *>	m_additionalComponents;

		/*! Area of all components used in VICUS project. */
		double						m_area;

		/*! Global EPD of used Component. */
		VICUS::EpdDataset			m_totalEpdData[NUM_C];

	};


	/*! Column enums. */
	enum ColData {
		ColUUID,
		ColVersion,
		ColNameDe,
		ColNameEn,
		ColCategoryDe,
		ColCategoryEn,
		ColConformity,
		ColCountryCode,
		ColType,
		ColReferenceYear,
		ColExpireYear,
		ColURL,
		ColDeclarationOwner,
		ColPublishedOn,
		ColRegistrationNumber,
		ColRegistrationBody,
		ColUUIDOfThePredecessor,
		ColVersionOfThePredecessor,
		ColURLOfThePredecessor,
		ColReferenceSize,
		ColReferenceUnit,
		ColReferenceFlowUUID,
		ColReferenceFlowName,
		ColBulkDensity,
		ColWeightPerUnitArea,
		ColRawDensity,
		ColLayerThickness,
		ColYield,
		ColLinearWeight,
		ColConversionFactorTo1kg,
		ColModule,
		ColGWP,
		ColODP,
		ColPOCP,
		ColAP,
		ColEP,
		ColADPE,
		ColADPF,
		ColPERE,
		ColPERM,
		ColPERT,
		ColPENRE,
		ColPENRM,
		ColPENRT,
		ColSM,
		ColRSF,
		ColNRSF,
		ColFW,
		ColHWD,
		ColNHWD,
		ColRWD,
		ColCRU,
		ColMFR,
		ColMER,
		ColEEE,
		ColEET,
		ColA2AP,
		ColA2gwptotal,
		ColA2gwpbiogenic,
		ColA2gwpfossil,
		ColA2gwpluluc,
		ColA2etpfw,
		ColA2PM,
		ColA2epmarine,
		ColA2epfreshwater,
		ColA2epterrestrial,
		ColA2htpc,
		ColA2htpnc,
		ColA2IRP,
		ColA2SOP,
		ColA2ODP,
		ColA2POCP,
		ColA2ADPF,
		ColA2ADPE,
		ColA2WDP,
	};

	SVSimulationLCAOptions(QWidget *parent, VICUS::LCASettings &settings);
	~SVSimulationLCAOptions();

	void calculateLCA();

private slots:
	void on_pushButtonImportOkoebaudat_clicked();

	void on_pushButtonLcaLcc_clicked();

private:
	/*! Import ÖKOBAUDAT as csv from
		https://www.oekobaudat.de/service/downloads.html. */
	void importOkoebauDat(const IBK::Path &csvPath);

	/*! Adds the area of a component instance to the aggregated component data. */
	void addComponentInstance(const VICUS::ComponentInstance &compInstance);

	/*! Analyses the current VICUS Project and adds all areas to m_compIdToAggregatedData. */
	void aggregateProjectComponents();

	/*! Aggregates all aggregated data by defined type in Component. Strored in m_compIdToAggregatedCompTypeData.
		Needed for result printing.
	*/
	void aggregateAggregatedComponentsByType();

	/*! Converts the material to the referenced reference quantity from the epd.
		\param layerThickness Thickness of layer in m
		\param layerArea Area of layer in m
	*/
	double conversionFactorEpdReferenceUnit(const IBK::Unit & refUnit, const VICUS::Material &layerMat,
											double layerThickness, double layerArea);

	/*! Writes calculated LCA data to a specified path. */
	void writeLcaDataToTxtFile(const IBK::Path & resultPath);

	/*! Calculates total EPD Data for each used Component in VICUS project.
		Transforms all epd data to the required epd reference unit from referenced categorys (A,B,C,D).
		Stores all data in m_compIdToAggregatedData.
	*/
	void calculateTotalLcaDataForComponents();

	/*! Reset all LCA Data. */
	void resetLcaData();


	void writeDataToStream(std::ofstream &lcaStream, const std::string &categoryText,
						   const AggregatedComponentData::Category & category);

	/*! Pointer to Ui */
	Ui::SVSimulationLCAOptions							*m_ui;

	/*! Pointer to LCA Settings. */
	VICUS::LCASettings									*m_lcaSettings = nullptr;

	/*! Cached pointer to database object. */
	SVDatabase											*m_db;

	/*! Map of aggregated component data.
		\param key ID of Component
		\param Value Aggregated Data where all areas are stored
	*/
	std::map<unsigned int, AggregatedComponentData>		m_compIdToAggregatedData;

	/*! Map of aggregated component data by component type.
		\param key Type enum from VICUS Component ComponentType
		\param Value Aggregated Data where all areas are stored
	*/
	std::map<VICUS::Component::ComponentType, AggregatedComponentData>	m_typeToAggregatedCompData;


	/*! Set with ids of components with undefined data, that we have to skip.
	*/
	std::set<unsigned int>								m_idComponentEpdUndefined;

	/*! Reference to VICUS Project. */
	const VICUS::Project								&m_prj;
};

#endif // SVSIMULATIONLCAOPTIONS_H
