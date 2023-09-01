#include "SVLcaLccResultsDialog.h"
#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "QtExt_Conversions.h"
#include "ui_SVLcaLccResultsDialog.h"

#include <QTreeWidgetItem>
#include <VICUS_KeywordList.h>

/*! Converts the material to the referenced reference quantity from the epd.
	\param layerThickness Thickness of layer in m
	\param layerArea Area of layer in m
*/
double SVLcaLccResultsDialog::conversionFactorEpdReferenceUnit(const IBK::Unit & refUnit, const VICUS::Material &layerMat,
																double layerThickness, double layerArea){
	if(refUnit.name() == "kg")
		return layerArea * layerThickness * layerMat.m_para[VICUS::Material::P_Density].get_value("kg/m3"); // area * thickness * density --> layer mass

	if(refUnit.name() == "m2")
		return layerArea;

	if(refUnit.name() == "m3")
		return layerArea * layerThickness;

	if(refUnit.name() == "-")
		return 1; // Pieces are always set to 1 for now

	if(refUnit.name() == "MJ")
		return 1; // Also not implemented yet

	if(refUnit.name() == "kg/m3")
		return layerMat.m_para[VICUS::Material::P_Density].get_value("kg/m3");

	if(refUnit.name() == "a")
		return 50; // Also not implemented yet
	return 1;
}


SVLcaLccResultsDialog::SVLcaLccResultsDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVLcaLccResultsDialog)
{
	m_ui->setupUi(this);
}


SVLcaLccResultsDialog::~SVLcaLccResultsDialog() {
	delete m_ui;
}


void SVLcaLccResultsDialog::setLcaResults(const std::map<VICUS::Component::ComponentType, AggregatedComponentData> &lcaResultMap,
												 const std::map<unsigned int, AggregatedComponentData> compIdToAggregatedData,
												 const VICUS::EpdDataset::Category &category,
												 const VICUS::LcaSettings &settings,
												 std::vector<double> &investCost) const {
	double scaleFactor = 1.0;

	if(category != VICUS::EpdDataset::C_CategoryB)
		scaleFactor /= settings.m_para[VICUS::LcaSettings::P_TimePeriod].get_value("a");
	scaleFactor /= settings.m_para[VICUS::LcaSettings::P_NetUsageArea].value;
	scaleFactor *= settings.m_para[VICUS::LcaSettings::P_FactorBnbSimpleMode].value;

	QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_ui->treeWidgetLcaResults);
	rootItem->setText(ColCategory, VICUS::KeywordList::Description("EpdDataset::Category", category));
	QFont font;
	font.setBold(true);
	rootItem->setFont(ColCategory, font);

	double totalArea = 0.0;

	VICUS::EpdModuleDataset epdDataset;

	const SVDatabase &db = SVSettings::instance().m_db;

	for(std::map<VICUS::Component::ComponentType, AggregatedComponentData>::const_iterator itAggregatedComp = lcaResultMap.begin();
		itAggregatedComp != lcaResultMap.end(); ++itAggregatedComp)
	{

		QStringList lcaDataType;
		const AggregatedComponentData &aggregatedTypeData = itAggregatedComp->second;

		std::set<unsigned int> usedCompIds;
		for(const VICUS::Component *comp : aggregatedTypeData.m_additionalComponents) {
			if(m_idComponentEpdUndefined.find(comp->m_id) == m_idComponentEpdUndefined.end()) {
				usedCompIds.insert(comp->m_id);
			}
		}
		if(usedCompIds.empty())
			continue;

		totalArea += aggregatedTypeData.m_area;

		QTreeWidgetItem *item = new QTreeWidgetItem();
		rootItem->addChild(item);
		item->setText(ColComponentType, VICUS::KeywordList::Description("Component::ComponentType", aggregatedTypeData.m_component->m_type));
		item->setText(ColArea, QString( "%1 m2" ).arg( aggregatedTypeData.m_area, 7, 'f', 2 ) );
		item->setTextAlignment(ColArea, Qt::AlignRight);
		item->setTextAlignment(ColInvestCost, Qt::AlignRight);

		item->setText(ColInvestCost, QString( "%1 €" ).arg( aggregatedTypeData.m_area * aggregatedTypeData.m_totalCost.value / 100, 7, 'f', 2 ));
		item->setBackgroundColor(ColColor, aggregatedTypeData.m_component->m_color);

		VICUS::EpdModuleDataset epd;

		item->setExpanded(false);

		m_ui->treeWidgetLcaResults->addTopLevelItem(item);

		for(const VICUS::Component *comp : aggregatedTypeData.m_additionalComponents) {

			const AggregatedComponentData &aggregatedCompData = compIdToAggregatedData.at(comp->m_id);

			if(usedCompIds.find(comp->m_id) == usedCompIds.end())
				continue; // Skip unused ids

			QStringList lcaData;
			QTreeWidgetItem *itemChild = new QTreeWidgetItem();
			VICUS::EpdModuleDataset epdChild;


			itemChild->setText(ColComponentName, QtExt::MultiLangString2QString(comp->m_displayName));
			itemChild->setText(ColArea, QString( "%1 m2" ).arg( aggregatedCompData.m_area, 7, 'f', 2 ) );
			itemChild->setTextAlignment(ColArea, Qt::AlignRight);
			itemChild->setTextAlignment(ColInvestCost, Qt::AlignRight);

			itemChild->setText(ColInvestCost, QString( "%1 €" ).arg( aggregatedCompData.m_area * aggregatedCompData.m_totalCost.value / 100, 7, 'f', 2 ) );
			itemChild->setBackgroundColor(ColColor, aggregatedCompData.m_component->m_color);

			item->addChild(itemChild);


			// Hopefully will work
			const VICUS::Construction &con = *db.m_constructions[comp->m_idConstruction];
			for(unsigned int i=0; i<con.m_materialLayers.size(); ++i) {
				const VICUS::Material &mat = *db.m_materials[con.m_materialLayers[i].m_idMaterial];
				const VICUS::MaterialLayer &matLayer = con.m_materialLayers[i];

				const VICUS::EpdDataset *epdMat = db.m_epdDatasets[mat.m_epdCategorySet.m_idCategory[category]];

				if(epdMat == nullptr)
					continue; // no epd defined

				int usageTime = 0;

				bool isEmpty = matLayer.m_para[VICUS::MaterialLayer::P_LifeTime].empty();
				if(isEmpty){
					IBK::IBK_Message(IBK::FormatString("No usage time is specified for material layer '%1' of construction '%2'. Skipping material calculation.").arg(i).arg(con.m_displayName));
					continue;
				}
				unsigned int period = matLayer.m_para[VICUS::MaterialLayer::P_LifeTime].get_value("a");

				usageTime = settings.m_para[VICUS::LcaSettings::P_TimePeriod].get_value("a");
				double constructionCount = category == VICUS::EpdDataset::C_CategoryB ?
							1 :  // Usage is already normated
							std::ceil((float)usageTime / period - 1E-4); // to make it save

				VICUS::EpdModuleDataset epdCatData = epdMat->calcTotalEpdByCategory(category, settings);
				double conversionFactor = conversionFactorEpdReferenceUnit(epdMat->m_referenceUnit, mat,
																		   matLayer.m_para[VICUS::MaterialLayer::P_Thickness].get_value(),
																		   aggregatedCompData.m_area);
				VICUS::EpdModuleDataset scaledEpdCatData = epdCatData.scaleByFactor( constructionCount  * conversionFactor );
				epdChild += scaledEpdCatData;

				QTreeWidgetItem *itemMatChild = new QTreeWidgetItem();
				itemMatChild->setText(ColConstructionName, QtExt::MultiLangString2QString(mat.m_displayName));
				itemMatChild->setText(ColEpdName, QtExt::MultiLangString2QString(epdMat->m_displayName));
				itemMatChild->setText(ColArea, QString( "%1 m2" ).arg( aggregatedCompData.m_area, 7, 'f', 2 ) );
				itemMatChild->setTextAlignment(ColArea, Qt::AlignRight);
				itemMatChild->setTextAlignment(ColInvestCost, Qt::AlignRight);

				double totalCost = aggregatedCompData.m_area * matLayer.m_cost.value / 100;

				itemMatChild->setText(ColInvestCost, QString( "%1 €" ).arg( totalCost, 7, 'f', 2 ) );
				itemMatChild->setBackgroundColor(ColColor, mat.m_color);

				itemMatChild->setText(ColGWP,  QString::number(scaleFactor * scaledEpdCatData.m_para[VICUS::EpdModuleDataset::P_GWP ].get_value()));
				itemMatChild->setText(ColAP,   QString::number(scaleFactor * scaledEpdCatData.m_para[VICUS::EpdModuleDataset::P_AP  ].get_value()));
				itemMatChild->setText(ColEP,   QString::number(scaleFactor * scaledEpdCatData.m_para[VICUS::EpdModuleDataset::P_EP  ].get_value()));
				itemMatChild->setText(ColODP,  QString::number(scaleFactor * scaledEpdCatData.m_para[VICUS::EpdModuleDataset::P_ODP ].get_value()));
				itemMatChild->setText(ColPOCP, QString::number(scaleFactor * scaledEpdCatData.m_para[VICUS::EpdModuleDataset::P_POCP].get_value()));

				itemMatChild->setTextAlignment(ColGWP,	Qt::AlignRight);
				itemMatChild->setTextAlignment(ColAP,	Qt::AlignRight);
				itemMatChild->setTextAlignment(ColEP,	Qt::AlignRight);
				itemMatChild->setTextAlignment(ColODP,	Qt::AlignRight);
				itemMatChild->setTextAlignment(ColPOCP,	Qt::AlignRight);

				itemMatChild->setFlags(itemMatChild->flags() | Qt::ItemIsEditable);

				itemChild->addChild(itemMatChild);

				if(category == VICUS::EpdDataset::C_CategoryA) {
					unsigned int yearCount = settings.m_para[VICUS::LcaSettings::P_TimePeriod].get_value("a");

					for(unsigned int i=0; i<yearCount; ++i) {
						if(i%usageTime == 0)
							investCost[i] += totalCost;
					}
				}
			}


			itemChild->setText(ColGWP,  QString::number(scaleFactor * epdChild.m_para[VICUS::EpdModuleDataset::P_GWP ].get_value()));
			itemChild->setText(ColAP,   QString::number(scaleFactor * epdChild.m_para[VICUS::EpdModuleDataset::P_AP  ].get_value()));
			itemChild->setText(ColEP,   QString::number(scaleFactor * epdChild.m_para[VICUS::EpdModuleDataset::P_EP  ].get_value()));
			itemChild->setText(ColODP,  QString::number(scaleFactor * epdChild.m_para[VICUS::EpdModuleDataset::P_ODP ].get_value()));
			itemChild->setText(ColPOCP, QString::number(scaleFactor * epdChild.m_para[VICUS::EpdModuleDataset::P_POCP].get_value()));

			epd += epdChild;
		}

		item->setText(ColGWP,  QString::number(scaleFactor * epd.m_para[VICUS::EpdModuleDataset::P_GWP ].get_value()));
		item->setText(ColAP,   QString::number(scaleFactor * epd.m_para[VICUS::EpdModuleDataset::P_AP  ].get_value()));
		item->setText(ColEP,   QString::number(scaleFactor * epd.m_para[VICUS::EpdModuleDataset::P_EP  ].get_value()));
		item->setText(ColODP,  QString::number(scaleFactor * epd.m_para[VICUS::EpdModuleDataset::P_ODP ].get_value()));
		item->setText(ColPOCP, QString::number(scaleFactor * epd.m_para[VICUS::EpdModuleDataset::P_POCP].get_value()));

		epdDataset += epd;
	}

	rootItem->setText(ColGWP,  QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdModuleDataset::P_GWP ].get_value()));
	rootItem->setText(ColAP,   QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdModuleDataset::P_AP  ].get_value()));
	rootItem->setText(ColEP,   QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdModuleDataset::P_EP  ].get_value()));
	rootItem->setText(ColODP,  QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdModuleDataset::P_ODP ].get_value()));
	rootItem->setText(ColPOCP, QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdModuleDataset::P_POCP].get_value()));

	for(unsigned int i=0; i<NumCol; ++i)
		m_ui->treeWidgetLcaResults->resizeColumnToContents(i);

	m_ui->treeWidgetLcaResults->setColumnWidth(ColColor, 20);
}

void SVLcaLccResultsDialog::setUsageResults(const VICUS::LcaSettings &settings,
												   const double &gasConsumption,
												   const double &electricityConsumption,
												   const double &coalConsumption) {

	const SVDatabase &db = SVSettings::instance().m_db;

	const VICUS::EpdDataset *epdGas			= db.m_epdDatasets[settings.m_idUsage[VICUS::LcaSettings::UT_Gas]];
	const VICUS::EpdDataset *epdElectricity = db.m_epdDatasets[settings.m_idUsage[VICUS::LcaSettings::UT_Electricity]];
	const VICUS::EpdDataset *epdCoal		= db.m_epdDatasets[settings.m_idUsage[VICUS::LcaSettings::UT_Coal]];

	VICUS::EpdDataset totalEpd;

	IBK::Parameter paraGas ("GasConsumption", gasConsumption, IBK::Unit("kWh"));
	IBK::Parameter paraElectricity ("ElectricityConsumption", electricityConsumption, IBK::Unit("kWh"));
	IBK::Parameter paraCoal ("CoalConsumption", coalConsumption, IBK::Unit("kWh"));

	QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_ui->treeWidgetLcaResults);
	rootItem->setText(ColCategory, VICUS::KeywordList::Description("EpdDataset::Category", VICUS::EpdDataset::C_CategoryB));

	QFont font;
	font.setBold(true);
	rootItem->setFont(ColCategory, font);

	double scaleFactor = 1.0;
	scaleFactor /= settings.m_para[VICUS::LcaSettings::P_NetUsageArea].get_value();
	scaleFactor *= settings.m_para[VICUS::LcaSettings::P_FactorBnbSimpleMode].get_value();

	VICUS::EpdModuleDataset epdDataset;

	if(epdCoal != nullptr) {

		QTreeWidgetItem *item = new QTreeWidgetItem();

		VICUS::EpdModuleDataset epdCatDataset = epdCoal->calcTotalEpdByCategory(VICUS::EpdDataset::C_CategoryB, settings);
		epdCatDataset = epdCatDataset.scaleByFactor(paraCoal.get_value(epdCoal->m_referenceUnit));

		epdDataset += epdCatDataset;

		item->setText(ColComponentType, "Coal Consumption");
		item->setTextAlignment(ColArea, Qt::AlignRight);
		item->setText(ColArea, QString( "%1 kWh" ).arg( coalConsumption, 7, 'f', 2 ));

		item->setText(ColGWP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_GWP ].get_value()));
		item->setText(ColAP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_AP  ].get_value()));
		item->setText(ColEP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_EP  ].get_value()));
		item->setText(ColODP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_ODP ].get_value()));
		item->setText(ColPOCP, QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_POCP].get_value()));

		rootItem->addChild(item);
	}

	if(epdElectricity != nullptr) {

		QTreeWidgetItem *item = new QTreeWidgetItem();

		VICUS::EpdModuleDataset epdCatDataset = epdElectricity->calcTotalEpdByCategory(VICUS::EpdDataset::C_CategoryB, settings);
		epdCatDataset = epdCatDataset.scaleByFactor(paraCoal.get_value(epdElectricity->m_referenceUnit));

		epdDataset += epdCatDataset;

		item->setText(ColComponentType, "Electricity Consumption");
		item->setTextAlignment(ColArea, Qt::AlignRight);
		item->setText(ColArea, QString( "%1 kWh" ).arg( electricityConsumption, 7, 'f', 2 ));

		item->setText(ColGWP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_GWP ].get_value()));
		item->setText(ColAP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_AP  ].get_value()));
		item->setText(ColEP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_EP  ].get_value()));
		item->setText(ColODP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_ODP ].get_value()));
		item->setText(ColPOCP, QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_POCP].get_value()));

		rootItem->addChild(item);
	}

	if(epdGas != nullptr) {

		QTreeWidgetItem *item = new QTreeWidgetItem();

		VICUS::EpdModuleDataset epdCatDataset = epdGas->calcTotalEpdByCategory(VICUS::EpdDataset::C_CategoryB, settings);
		epdCatDataset = epdCatDataset.scaleByFactor(paraCoal.get_value(epdGas->m_referenceUnit));

		epdDataset += epdCatDataset;

		item->setText(ColComponentType, "Gas Consumption");
		item->setTextAlignment(ColArea, Qt::AlignRight);
		item->setText(ColArea, QString( "%1 kWh" ).arg( coalConsumption, 7, 'f', 2 ));

		item->setText(ColGWP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_GWP ].get_value()));
		item->setText(ColAP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_AP  ].get_value()));
		item->setText(ColEP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_EP  ].get_value()));
		item->setText(ColODP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_ODP ].get_value()));
		item->setText(ColPOCP, QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdModuleDataset::P_POCP].get_value()));

		rootItem->addChild(item);
	}

	rootItem->setText(ColGWP,  QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdModuleDataset::P_GWP ].get_value()));
	rootItem->setText(ColAP,   QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdModuleDataset::P_AP  ].get_value()));
	rootItem->setText(ColEP,   QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdModuleDataset::P_EP  ].get_value()));
	rootItem->setText(ColODP,  QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdModuleDataset::P_ODP ].get_value()));
	rootItem->setText(ColPOCP, QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdModuleDataset::P_POCP].get_value()));

	// Not useful
	m_ui->treeWidgetLcaResults->hideColumn(ColInvestCost);
}


void SVLcaLccResultsDialog::setCostResults(const VICUS::LccSettings &lccSettings, const VICUS::LcaSettings &lcaSettings,
										   double electricityCost, double coalCost, double gasCost,
										   const std::vector<double> &totalMaterialCost)
{

	QTableWidget &tab = *m_ui->tableWidgetLccResults;


	unsigned int rowCount = totalMaterialCost.size();
	m_ui->tableWidgetLccResults->setRowCount(rowCount);
	m_ui->tableWidgetLccResults->setSortingEnabled(false);

	double sumMaterialCost = 0.0;
	double sumEnergyCost = 0.0;
	double sumMaterialCostDiscounted = 0.0;
	double sumEnergyCostDiscounted = 0.0;
	double totalArea = lcaSettings.m_para[VICUS::LcaSettings::P_NetUsageArea].value;

	double annualPriceIncreaseEnergy = lccSettings.m_para[VICUS::LccSettings::P_PriceIncreaseEnergy].value;
	double annualPriceIncreaseGeneral = lccSettings.m_para[VICUS::LccSettings::P_PriceIncreaseGeneral].value;
	double discountingInterestRate = lccSettings.m_para[VICUS::LccSettings::P_DiscountingInterestRate].value;

	for (unsigned int i=0; i<rowCount; ++i) {

		tab.setItem(i, LCC_Year, new QTableWidgetItem(QString::number(i+1)));
		double discountingRate = 1/std::pow(1.0 + discountingInterestRate, i);
		tab.setItem(i, LCC_DiscountingRate, new QTableWidgetItem( QString( "%L1" ).arg( discountingRate, 0, 'f', 2 ) ));

		double priceEnergyFactorThisYear = std::pow(1.0 + annualPriceIncreaseEnergy, i);

		double electricityCostThisYear	= electricityCost * priceEnergyFactorThisYear;
		double coalCostThisYear			= coalCost * priceEnergyFactorThisYear;
		double gasCostThisYear			= gasCost * priceEnergyFactorThisYear;

		tab.setItem(i, LCC_PriceElectricity, new QTableWidgetItem( QString( "%L1 €" ).arg( electricityCostThisYear, 0, 'f', 2 ) ));
		tab.setItem(i, LCC_PriceCoal, new QTableWidgetItem( QString( "%L1 €" ).arg( coalCostThisYear, 0, 'f', 2 ) ));
		tab.setItem(i, LCC_PriceGas, new QTableWidgetItem( QString( "%L1 €" ).arg( gasCostThisYear, 0, 'f', 2 ) ));
		double totalEnergyCostsThisYear = electricityCostThisYear + coalCostThisYear + gasCostThisYear;
		tab.setItem(i, LCC_PriceEnergyTotal, new QTableWidgetItem( QString( "%L1 €" ).arg( totalEnergyCostsThisYear, 0, 'f', 2 ) ));
		sumEnergyCost += totalEnergyCostsThisYear;
		tab.setItem(i, LCC_PriceInvestEnergy, new QTableWidgetItem( QString( "%L1 €" ).arg( discountingRate*totalEnergyCostsThisYear, 0, 'f', 2 ) ));
		sumEnergyCostDiscounted += discountingRate*totalEnergyCostsThisYear;

		double priceMaterialFactorThisYear = std::pow(1.0 + annualPriceIncreaseGeneral, i);
		double totalMaterialCostsThisYear = totalMaterialCost[i]*priceMaterialFactorThisYear;
		tab.setItem(i, LCC_PriceMaterialCosts, new QTableWidgetItem( QString( "%L1 €" ).arg( totalMaterialCostsThisYear, 0, 'f', 2 ) ));
		sumMaterialCost += totalMaterialCostsThisYear;
		tab.setItem(i, LCC_PriceInvestMaterial, new QTableWidgetItem( QString( "%L1 €" ).arg( discountingRate*totalMaterialCostsThisYear, 0, 'f', 2 ) ));
		sumMaterialCostDiscounted += discountingRate*totalMaterialCostsThisYear;

		// make all items read-only and right aligned
		for (unsigned int j=0; j<NUM_LCCColumns; ++j) {
			QTableWidgetItem &item = *tab.item(i, j);
			item.setFlags(item.flags() & ~Qt::ItemIsEditable);
			item.setTextAlignment(Qt::AlignRight);
		}
	}

	double totalCost = sumEnergyCost+sumMaterialCost;
	double totalCostDiscounted = sumEnergyCostDiscounted+sumMaterialCostDiscounted;

	m_ui->tableWidgetLccOverview->setRowCount(3);

	QTableWidgetItem *item = new QTableWidgetItem();
	item->setText(tr("Present value building"));
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	m_ui->tableWidgetLccOverview->setItem(0, ColTitle, item);

	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setText("Normated net present value");
	m_ui->tableWidgetLccOverview->setItem(1, ColTitle, item);

	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setText(tr("Net present value part"));
	m_ui->tableWidgetLccOverview->setItem(2, ColTitle, item);

	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignRight);
	item->setText(QString( "%1 €" ).arg( sumMaterialCostDiscounted, 0, 'f', 2 ));
	m_ui->tableWidgetLccOverview->setItem(0, ColsProduction, item);

	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignRight);
	item->setText(QString( "%1 €" ).arg( sumEnergyCostDiscounted, 0, 'f', 2 ));
	m_ui->tableWidgetLccOverview->setItem(0, ColsEnergy, item);

	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignRight);
	item->setText(QString( "%1 €" ).arg( totalCostDiscounted, 0, 'f', 2 ));
	m_ui->tableWidgetLccOverview->setItem(0, ColsTotal, item);


	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignRight);
	item->setText(QString( "%1" ).arg( sumMaterialCostDiscounted/totalArea, 0, 'f', 2 ));
	m_ui->tableWidgetLccOverview->setItem(1, ColsProduction, item);

	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignRight);
	item->setText(QString( "%1" ).arg( sumEnergyCostDiscounted/totalArea, 0, 'f', 2 ));
	m_ui->tableWidgetLccOverview->setItem(1, ColsEnergy, item);

	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignRight);
	item->setText(QString( "%1" ).arg( totalCostDiscounted/totalArea, 0, 'f', 2 ));
	m_ui->tableWidgetLccOverview->setItem(1, ColsTotal, item);


	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignRight);
	item->setText(QString( "%1" ).arg( 100.0*sumMaterialCostDiscounted/totalCostDiscounted, 0, 'f', 2 ));
	m_ui->tableWidgetLccOverview->setItem(2, ColsProduction, item);

	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignRight);
	item->setText(QString( "%1" ).arg( 100.0*sumEnergyCost/totalCostDiscounted, 0, 'f', 2 ));
	m_ui->tableWidgetLccOverview->setItem(2, ColsEnergy, item);

	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignRight);
	item->setText(QString( "%1" ).arg( 100.0*totalCostDiscounted/totalCostDiscounted, 0, 'f', 2 ));
	m_ui->tableWidgetLccOverview->setItem(2, ColsTotal, item);

	m_ui->tableWidgetLccResults->resizeColumnsToContents();
	m_ui->tableWidgetLccOverview->resizeColumnsToContents();
}


void SVLcaLccResultsDialog::setup() {
	// Add data to treeWidget
	m_ui->treeWidgetLcaResults->clear();
	m_ui->treeWidgetLcaResults->setColumnCount(NumCol);
	QStringList headersLca;
	headersLca << tr("Category") << "" << tr("Type") << tr("Name") << tr("Name") << tr("EPD") << tr("Amount") << tr("Invest-Cost [€]") << tr("GWP (CO2-Äqu.) [kg/(m2a)");
	headersLca << tr("ODP (R11-Äqu.) [kg/(m2a)]") << tr("POCP (C2H4-Äqu.) [kg/(m2a)]") << tr("AP (SO2-Äqu.) [kg/(m2a)]") << tr("EP (PO4-Äqu.) [kg/(m2a)]");

	m_ui->treeWidgetLcaResults->setHeaderLabels(headersLca);
	m_ui->treeWidgetLcaResults->setAlternatingRowColors(true);

	//SVStyle::formatDatabaseTreeView(m_ui->treeWidgetLcaResults);

	m_ui->treeWidgetLcaResults->setItemsExpandable(true);
	m_ui->tableWidgetLccResults->setColumnCount(NUM_LCCColumns);

	QStringList headersLcc;
	headersLcc << tr("Year [a]")
			   << tr("Eletricity costs [€]")
			   << tr("Gas costs [€]")
			   << tr("Coal costs [€]")
			   << tr("Total energy costs [€]")
			   << tr("Discounting rate")
			   << tr("Energy - net present value  [€]")
			   << tr("Material costs [€]")
			   << tr("Material - net present value  [€]")
			   << tr("Price increase general [%]")
			   << tr("Price increase energy [%]");
	m_ui->tableWidgetLccResults->setHorizontalHeaderLabels(headersLcc);

	m_ui->tableWidgetLccOverview->setColumnCount(NumColLccOverview);
	QStringList headerLccOverview;
	headerLccOverview << tr("Name") << tr("Unit") << tr("Production") << tr("Energy")	<< tr("Total");
	m_ui->tableWidgetLccOverview->setHorizontalHeaderLabels(headerLccOverview);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetLccResults);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetLccOverview);

	m_ui->tableWidgetLccOverview->setSortingEnabled(false);
}

void SVLcaLccResultsDialog::on_treeWidgetLcaResults_itemExpanded(QTreeWidgetItem *item) {
	for(unsigned int i=0; i<NumCol; ++i)
		m_ui->treeWidgetLcaResults->resizeColumnToContents(i);

	m_ui->treeWidgetLcaResults->setColumnWidth(ColColor, 20);
}


void SVLcaLccResultsDialog::on_treeWidgetLcaResults_itemCollapsed(QTreeWidgetItem *item) {
	for(unsigned int i=0; i<NumCol; ++i)
		m_ui->treeWidgetLcaResults->resizeColumnToContents(i);

	m_ui->treeWidgetLcaResults->setColumnWidth(ColColor, 20);
}


void SVLcaLccResultsDialog::on_pushButtonClose_clicked() {
	accept();
}

