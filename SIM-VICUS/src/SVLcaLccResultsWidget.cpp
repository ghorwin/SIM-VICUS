#include "SVLcaLccResultsWidget.h"
#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "QtExt_Conversions.h"
#include "ui_SVLcaLccResultsWidget.h"

#include <QTreeWidgetItem>
#include <VICUS_KeywordList.h>


double SVLcaLccResultsWidget::conversionFactorEpdReferenceUnit(const IBK::Unit & refUnit, const VICUS::Material &layerMat,
																double layerThickness, double layerArea){
	if(refUnit.name() == "kg")
		return layerArea * layerThickness * layerMat.m_para[VICUS::Material::P_Density].get_value("kg/m3"); // area * thickness * density --> layer mass

	if(refUnit.name() == "m2")
		return layerArea;

	if(refUnit.name() == "m3")
		return layerArea * layerThickness;

	if(refUnit.name() == "-")
		return 1.0; // Pieces are always set to 1 for now

	if(refUnit.name() == "MJ")
		return 1.0; // Also not implemented yet

	if(refUnit.name() == "kg/m3")
		return layerMat.m_para[VICUS::Material::P_Density].get_value("kg/m3");

	if(refUnit.name() == "a")
		return 50.0; // Also not implemented yet

	// if no conversion takes place
	return -99;
}


SVLcaLccResultsWidget::SVLcaLccResultsWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVLcaLccResultsWidget)
{
	m_ui->setupUi(this);
}


SVLcaLccResultsWidget::~SVLcaLccResultsWidget() {
	delete m_ui;
}


void SVLcaLccResultsWidget::setLcaResults(const std::map<VICUS::Component::ComponentType, AggregatedComponentData> &lcaResultMap,
												 const std::map<unsigned int, AggregatedComponentData> compIdToAggregatedData,
												 const VICUS::EpdDataset::Category &category,
												 const VICUS::LcaSettings &settings,
												 std::vector<double> &investCost) const {
	// ToDo Stephan: Proper result handling and viewing of lca results
}

void SVLcaLccResultsWidget::setUsageResults(const VICUS::LcaSettings &settings, double gasConsumption,
											double electricityConsumption, double coalConsumption) {
	// ToDo Stephan: Proper result handling and viewing of lca results
}


void SVLcaLccResultsWidget::setCostResults(const VICUS::LccSettings &lccSettings, const VICUS::LcaSettings &lcaSettings,
										   double electricityCost, double coalCost, double gasCost,
										   const std::vector<double> &totalMaterialCost) {
	// ToDo Stephan: Proper result handling and viewing of lcc results
}


void SVLcaLccResultsWidget::setup() {
	// Add data to treeWidget
	m_ui->treeWidgetLcaResults->clear();
	m_ui->treeWidgetLcaResults->setColumnCount(NumCol);
	QStringList headersLca;
	headersLca << tr("Category") << tr("Type") << tr("") << tr("Component") << tr("Material") << tr("EPD") << tr("Quantity") << tr("Invest-Cost") << tr("GWP (CO2-Äqu.)\n[kg/(m2a)]");
	headersLca << tr("ODP (R11-Äqu.)\n[kg/(m2a)]") << tr("POCP (C2H4-Äqu.)\n[kg/(m2a)]") << tr("AP (SO2-Äqu.)\n[kg/(m2a)]") << tr("EP (PO4-Äqu.)\n[kg/(m2a)]");

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
			   << tr("Energy - net present value [€]")
			   << tr("Material costs [€]")
			   << tr("Material - net present value  [€]")
			   << tr("Price increase general [%]")
			   << tr("Price increase energy [%]");
	m_ui->tableWidgetLccResults->setHorizontalHeaderLabels(headersLcc);

	m_ui->tableWidgetLccOverview->setColumnCount(NUM_LCCS);
	QStringList headerLccOverview;
	headerLccOverview << tr("Name") << tr("Production") << tr("Energy")	<< tr("Total");
	m_ui->tableWidgetLccOverview->setHorizontalHeaderLabels(headerLccOverview);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetLccResults);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetLccOverview);

	m_ui->tableWidgetLccOverview->setSortingEnabled(false);
}

void SVLcaLccResultsWidget::on_treeWidgetLcaResults_itemExpanded(QTreeWidgetItem */*item*/) {
	for(unsigned int i=0; i<NumCol; ++i)
		m_ui->treeWidgetLcaResults->resizeColumnToContents(i);

	m_ui->treeWidgetLcaResults->setColumnWidth(ColColor, 20);
}


void SVLcaLccResultsWidget::on_treeWidgetLcaResults_itemCollapsed(QTreeWidgetItem */*item*/) {
	for(unsigned int i=0; i<NumCol; ++i)
		m_ui->treeWidgetLcaResults->resizeColumnToContents(i);

	m_ui->treeWidgetLcaResults->setColumnWidth(ColColor, 20);
}


