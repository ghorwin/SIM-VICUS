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

		AggregatedComponentData() {}

		/*! Construct AggregatedComponentData with pointer to VICUS::Component and
			empty Area. */
		AggregatedComponentData(const VICUS::ComponentInstance &compInst) :
			m_component(SVSettings::instance().m_db.m_components[compInst.m_idComponent]),
			m_area(0.0)
		{}

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

		/*! Pointer to VICUS Component. */
		const VICUS::Component		*m_component = nullptr;

		/*! Area of all components used in VICUS project. */
		double						m_area;

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
	void analyseProjectComponents();


	/*! Pointer to Ui */
	Ui::SVSimulationLCAOptions							*m_ui;

	/*! Pointer to LCA Settings. */
	VICUS::LCASettings									*m_lcaSettings = nullptr;

	/*! Cached pointer to database object. */
	SVDatabase											*m_db;

	/*! Map of aggregated component data.
		\param key is ID of Component
		\param Value is Aggregated Data where all areas are stored
	*/
	std::map<unsigned int, AggregatedComponentData>		m_compIdToAggregatedData;

	/*! Copy of VICUS Project. */
	VICUS::Project										m_prj;
};

#endif // SVSIMULATIONLCAOPTIONS_H
