#include "SVSimulationLCAResultsDialog.h"
#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "QtExt_Conversions.h"
#include "ui_SVSimulationLCAResultsDialog.h"

#include <QTreeWidgetItem>

#include <VICUS_KeywordList.h>

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
												 const VICUS::LcaSettings &settings) const {
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

	VICUS::EpdCategoryDataset epdDataset;

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

		QTreeWidgetItem *item = new QTreeWidgetItem();
		rootItem->addChild(item);
		item->setText(ColComponentType, VICUS::KeywordList::Description("Component::ComponentType", aggregatedTypeData.m_component->m_type));
		item->setText(ColArea, QString::number(aggregatedTypeData.m_area));
		item->setBackgroundColor(ColColor, aggregatedTypeData.m_component->m_color);

		VICUS::EpdCategoryDataset epd = aggregatedTypeData.m_totalEpdData[category].calcTotalEpdByCategory(category, SVProjectHandler::instance().project().m_lcaSettings);
		epdDataset += epd;

		item->setText(ColGWP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_GWP].get_value()));
		item->setText(ColAP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_AP].get_value()));
		item->setText(ColEP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_EP].get_value()));
		item->setText(ColODP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_ODP].get_value()));
		item->setText(ColPOCP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));

		item->setExpanded(false);

		m_ui->treeWidgetLcaResults->addTopLevelItem(item);

		for(const VICUS::Component *comp : aggregatedTypeData.m_additionalComponents) {

			const AggregatedComponentData &aggregatedCompData = compIdToAggregatedData.at(comp->m_id);

			if(usedCompIds.find(comp->m_id) == usedCompIds.end())
				continue; // Skip unused ids

			QStringList lcaData;
			QTreeWidgetItem *itemChild = new QTreeWidgetItem();
			VICUS::EpdCategoryDataset epdChild = aggregatedCompData.m_totalEpdData[category].calcTotalEpdByCategory(category, SVProjectHandler::instance().project().m_lcaSettings);
			epdDataset += epdChild;

			itemChild->setText(ColComponentName, QtExt::MultiLangString2QString(comp->m_displayName));
			itemChild->setText(ColArea, QString::number(aggregatedCompData.m_area));
			itemChild->setBackgroundColor(ColColor, aggregatedCompData.m_component->m_color);

			itemChild->setText(ColGWP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_GWP].get_value()));
			itemChild->setText(ColAP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_AP].get_value()));
			itemChild->setText(ColEP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_EP].get_value()));
			itemChild->setText(ColODP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_ODP].get_value()));
			itemChild->setText(ColPOCP, QString::number(scaleFactor * epd.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));

			item->addChild(itemChild);

		}
	}

	rootItem->setText(ColGWP, QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_GWP].get_value()));
	rootItem->setText(ColAP, QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_AP].get_value()));
	rootItem->setText(ColEP, QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_EP].get_value()));
	rootItem->setText(ColODP, QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_ODP].get_value()));
	rootItem->setText(ColPOCP, QString::number(scaleFactor * epdDataset.m_para[VICUS::EpdCategoryDataset::P_POCP].get_value()));

	for(unsigned int i=0; i<NumCol; ++i)
		m_ui->treeWidgetLcaResults->resizeColumnToContents(i);

	m_ui->treeWidgetLcaResults->setColumnWidth(ColColor, 20);
}


void SVSimulationLCAResultsDialog::setup() {
	// Add data to treeWidget
	m_ui->treeWidgetLcaResults->clear();
	m_ui->treeWidgetLcaResults->setColumnCount(NumCol);
	QStringList headers;
	headers << "Category" << "" << "Component type" << "Component name" << "Area [m2]" << "GWP (CO2-Äqu.) [kg/(m2a)";
	headers << "ODP (R11-Äqu.) [kg/(m2a)]" << "POCP (C2H4-Äqu.) [kg/(m2a)]" << "AP (SO2-Äqu.) [kg/(m2a)]" << "EP (PO4-Äqu.) [kg/(m2a)]";

	m_ui->treeWidgetLcaResults->setHeaderLabels(headers);

	SVStyle::formatDatabaseTreeView(m_ui->treeWidgetLcaResults);

	m_ui->treeWidgetLcaResults->setItemsExpandable(true);

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

