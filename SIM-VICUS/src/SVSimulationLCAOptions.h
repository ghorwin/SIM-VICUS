#ifndef SVSimulationLCAOptionsH
#define SVSimulationLCAOptionsH

#include <QWidget>

#include "SVSimulationLCAResultsDialog.h"

#include <SVSettings.h>

#include <VICUS_LcaSettings.h>
#include <VICUS_LccSettings.h>
#include <VICUS_Project.h>

namespace Ui {
class SVSimulationLCAOptions;
}

class SVSimulationLCAOptions : public QWidget {
	Q_OBJECT

public:


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

	SVSimulationLCAOptions(QWidget *parent, VICUS::LcaSettings &settings);
	~SVSimulationLCAOptions();

	void calculateLCA();

	/*! Updates user interface with properties from the project data structure.
		This function is called whenever the dialog is first shown.
	*/
	void updateUi();

private slots:
	void on_pushButtonImportOkoebaudat_clicked();


	/*! Sets the modules state in lca settings from the specified toggled checkbox.
		Uses the checkbox' text and determines which states need to be set.
	*/
	void setModuleState(int state);




	void on_comboBoxCalculationMode_currentIndexChanged(int index);

	void on_comboBoxCertificationSystem_currentIndexChanged(int index);



	void on_pushButtonAreaDetection_clicked();

	void on_pushButtonLcc_clicked();

	void on_pushButtonLca_clicked();

	void on_lineEditArea_editingFinishedSuccessfully();

	void on_lineEditTimePeriod_editingFinishedSuccessfully();

	void on_lineEditPriceIncrease_editingFinishedSuccessfully();

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

	/*! Writes calculated LCA data to a specified path. */
	void writeLcaDataToTxtFile(const IBK::Path & resultPath);

	/*! Calculates total EPD Data for each used Component in VICUS project.
		Transforms all epd data to the required epd reference unit from referenced categorys (A,B,C,D).
		Stores all data in m_compIdToAggregatedData.
	*/
	void calculateTotalLcaDataForComponents();

	/*! Reset all LCA Data. */
	void resetLcaData();

	/*! Write data to output file stream. */
	void writeDataToStream(std::ofstream &lcaStream,
						   const std::string &categoryText,
						   const VICUS::EpdDataset::Category &category);

	/*! Sets the CheckBox state and uses a specified bitmask to determine whether it is checked. */
	void setCheckBoxState(QCheckBox *cb, int bitmask);

	/*! set value. */
	template<typename T>
	void setValue(T & member, const T & value, bool foundExistingEpd);

	/*! Returns the pointer to the Results Dialog. */
	SVSimulationLCAResultsDialog						*lcaResultsDialog();

	/*! Pointer to Ui */
	Ui::SVSimulationLCAOptions							*m_ui;

	/*! Pointer to LCA Settings. */
	VICUS::LcaSettings									*m_lcaSettings = nullptr;

	/*! Pointer to LCC Settings. */
	VICUS::LccSettings									*m_lccSettings = nullptr;

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

	/*! Pointer to LCA Result Dialog. */
	SVSimulationLCAResultsDialog						*m_lcaResultDialog = nullptr;

	/*! Reference to VICUS Project. */
	const VICUS::Project								&m_prj;

	/*! Store checked data. */
	std::vector<IBK::Flag>								m_intParas[VICUS::EpdCategoryDataset::NUM_M];

};

#endif // SVSimulationLCAOptionsH
