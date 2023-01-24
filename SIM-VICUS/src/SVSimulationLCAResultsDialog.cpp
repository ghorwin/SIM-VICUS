#include "SVSimulationLCAResultsDialog.h"
#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "QtExt_Conversions.h"
#include "ui_SVSimulationLCAResultsDialog.h"

#include <QTreeWidgetItem>
#include <VICUS_KeywordList.h>

/*! Converts the material to the referenced reference quantity from the epd.
	\param layerThickness Thickness of layer in m
	\param layerArea Area of layer in m
*/
double SVSimulationLCAResultsDialog::conversionFactorEpdReferenceUnit(const IBK::Unit & refUnit, const VICUS::Material &layerMat,
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
}


SVSimulationLCAResultsDialog::SVSimulationLCAResultsDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSimulationLCAResultsDialog)
{
	m_ui->setupUi(this);

	setup();
}

SVSimulationLCAResultsDialog::~SVSimulationLCAResultsDialog() {
	delete m_ui;
}

void SVSimulationLCAResultsDialog::setLcaResults(const std::map<VICUS::Component::ComponentType, AggregatedComponentData> &lcaResultMap,
												 const std::map<unsigned int, AggregatedComponentData> compIdToAggregatedData,
												 const VICUS::EpdDataset::Category &category,
												 const VICUS::LcaSettings &settings,
												 std::vector<double> &investCost) const {
	double scaleFactor = 1.0;

	if(category != VICUS::EpdDataset::C_CategoryB)
		scaleFactor /= settings.m_para[VICUS::LcaSettings::P_TimePeriod].get_value();
	scaleFactor /= settings.m_para[VICUS::LcaSettings::P_NetUsageArea].get_value();
	scaleFactor *= settings.m_para[VICUS::LcaSettings::P_FactorBnbSimpleMode].get_value();

	QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_ui->treeWidgetLcaResults);
	rootItem->setText(ColCategory, VICUS::KeywordList::Description("EpdDataset::Category", category));
	QFont font;
	font.setBold(true);
	rootItem->setFont(ColCategory, font);

	double totalArea = 0.0;

	VICUS::EpdCategoryDataset epdDataset;

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

		VICUS::EpdCategoryDataset epd;

		item->setExpanded(false);

		m_ui->treeWidgetLcaResults->addTopLevelItem(item);

		for(const VICUS::Component *comp : aggregatedTypeData.m_additionalComponents) {

			const AggregatedComponentData &aggregatedCompData = compIdToAggregatedData.at(comp->m_id);

			if(usedCompIds.find(comp->m_id) == usedCompIds.end())
				continue; // Skip unused ids

			QStringList lcaData;
			QTreeWidgetItem *itemChild = new QTreeWidgetItem();
			VICUS::EpdCategoryDataset epdChild;


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

				double renewingFactor = category == VICUS::EpdDataset::C_CategoryB ?
							1 :  // Usage is already normated
							settings.m_para[VICUS::LcaSettings::P_TimePeriod].get_value() / matLayer.m_para[VICUS::MaterialLayer::P_LifeTime].get_value();

				VICUS::EpdCategoryDataset epdCatData = epdMat->calcTotalEpdByCategory(category, settings);
				double conversionFactor = conversionFactorEpdReferenceUnit(epdMat->m_referenceUnit, mat,
																		   matLayer.m_para[VICUS::MaterialLayer::P_Thickness].get_value(),
																		   aggregatedCompData.m_area);
				VICUS::EpdCategoryDataset scaledEpdCatData = epdCatData.scaleByFactor( renewingFactor * conversionFactor );
				epdChild += scaledEpdCatData;

				QTreeWidgetItem *itemMatChild = new QTreeWidgetItem();
				itemMatChild->setText(ColConstructionName, QtExt::MultiLangString2QString(mat.m_displayName));
				itemMatChild->setText(ColEpdName, QtExt::MultiLangString2QString(epdMat->m_displayName));
				itemMatChild->setText(ColArea, QString( "%1 m2" ).arg( aggregatedCompData.m_area, 7, 'f', 2 ) );
				itemMatChild->setTextAlignment(ColArea, Qt::AlignRight);
				itemMatChild->setTextAlignment(ColInvestCost, Qt::AlignRight);

				double totalCost = aggregatedCompData.m_area * matLayer.m_cost.value / 100;
				int usageTime = matLayer.m_para[VICUS::MaterialLayer::P_LifeTime].get_value("a");

				itemMatChild->setText(ColInvestCost, QString( "%1 €" ).arg( totalCost, 7, 'f', 2 ) );
				itemMatChild->setBackgroundColor(ColColor, mat.m_color);

				itemMatChild->setText(ColGWP,  QString::number(scaleFactor * scaledEpdCatData.m_para[VICUS::EpdCategoryDataset::P_GWP ].get_value()));
				itemMatChild->setText(ColAP,   QString::number(scaleFactor * scaledEpdCatData.m_para[VICUS::EpdCategoryDataset::P_AP  ].get_value()));
				itemMatChild->setText(ColEP,   QString::number(scaleFactor * scaledEpdCatData.m_para[VICUS::EpdCategoryDataset::P_EP  ].get_value()));
				itemMatChild->setText(ColODP,  QString::number(scaleFactor * scaledEpdCatData.m_para[VICUS::EpdCategoryDataset::P_ODP ].get_value()));
				itemMatChild->setText(ColPOCP, QString::number(scaleFactor * scaledEpdCatData.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));

				itemChild->addChild(itemMatChild);

				if(category == VICUS::EpdDataset::C_CategoryA) {
					unsigned int yearCount = settings.m_para[VICUS::LcaSettings::P_TimePeriod].get_value("a");

					for(unsigned int i=0; i<yearCount; ++i) {
						if(i%usageTime == 0)
							investCost[i] += totalCost;
					}
				}
			}


			itemChild->setText(ColGWP,  QString::number(scaleFactor * epdChild.m_para[VICUS::EpdCategoryDataset::P_GWP ].get_value()));
			itemChild->setText(ColAP,   QString::number(scaleFactor * epdChild.m_para[VICUS::EpdCategoryDataset::P_AP  ].get_value()));
			itemChild->setText(ColEP,   QString::number(scaleFactor * epdChild.m_para[VICUS::EpdCategoryDataset::P_EP  ].get_value()));
			itemChild->setText(ColODP,  QString::number(scaleFactor * epdChild.m_para[VICUS::EpdCategoryDataset::P_ODP ].get_value()));
			itemChild->setText(ColPOCP, QString::number(scaleFactor * epdChild.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));

			epd += epdChild;
		}

		item->setText(ColGWP,  QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_GWP ].get_value()));
		item->setText(ColAP,   QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_AP  ].get_value()));
		item->setText(ColEP,   QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_EP  ].get_value()));
		item->setText(ColODP,  QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_ODP ].get_value()));
		item->setText(ColPOCP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));

		epdDataset += epd;
	}

	rootItem->setText(ColGWP,  QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_GWP ].get_value()));
	rootItem->setText(ColAP,   QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_AP  ].get_value()));
	rootItem->setText(ColEP,   QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_EP  ].get_value()));
	rootItem->setText(ColODP,  QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_ODP ].get_value()));
	rootItem->setText(ColPOCP, QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));



	for(unsigned int i=0; i<NumCol; ++i)
		m_ui->treeWidgetLcaResults->resizeColumnToContents(i);

	m_ui->treeWidgetLcaResults->setColumnWidth(ColColor, 20);
}

void SVSimulationLCAResultsDialog::setUsageResults(const VICUS::LcaSettings &settings,
												   const double &gasConsumption,
												   const double &electricityConsumption,
												   const double &coalConsumption) {

	QTreeWidgetItem *itemGas		 = new QTreeWidgetItem();
	QTreeWidgetItem *itemElectricity = new QTreeWidgetItem();
	QTreeWidgetItem *itemCoal		 = new QTreeWidgetItem();

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

	VICUS::EpdCategoryDataset epdDataset;

	if(epdCoal != nullptr) {

		QTreeWidgetItem *item = new QTreeWidgetItem();

		VICUS::EpdCategoryDataset epdCatDataset = epdCoal->calcTotalEpdByCategory(VICUS::EpdDataset::C_CategoryB, settings);
		epdCatDataset = epdCatDataset.scaleByFactor(paraCoal.get_value(epdCoal->m_referenceUnit));

		epdDataset += epdCatDataset;

		item->setText(ColComponentType, "Coal Consumption");
		item->setTextAlignment(ColArea, Qt::AlignRight);
		item->setText(ColArea, QString( "%1 kWh" ).arg( coalConsumption, 7, 'f', 2 ));

		item->setText(ColGWP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_GWP ].get_value()));
		item->setText(ColAP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_AP  ].get_value()));
		item->setText(ColEP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_EP  ].get_value()));
		item->setText(ColODP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_ODP ].get_value()));
		item->setText(ColPOCP, QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));

		rootItem->addChild(item);
	}

	if(epdElectricity != nullptr) {

		QTreeWidgetItem *item = new QTreeWidgetItem();

		VICUS::EpdCategoryDataset epdCatDataset = epdElectricity->calcTotalEpdByCategory(VICUS::EpdDataset::C_CategoryB, settings);
		epdCatDataset = epdCatDataset.scaleByFactor(paraCoal.get_value(epdElectricity->m_referenceUnit));

		epdDataset += epdCatDataset;

		item->setText(ColComponentType, "Electricity Consumption");
		item->setTextAlignment(ColArea, Qt::AlignRight);
		item->setText(ColArea, QString( "%1 kWh" ).arg( electricityConsumption, 7, 'f', 2 ));

		item->setText(ColGWP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_GWP ].get_value()));
		item->setText(ColAP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_AP  ].get_value()));
		item->setText(ColEP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_EP  ].get_value()));
		item->setText(ColODP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_ODP ].get_value()));
		item->setText(ColPOCP, QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));

		rootItem->addChild(item);
	}

	if(epdGas != nullptr) {

		QTreeWidgetItem *item = new QTreeWidgetItem();

		VICUS::EpdCategoryDataset epdCatDataset = epdGas->calcTotalEpdByCategory(VICUS::EpdDataset::C_CategoryB, settings);
		epdCatDataset = epdCatDataset.scaleByFactor(paraCoal.get_value(epdGas->m_referenceUnit));

		epdDataset += epdCatDataset;

		item->setText(ColComponentType, "Gas Consumption");
		item->setTextAlignment(ColArea, Qt::AlignRight);
		item->setText(ColArea, QString( "%1 kWh" ).arg( coalConsumption, 7, 'f', 2 ));

		item->setText(ColGWP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_GWP ].get_value()));
		item->setText(ColAP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_AP  ].get_value()));
		item->setText(ColEP,   QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_EP  ].get_value()));
		item->setText(ColODP,  QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_ODP ].get_value()));
		item->setText(ColPOCP, QString::number(scaleFactor * epdCatDataset.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));

		rootItem->addChild(item);
	}

	rootItem->setText(ColGWP,  QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_GWP ].get_value()));
	rootItem->setText(ColAP,   QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_AP  ].get_value()));
	rootItem->setText(ColEP,   QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_EP  ].get_value()));
	rootItem->setText(ColODP,  QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_ODP ].get_value()));
	rootItem->setText(ColPOCP, QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));

	// Not useful
	m_ui->treeWidgetLcaResults->hideColumn(ColInvestCost);
}

void SVSimulationLCAResultsDialog::setCostResults(const VICUS::LccSettings &lccSettings, const VICUS::LcaSettings &lcaSettings, const double &totalEnergyCost,
												  const std::vector<double> &totalMaterialCost) {

	QTableWidget &tab = *m_ui->tableWidgetLccResults;


	unsigned int rowCount = lcaSettings.m_para[VICUS::LcaSettings::P_TimePeriod].get_value("a");
	m_ui->tableWidgetLccResults->setRowCount(rowCount);
	m_ui->tableWidgetLccResults->setSortingEnabled(false);


	for(unsigned int i=0; i<rowCount; ++i) {

		unsigned int index = i == 0 ? 0 : i-1;

		double priceEnergyIncreaseBefore = std::pow(1.0 + lccSettings.m_para[VICUS::LccSettings::P_PriceIncreaseEnergy].value, index);
		double priceMaterialIncreaseBefore = std::pow(1.0 + lccSettings.m_para[VICUS::LccSettings::P_PriceIncreaseGeneral].value, index);

		double priceEnergyIncrease = std::pow(1.0 + lccSettings.m_para[VICUS::LccSettings::P_PriceIncreaseEnergy].value, i);
		double priceMaterialIncrease = std::pow(1.0 + lccSettings.m_para[VICUS::LccSettings::P_PriceIncreaseGeneral].value, i);

		double discountingRate = 1/std::pow(1.0 + lccSettings.m_para[VICUS::LccSettings::P_DiscountingInterestRate].value, i);


		tab.setItem(i, ColYear, new QTableWidgetItem(QString::number(i+1)));

		tab.setItem(i, ColDiscountingRate, new QTableWidgetItem( QString( "%1 %" ).arg( discountingRate, 7, 'f', 2 ) ));

		tab.setItem(i, ColPriceIncreaseEnergy, new QTableWidgetItem( QString( "%1 %" ).arg( priceEnergyIncrease, 7, 'f', 2 ) ));
		tab.setItem(i, ColPriceIncreaseGeneral, new QTableWidgetItem(QString( "%1 %" ).arg( priceMaterialIncrease, 7, 'f', 2 )));

		tab.setItem(i, ColPriceInvestMaterial, new QTableWidgetItem( QString( "%1 €" ).arg( discountingRate * priceMaterialIncreaseBefore * totalMaterialCost[i], 7, 'f', 2 )));
		tab.setItem(i, ColPriceInvestEnergy,   new QTableWidgetItem( QString( "%1 €" ).arg( discountingRate * priceEnergyIncreaseBefore   * totalEnergyCost  , 7, 'f', 2 )));

		tab.item(i, ColPriceInvestMaterial)->setTextAlignment(Qt::AlignRight);
		tab.item(i, ColPriceInvestEnergy)->setTextAlignment(Qt::AlignRight);

		for(unsigned int j=0; j<NumColLcc; ++j) {
			QTableWidgetItem &item = *tab.item(i, j);
			item.setFlags(item.flags() & ~Qt::ItemIsEditable);
			item.setTextAlignment(Qt::AlignRight);
		}
	}


	m_ui->tableWidgetLccResults->resizeColumnsToContents();
	m_ui->tableWidgetLccResults->resizeRowsToContents();
}


void SVSimulationLCAResultsDialog::setup() {
	// Add data to treeWidget
	m_ui->treeWidgetLcaResults->clear();
	m_ui->treeWidgetLcaResults->setColumnCount(NumCol);
	QStringList headers;
	headers << "Category" << "" << "Type" << "Name" << "Name" << "EPD" << "Amount" << "Invest-Cost [€]" << "GWP (CO2-Äqu.) [kg/(m2a)";
	headers << "ODP (R11-Äqu.) [kg/(m2a)]" << "POCP (C2H4-Äqu.) [kg/(m2a)]" << "AP (SO2-Äqu.) [kg/(m2a)]" << "EP (PO4-Äqu.) [kg/(m2a)]";

	m_ui->treeWidgetLcaResults->setHeaderLabels(headers);
	m_ui->treeWidgetLcaResults->setAlternatingRowColors(true);

	//SVStyle::formatDatabaseTreeView(m_ui->treeWidgetLcaResults);

	m_ui->treeWidgetLcaResults->setItemsExpandable(true);
	m_ui->tableWidgetLccResults->setColumnCount(NumColLcc);

	QStringList header;
	header << "Year [a]" << "Discounting rate [%]"  << "Price increase general [%]" << "Price increase energy [%]" << "Energy - net present value  [€]" << "Material - net present value  [€]";
	m_ui->tableWidgetLccResults->setHorizontalHeaderLabels(header);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetLccResults);
}

void SVSimulationLCAResultsDialog::on_treeWidgetLcaResults_itemExpanded(QTreeWidgetItem *item) {
	for(unsigned int i=0; i<NumCol; ++i)
		m_ui->treeWidgetLcaResults->resizeColumnToContents(i);

	m_ui->treeWidgetLcaResults->setColumnWidth(ColColor, 20);
}


void SVSimulationLCAResultsDialog::on_treeWidgetLcaResults_itemCollapsed(QTreeWidgetItem *item) {
	for(unsigned int i=0; i<NumCol; ++i)
		m_ui->treeWidgetLcaResults->resizeColumnToContents(i);

	m_ui->treeWidgetLcaResults->setColumnWidth(ColColor, 20);
}

