#include "SVLcaLccSettingsWidget.h"
#include "SVUndoModifyLcaLcc.h"
#include "ui_SVLcaLccSettingsWidget.h"

#include "IBKMK_3DCalculations.h"

// SIM-VIUCS
#include "SVSettings.h"
#include "SVDatabase.h"
#include "SVDatabaseEditDialog.h"
#include "SVDBEpdTableModel.h"
#include "SVProjectHandler.h"
#include "SVMainWindow.h"
#include "SVLcaLccResultsWidget.h"

// IBK
#include <IBK_Parameter.h>
#include <IBK_FileReader.h>
#include <IBK_StopWatch.h>

// VICUS
#include <VICUS_KeywordList.h>
#include <VICUS_Project.h>
#include <VICUS_EpdDataset.h>
#include <VICUS_EpdModuleDataset.h>

// Qt-Ext
#include <QtExt_Conversions.h>

// Qt
#include <QProgressDialog>

// std
#include <fstream>


SVLcaLccSettingsWidget::SVLcaLccSettingsWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVLcaLccSettingsWidget)
{
	m_ui->setupUi(this);
	layout()->setContentsMargins(0,0,0,0);

	m_db = &SVSettings::instance().m_db;

	m_ui->checkBoxA1->setProperty("category", (int)VICUS::EpdModuleDataset::M_A1);
	m_ui->checkBoxA2->setProperty("category", (int)VICUS::EpdModuleDataset::M_A2);
	m_ui->checkBoxA3->setProperty("category", (int)VICUS::EpdModuleDataset::M_A3);
	m_ui->checkBoxA4->setProperty("category", (int)VICUS::EpdModuleDataset::M_A4);
	m_ui->checkBoxA5->setProperty("category", (int)VICUS::EpdModuleDataset::M_A5);
	m_ui->checkBoxB1->setProperty("category", (int)VICUS::EpdModuleDataset::M_B1);
	m_ui->checkBoxB2->setProperty("category", (int)VICUS::EpdModuleDataset::M_B2);
	m_ui->checkBoxB3->setProperty("category", (int)VICUS::EpdModuleDataset::M_B3);
	m_ui->checkBoxB4->setProperty("category", (int)VICUS::EpdModuleDataset::M_B4);
	m_ui->checkBoxB5->setProperty("category", (int)VICUS::EpdModuleDataset::M_B5);
	m_ui->checkBoxB6->setProperty("category", (int)VICUS::EpdModuleDataset::M_B6);
	m_ui->checkBoxB7->setProperty("category", (int)VICUS::EpdModuleDataset::M_B7);
	m_ui->checkBoxC1->setProperty("category", (int)VICUS::EpdModuleDataset::M_C1);
	m_ui->checkBoxC2->setProperty("category", (int)VICUS::EpdModuleDataset::M_C2);
	m_ui->checkBoxC3->setProperty("category", (int)VICUS::EpdModuleDataset::M_C3);
	m_ui->checkBoxC4->setProperty("category", (int)VICUS::EpdModuleDataset::M_C4);
	m_ui->checkBoxD ->setProperty("category", (int)VICUS::EpdModuleDataset::M_D);


	m_ui->checkBoxA1->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_A1));
	m_ui->checkBoxA2->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_A2));
	m_ui->checkBoxA3->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_A3));
	m_ui->checkBoxA4->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_A4));
	m_ui->checkBoxA5->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_A5));

	m_ui->checkBoxB1->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_B1));
	m_ui->checkBoxB2->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_B2));
	m_ui->checkBoxB3->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_B3));
	m_ui->checkBoxB4->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_B4));
	m_ui->checkBoxB5->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_B5));
	m_ui->checkBoxB6->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_B6));
	m_ui->checkBoxB7->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_B7));

	m_ui->checkBoxC1->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_C1));
	m_ui->checkBoxC2->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_C2));
	m_ui->checkBoxC3->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_C3));
	m_ui->checkBoxC4->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_C4));

	m_ui->checkBoxD->setText(VICUS::KeywordList::Description("EpdModuleDataset::Module", VICUS::EpdModuleDataset::M_D));

	connect(m_ui->checkBoxA1, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxA2, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxA3, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxA4, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxA5, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxB1, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxB2, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxB3, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxB4, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxB5, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxB6, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxB7, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxC1, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxC2, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxC3, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxC4, &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);
	connect(m_ui->checkBoxD,  &QCheckBox::stateChanged, this, &SVLcaLccSettingsWidget::setModuleState);

	m_ui->comboBoxCalculationMode->blockSignals(true);
	m_ui->comboBoxCertificationSystem->blockSignals(true);
	for(int i=0; i<VICUS::LcaSettings::NUM_CM; ++i)
		m_ui->comboBoxCalculationMode->addItem(VICUS::KeywordList::Description("LcaSettings::CalculationMode", i), i);

	for(int i=0; i<VICUS::LcaSettings::NUM_CS; ++i)
		m_ui->comboBoxCertificationSystem->addItem(VICUS::KeywordList::Description("LcaSettings::CertificationSytem", i), i);

	m_ui->comboBoxCalculationMode->blockSignals(false);
	m_ui->comboBoxCertificationSystem->blockSignals(false);

	m_ui->lineEditArea->setup(0, 1e10, "Net usage area of Building(s)", false, true);
	m_ui->lineEditPriceIncreaseEnergy->setup(0, 100, "Energy Price increase", true, true);
	m_ui->lineEditPriceIncreaseGeneral->setup(0, 100, "General Price increase", true, true);

	m_ui->lineEditGasPrice->setup(0, 1e10, "Gas price for evaluation in €/kWh", false, true);
	m_ui->lineEditCoalPrice->setup(0, 1e10, "Coal price for evaluation in €/kWh", false, true);
	m_ui->lineEditElectricityPrice->setup(0, 1e10, "Electricity price for evaluation in €/kWh", false, true);

	m_resultsWidget = new SVLcaLccResultsWidget();
	m_ui->tabWidget->addTab(m_resultsWidget, "LCC/LCA results");

	m_ui->tabWidget->setTabEnabled(3, false);
	m_ui->tabWidget->setCurrentIndex(0);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVLcaLccSettingsWidget::onModified);
}


SVLcaLccSettingsWidget::~SVLcaLccSettingsWidget() {
	delete m_ui;
}


void SVLcaLccSettingsWidget::calculateLCA() {
	FUNCID(SVLcaLccSettingsWidget::calculateLCA);

//	IBK::Path path(m_ui->filepathResults->filename().toStdString());
//	QString filename = m_ui->lineEditResultName->text();
//	IBK::Path file(filename.toStdString());
//	file.addExtension(".txt");
//	path /= file;

//	if(!path.isValid()) {
//		QMessageBox::warning(this, tr("Invalid Result path or file"), tr("Define a valid result path and filename first before calculating LCA."));
//		return;
//	}


	/// 1) Aggregate all used components from project and sum up all their areas
	/// 2) go through all layers and their referenced epds and use the epds reference unit for global calculation
	/// 3) Global calculation means that layer data needs to be converted in a manner that it coresponds to reference unit
	/// 4) Now calculate the final LCA Data for each component and material layer using converted material data and reference unit
	/// 5) Aggregate data for each component type --> such as floor, etc.

	// Reset all LCA Data from previous calculations.
	resetLcaData();

	// First we aggregate all data for used components
	aggregateProjectComponents();

	// Calculate all total EPD Data for Components
	calculateTotalLcaDataForComponents();

	// Calculates all usage specific data
	// calculateUsageSpecificData();

	// Aggregate all data by type of component
	aggregateAggregatedComponentsByType();

	// Write calculation to file
	//writeLcaDataToTxtFile(path);

	//m_ui->tabWidget->setCurrentIndex(0);
	m_ui->tabWidget->setTabEnabled(3, true);
}


bool SVLcaLccSettingsWidget::convertString2Val(double &val, std::string &text, unsigned int row, unsigned int column) {
	if(text == "") {
		val = 0.0;
		return true;
	}

	std::replace( text.begin(), text.end(), ',', '.'); // replace all ',' to '.'

	try {
		val = IBK::string2val<double>(text);
	}  catch (IBK::Exception &ex) {
		IBK::IBK_Message(IBK::FormatString("%4\nCould not convert string '%1' of row %2 and column %3")
						 .arg(text).arg(row+1).arg(column+1).arg(ex.what()));
		return false;
	}

	return true;
}


template<typename T>
void SVLcaLccSettingsWidget::setValue(T &member, const T &value, bool foundExistingEpd) {
	if(!foundExistingEpd)
		member = value;
	else {
		if(member != value) {
			qDebug() << "Error between already defined values of EPD '" << member << "' <> '" << value << "'";
		}
	}
}



void SVLcaLccSettingsWidget::importOkoebauDat(const IBK::Path & csvPath) {
	FUNCID(SVDBEPDEditWidget::importOkoebauDat);

	// Read csv with ÖKOBAUDAT
	std::vector< std::string > dataLines;

	if (SVSettings::instance().showDoNotShowAgainQuestion(this, "reloading-oekobaudat",
														  tr("Reloading ÖKOBAUDAT"),
														  tr("Reloading ÖKOBAUDAT will delete all references to currently existing EPDs in Database. Continue?"),
														  QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
		return;

	// Explode all lines
	if (IBK::FileReader::readAll(csvPath, dataLines, std::vector<std::string>()) == -1)
		throw IBK::Exception("Error reading csv-file with ÖKÖBAUDAT!", FUNC_ID);

	// Remove header
	dataLines.erase(dataLines.begin());

	// extract vector of string-xy-pairs
	std::vector<std::string> tokens;

	QProgressDialog *dlg = new QProgressDialog(tr("Importing EPD-Database"), tr("Stop"), 0, (int)dataLines.size(), this);

	std::map<QString, VICUS::EpdDataset> dataSets;

	IBK::StopWatch timer;
	timer.start();

	std::map<std::string, std::string> oekobauDatUnit2IBKUnit;
	oekobauDatUnit2IBKUnit["qm"] = "m2";
	oekobauDatUnit2IBKUnit["pcs."] = "-";
	oekobauDatUnit2IBKUnit["kgkm"] = "kg/m3";
	oekobauDatUnit2IBKUnit["MJ"] = "MJ";
	oekobauDatUnit2IBKUnit["m3"] = "m3";
	oekobauDatUnit2IBKUnit["m"] = "m";
	oekobauDatUnit2IBKUnit["kg"] = "kg";
	oekobauDatUnit2IBKUnit["a"] = "a";
	oekobauDatUnit2IBKUnit[""] = "-";

	unsigned int id = 1090000;

	for (unsigned int row = 0; row<dataLines.size(); ++row) {
		std::string &line = dataLines[row];
		//		IBK::trim(line, ",");
		//		IBK::trim(line, "\"");
		//		IBK::trim(line, "MULTILINESTRING ((");
		//		IBK::trim(line, "))");
		IBK::explode(line, tokens, ";", IBK::EF_KeepEmptyTokens);

		VICUS::EpdDataset *epd = nullptr;
		VICUS::EpdModuleDataset *epdCategoryDataSet = new VICUS::EpdModuleDataset;

		// convert this vector to double and add it as a graph
		std::vector<std::vector<double> > polyLine;
		for (unsigned int col = 0; col < tokens.size(); ++col){

			bool foundExistingEpd = false;
#if defined(win32)
			std::string t = IBK::ANSIToUTF8String(tokens[col]);
#else
			std::string t = tokens[col];

			std::string strOut;
			for (std::string::iterator it = t.begin(); it != t.end(); ++it)
			{
				uint8_t ch = *it;
				if (ch < 0x80) {
					strOut.push_back(ch);
				}
				else {
					strOut.push_back(0xc0 | ch >> 6);
					strOut.push_back(0x80 | (ch & 0x3f));
				}
			}
			t = strOut;
#endif

			if(timer.difference() > 200) {
				dlg->setValue(row);
				qApp->processEvents();
			}

			if(dlg->wasCanceled()) {
				IBK::IBK_Message(IBK::FormatString("EPD-Import has been interrupted by user."));
				return;
			}

			// qDebug() << "Row: " << row << " Column: " << col << " Text: " << QString::fromStdString(t);

			switch (col) {
			// Not imported coloumns
			case ColVersion:
			case ColConformity:
			case ColCountryCode:
			case ColReferenceYear:
			case ColPublishedOn:
			case ColRegistrationNumber:
			case ColRegistrationBody:
			case ColUUIDOfThePredecessor: {
				if(t == "")
					continue;
			} break;

			case ColUUID: {
				if(t == "")
					continue;

				QString uuid = QString::fromStdString(t);

				// do we already have an EPD with the specific UUID
				if(dataSets.find(uuid) != dataSets.end()) { // We found one
					epd = &dataSets[uuid];
					foundExistingEpd = true;
				}
				else { // Create new one
					dataSets[uuid] = VICUS::EpdDataset();
					epd = &dataSets[uuid];
					epd->m_uuid = uuid;
					epd->m_id = id++;
				}

				setValue<QString>(epd->m_uuid, uuid, foundExistingEpd);

			} break;
			case ColNameDe:
				if(t == "")
					continue;
				epd->m_displayName.setString(t, "De");
				break;
			case ColNameEn:
				if(t == "")
					continue;
				epd->m_displayName.setString(t, "En");
			break;
			case ColCategoryDe:
				if(t == "")
					continue;
				epd->m_category.setString(t, "De");
			break;
			case ColCategoryEn:
				if(t == "")
					continue;
				epd->m_category.setString(t, "En");
			break;
			case ColType: {
				if(t == "")
					continue;

				VICUS::EpdDataset::Type type = VICUS::EpdDataset::NUM_T;

				if (t == "average dataset")
					type = VICUS::EpdDataset::T_Average;
				else if (t == "specific dataset")
					type = VICUS::EpdDataset::T_Specific;
				else if (t == "representative dataset")
					type = VICUS::EpdDataset::T_Representative;
				else if (t == "generic dataset")
					type = VICUS::EpdDataset::T_Generic;
				else if (t == "template dataset")
					type = VICUS::EpdDataset::T_Template;

				setValue<VICUS::EpdDataset::Type>(epd->m_type, type, foundExistingEpd);

			} break;
			case ColExpireYear:			epd->m_expireYear = QString::fromStdString(t);			break;
			case ColDeclarationOwner:	epd->m_manufacturer = QString::fromStdString(t);			break;
			case ColReferenceSize: {
				if(t == "" || t == "not available")
					continue;

				double val;
				if(!convertString2Val(val, t, row, col))
					continue;


				setValue<double>(epd->m_referenceQuantity, val, foundExistingEpd);
			} break;
			case ColReferenceUnit: {
				if(t == "")
					continue;

				if (oekobauDatUnit2IBKUnit.find(t) == oekobauDatUnit2IBKUnit.end())
					continue;

				if(!foundExistingEpd) {
					IBK::Unit unit(oekobauDatUnit2IBKUnit[t]);
					epd->m_referenceUnit = unit;
				}
				else {
					if(epd->m_referenceUnit.name() != oekobauDatUnit2IBKUnit[t])
						qDebug() << "Units do not match";
				}

			} break;
			case ColURL: {
				if(t == "")
					continue;

				QString string = QString::fromStdString(t);
				setValue<QString>(epd->m_dataSource, string, foundExistingEpd);
			} break;


			case ColModule: {
				if(t == "")
					continue;

				std::vector<VICUS::EpdModuleDataset::Module> modules;
				std::string moduleType = t.substr(0, 1);

				if(!epd->m_modules.isEmpty()) {
					if(epd->m_modules.indexOf(moduleType.c_str(), 0) == -1)
						epd->m_modules = QString("%1, %2").arg(epd->m_modules).arg(QString::fromStdString(moduleType));
				}
				else
					epd->m_modules = QString::fromStdString(moduleType);

				if (t == "A1")
					modules.push_back(VICUS::EpdModuleDataset::M_A1);
				else if (t == "A2")
					modules.push_back(VICUS::EpdModuleDataset::M_A2);
				else if (t == "A3")
					modules.push_back(VICUS::EpdModuleDataset::M_A3);
				else if (t == "A1-A2") {
					modules.push_back(VICUS::EpdModuleDataset::M_A1);
					modules.push_back(VICUS::EpdModuleDataset::M_A2);
				}
				else if (t == "A1-A3") {
					modules.push_back(VICUS::EpdModuleDataset::M_A1);
					modules.push_back(VICUS::EpdModuleDataset::M_A2);
					modules.push_back(VICUS::EpdModuleDataset::M_A3);
				}
				else if (t == "A4")
					modules.push_back(VICUS::EpdModuleDataset::M_A4);
				else if (t == "A5")
					modules.push_back(VICUS::EpdModuleDataset::M_A5);
				else if (t == "B1")
					modules.push_back(VICUS::EpdModuleDataset::M_B1);
				else if (t == "B2")
					modules.push_back(VICUS::EpdModuleDataset::M_B2);
				else if (t == "B3")
					modules.push_back(VICUS::EpdModuleDataset::M_B3);
				else if (t == "B4")
					modules.push_back(VICUS::EpdModuleDataset::M_B4);
				else if (t == "B5")
					modules.push_back(VICUS::EpdModuleDataset::M_B5);
				else if (t == "B6")
					modules.push_back(VICUS::EpdModuleDataset::M_B6);
				else if (t == "B7")
					modules.push_back(VICUS::EpdModuleDataset::M_B7);
				else if (t == "C1")
					modules.push_back(VICUS::EpdModuleDataset::M_C1);
				else if (t == "C2")
					modules.push_back(VICUS::EpdModuleDataset::M_C2);
				else if (t == "C2-C3") {
					modules.push_back(VICUS::EpdModuleDataset::M_C2);
					modules.push_back(VICUS::EpdModuleDataset::M_C3);
				}
				else if (t == "C2-C4") {
					modules.push_back(VICUS::EpdModuleDataset::M_C2);
					modules.push_back(VICUS::EpdModuleDataset::M_C3);
					modules.push_back(VICUS::EpdModuleDataset::M_C4);
				}
				else if (t == "C3")
					modules.push_back(VICUS::EpdModuleDataset::M_C3);
				else if (t == "C3-C4") {
					modules.push_back(VICUS::EpdModuleDataset::M_C3);
					modules.push_back(VICUS::EpdModuleDataset::M_C4);
				}
				else if (t == "C4")
					modules.push_back(VICUS::EpdModuleDataset::M_C4);
				else if (t == "D")
					modules.push_back(VICUS::EpdModuleDataset::M_D);

				epdCategoryDataSet->m_modules = modules;

			} break;


			case ColWeightPerUnitArea: {
				double val;
				if(!convertString2Val(val, t, row, col))
					continue;

				epdCategoryDataSet->m_para[VICUS::EpdModuleDataset::P_AreaDensity].set("AreaDensity", val, IBK::Unit("kg/m2"));
			} break;
			case ColBulkDensity: {
				double val;
				if(!convertString2Val(val, t, row, col))
					continue;
				epdCategoryDataSet->m_para[VICUS::EpdModuleDataset::P_DryDensity].set("DryDensity", val, IBK::Unit("kg/m3"));
			} break;
			case ColGWP: {
				double val;
				if(!convertString2Val(val, t, row, col))
					continue;

				epdCategoryDataSet->m_para[VICUS::EpdModuleDataset::P_GWP].set("GWP", val, IBK::Unit("kg"));
			} break;
			case ColODP: {
				double val;
				if(!convertString2Val(val, t, row, col))
					continue;
				epdCategoryDataSet->m_para[VICUS::EpdModuleDataset::P_ODP].set("ODP", val, IBK::Unit("kg"));
			} break;
			case ColPOCP: {
				double val;
				if(!convertString2Val(val, t, row, col))
					continue;
				epdCategoryDataSet->m_para[VICUS::EpdModuleDataset::P_POCP].set("POCP", val, IBK::Unit("kg"));

			} break;
			case ColAP: {
				double val;
				if(!convertString2Val(val, t, row, col))
					continue;
				epdCategoryDataSet->m_para[VICUS::EpdModuleDataset::P_AP].set("AP", val, IBK::Unit("kg"));
			} break;
			case ColEP: {
				double val;
				if(!convertString2Val(val, t, row, col))
					continue;
				epdCategoryDataSet->m_para[VICUS::EpdModuleDataset::P_EP].set("EP", val, IBK::Unit("kg"));
			} break;
			case ColPENRT: {
				double val;
				if(!convertString2Val(val, t, row, col))
					continue;
				epdCategoryDataSet->m_para[VICUS::EpdModuleDataset::P_PENRT].set("PENRT", val, IBK::Unit("W/mK"));
			} break;
			case ColPERT: {
				double val;
				if(!convertString2Val(val, t, row, col))
					continue;
				epdCategoryDataSet->m_para[VICUS::EpdModuleDataset::P_PERT].set("PERT", val, IBK::Unit("W/mK"));
			} break;
			}
		}
		epd->m_epdModuleDataset.push_back(*epdCategoryDataSet);

		if(epd->m_manufacturer.isEmpty())
			qDebug() << "Found emtpty epd: " << epd->m_uuid;
	}

	dlg->setValue(dataLines.size());
	// Get pointer to our EPD edit widget
	SVDatabaseEditDialog *conEditDialog = SVMainWindow::instance().dbEpdEditDialog();
	Q_ASSERT(conEditDialog != nullptr);

	// now we import all Datasets
	dynamic_cast<SVDBEpdTableModel*>(conEditDialog->dbModel())->importDatasets(dataSets);
}


void SVLcaLccSettingsWidget::addComponentInstance(const VICUS::ComponentInstance & compInstance) {
	if(m_compIdToAggregatedData.find(compInstance.m_idComponent) == m_compIdToAggregatedData.end())
		m_compIdToAggregatedData[compInstance.m_idComponent] = AggregatedComponentData(compInstance);
	else
		m_compIdToAggregatedData[compInstance.m_idComponent].addArea(compInstance);
}


void SVLcaLccSettingsWidget::aggregateProjectComponents() {
	QStringList lifetime, cost, epd;

	for (const VICUS::ComponentInstance &ci : project().m_componentInstances) {
		// Add CI to aggregated data map
		addComponentInstance(ci);

		const VICUS::Component *comp = m_db->m_components[ci.m_idComponent];
		if(comp == nullptr)
			continue;

		const VICUS::Construction *con = m_db->m_constructions[comp->m_idConstruction];
		if(con == nullptr)
			continue;

		for(const VICUS::MaterialLayer &matLayer : con->m_materialLayers) {
			// Check Cost & lifetime of used Materials
			bool isLifetimeDefined = !matLayer.m_para[VICUS::MaterialLayer::P_LifeTime].empty();
			bool isCostDefined = !matLayer.m_cost.empty();
			// Check EPDs of used Materials
			const VICUS::Material *mat = m_db->m_materials[matLayer.m_idMaterial];

			if(mat == nullptr)
				continue;

			bool isEPDDefined = !mat->m_epdCategorySet.isEmpty();

			if(!isLifetimeDefined)
				lifetime << QString::fromStdString(mat->m_displayName.string());
			if(!isCostDefined)
				cost << QString::fromStdString(mat->m_displayName.string());
			if(!isEPDDefined) {
				epd << QString::fromStdString(mat->m_displayName.string());
				m_idComponentEpdUndefined.insert(comp->m_id);
			}
		}
	}

	lifetime.removeDuplicates();
	cost.removeDuplicates();
	epd.removeDuplicates();


//	QString messageText(tr("Lifetime:\t%1\nCost:\t%2\nEPD:\t%3\n\nProceed and skip all components without needed Data?")
//						.arg(lifetime.join("\n\t\t"))
//						.arg(cost.join("\n\t\t"))
//						.arg(epd.join("\n\t\t")));
//	if(QMessageBox::warning(this, "LCA/LCC Information is missing", messageText) == QMessageBox::Cancel)
//		return;
}

void SVLcaLccSettingsWidget::aggregateAggregatedComponentsByType() {
	for(std::map<unsigned int, AggregatedComponentData>::iterator itAggregatedComp = m_compIdToAggregatedData.begin();
		itAggregatedComp != m_compIdToAggregatedData.end(); ++itAggregatedComp)
	{
		const AggregatedComponentData &aggregatedData = itAggregatedComp->second;
		if(aggregatedData.m_component == nullptr)
			continue;

		if(m_typeToAggregatedCompData.find(aggregatedData.m_component->m_type) == m_typeToAggregatedCompData.end())
			m_typeToAggregatedCompData[aggregatedData.m_component->m_type] = aggregatedData;
		else
			m_typeToAggregatedCompData[aggregatedData.m_component->m_type].addAggregatedData(aggregatedData);
	}
}


void SVLcaLccSettingsWidget::writeDataToStream(std::ofstream &lcaStream, const std::string &categoryText,
											   const VICUS::EpdDataset::Category &/*category*/) {

	lcaStream << categoryText + "\t\t\t\t\t\t\t"  << std::endl;

	for(std::map<VICUS::Component::ComponentType, AggregatedComponentData>::iterator itAggregatedComp = m_typeToAggregatedCompData.begin();
		itAggregatedComp != m_typeToAggregatedCompData.end(); ++itAggregatedComp)
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

		lcaDataType << "";
		lcaDataType << VICUS::KeywordList::Description("Component::ComponentType", aggregatedTypeData.m_component->m_type);
		lcaDataType << "";
		lcaDataType << QString::number(aggregatedTypeData.m_area);
//		lcaDataType << QString::number(aggregatedTypeData.m_totalEpdData[category].m_para[VICUS::EpdDataset::P_GWP].get_value());
//		lcaDataType << QString::number(aggregatedTypeData.m_totalEpdData[category].m_para[VICUS::EpdDataset::P_ODP].get_value());
//		lcaDataType << QString::number(aggregatedTypeData.m_totalEpdData[category].m_para[VICUS::EpdDataset::P_POCP].get_value());
//		lcaDataType << QString::number(aggregatedTypeData.m_totalEpdData[category].m_para[VICUS::EpdDataset::P_AP].get_value());
//		lcaDataType << QString::number(aggregatedTypeData.m_totalEpdData[category].m_para[VICUS::EpdDataset::P_EP].get_value());

		lcaStream << lcaDataType.join("\t").toStdString() << std::endl;

		for(const VICUS::Component *comp : aggregatedTypeData.m_additionalComponents) {

			const AggregatedComponentData &aggregatedCompData = m_compIdToAggregatedData[comp->m_id];

			if(usedCompIds.find(comp->m_id) == usedCompIds.end())
				continue; // Skip unused ids

			QStringList lcaData;

			lcaData << "";
			lcaData << "";
			lcaData << QtExt::MultiLangString2QString(comp->m_displayName);
			lcaData << QString::number(aggregatedCompData.m_area);
			//			lcaData << QString::number(aggregatedCompData.m_totalEpdData[category].m_para[VICUS::EpdDataset::P_GWP].get_value());
			//			lcaData << QString::number(aggregatedCompData.m_totalEpdData[category].m_para[VICUS::EpdDataset::P_ODP].get_value());
			//			lcaData << QString::number(aggregatedCompData.m_totalEpdData[category].m_para[VICUS::EpdDataset::P_POCP].get_value());
			//			lcaData << QString::number(aggregatedCompData.m_totalEpdData[category].m_para[VICUS::EpdDataset::P_AP].get_value());
			//			lcaData << QString::number(aggregatedCompData.m_totalEpdData[category].m_para[VICUS::EpdDataset::P_EP].get_value());

			lcaStream << lcaData.join("\t").toStdString() << std::endl;

		}

	}
}


void SVLcaLccSettingsWidget::setModuleState(int state) {
	QCheckBox *cb = dynamic_cast<QCheckBox*>(sender());
	Q_ASSERT(cb != nullptr);
	VICUS::EpdModuleDataset::Module mod = static_cast<VICUS::EpdModuleDataset::Module>(cb->property("category").toInt());

	VICUS::Project p = project();
	p.m_lcaSettings.m_flags[mod].set(p.m_lcaSettings.m_flags[mod].name(), state == Qt::Checked);
	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", p.m_lcaSettings, p.m_lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::setCheckBoxState(QCheckBox *cb, int bitmask) {
	cb->blockSignals(true);
	cb->setChecked((project().m_lcaSettings.m_certificationModules & bitmask) == bitmask);
	cb->blockSignals(false);
}



void SVLcaLccSettingsWidget::updateUi() {
	FUNCID(SVLcaLccSettingsWidget::updateUi);

	m_ui->groupBoxCatA->blockSignals(true);
	m_ui->groupBoxCatB->blockSignals(true);
	m_ui->groupBoxCatC->blockSignals(true);
	m_ui->groupBoxCatD->blockSignals(true);

	if (!SVProjectHandler::instance().isValid())
		return;

	const VICUS::LcaSettings &lcaSettings = project().m_lcaSettings;
	const VICUS::LccSettings &lccSettings = project().m_lccSettings;

	m_ui->lineEditArea->setText(QString("%1").arg(lcaSettings.m_para[VICUS::LcaSettings::P_NetUsageArea].get_value("m2")));

	int years = lcaSettings.m_para[VICUS::LcaSettings::P_TimePeriod].get_value("a");
	m_ui->spinBoxTimePeriod->setValue(years);

	m_ui->lineEditInterestRate->setText(QString("%1").arg(lccSettings.m_para[VICUS::LccSettings::P_DiscountingInterestRate].get_value("%")));
	m_ui->lineEditPriceIncreaseEnergy->setText(QString("%1").arg(lccSettings.m_para[VICUS::LccSettings::P_PriceIncreaseEnergy].get_value("%")));
	m_ui->lineEditPriceIncreaseGeneral->setText(QString("%1").arg(lccSettings.m_para[VICUS::LccSettings::P_PriceIncreaseGeneral].get_value("%")));

	m_ui->lineEditCoalConsumption->setText(QString("%1").arg(lccSettings.m_para[VICUS::LccSettings::P_CoalConsumption].get_value("kWh/a")));
	m_ui->lineEditGasConsumption->setText(QString("%1").arg(lccSettings.m_para[VICUS::LccSettings::P_GasConsumption].get_value("kWh/a")));
	m_ui->lineEditElectricityConsumption->setText(QString("%1").arg(lccSettings.m_para[VICUS::LccSettings::P_ElectricityConsumption].get_value("kWh/a")));

	m_ui->filepathOekoBauDat->setup(tr("Select csv with ÖKOBAUDAT"), true, true, tr("ÖKOBAUDAT-csv (*.csv)"),
									SVSettings::instance().m_dontUseNativeDialogs);

	// m_ui->filepathResults->setup("Select directory for LCA results", false, true, "", SVSettings::instance().m_dontUseNativeDialogs);

	QObjectList ol;
	ol.push_back(m_ui->checkBoxA1);
	ol.push_back(m_ui->checkBoxA2);
	ol.push_back(m_ui->checkBoxA3);
	ol.push_back(m_ui->checkBoxA4);
	ol.push_back(m_ui->checkBoxA5);
	ol.push_back(m_ui->checkBoxB1);
	ol.push_back(m_ui->checkBoxB2);
	ol.push_back(m_ui->checkBoxB3);
	ol.push_back(m_ui->checkBoxB4);
	ol.push_back(m_ui->checkBoxB5);
	ol.push_back(m_ui->checkBoxB6);
	ol.push_back(m_ui->checkBoxB7);
	ol.push_back(m_ui->checkBoxC1);
	ol.push_back(m_ui->checkBoxC2);
	ol.push_back(m_ui->checkBoxC3);
	ol.push_back(m_ui->checkBoxC4);
	ol.push_back(m_ui->checkBoxD);

	//qDebug() << m_lcaSettings->m_lcaCertificationSystem;

	setCheckBoxState(m_ui->checkBoxA1, VICUS::LcaSettings::M_A1);
	setCheckBoxState(m_ui->checkBoxA2, VICUS::LcaSettings::M_A2);
	setCheckBoxState(m_ui->checkBoxA3, VICUS::LcaSettings::M_A3);
	setCheckBoxState(m_ui->checkBoxA4, VICUS::LcaSettings::M_A4);
	setCheckBoxState(m_ui->checkBoxA5, VICUS::LcaSettings::M_A5);

	setCheckBoxState(m_ui->checkBoxB1, VICUS::LcaSettings::M_B1);
	setCheckBoxState(m_ui->checkBoxB2, VICUS::LcaSettings::M_B2);
	setCheckBoxState(m_ui->checkBoxB3, VICUS::LcaSettings::M_B3);
	setCheckBoxState(m_ui->checkBoxB4, VICUS::LcaSettings::M_B4);
	setCheckBoxState(m_ui->checkBoxB5, VICUS::LcaSettings::M_B5);
	setCheckBoxState(m_ui->checkBoxB6, VICUS::LcaSettings::M_B6);
	setCheckBoxState(m_ui->checkBoxB7, VICUS::LcaSettings::M_B7);

	setCheckBoxState(m_ui->checkBoxC1, VICUS::LcaSettings::M_C1);
	setCheckBoxState(m_ui->checkBoxC2, VICUS::LcaSettings::M_C2);
	setCheckBoxState(m_ui->checkBoxC3, VICUS::LcaSettings::M_C3);
	setCheckBoxState(m_ui->checkBoxC4, VICUS::LcaSettings::M_C4);

	setCheckBoxState(m_ui->checkBoxD, VICUS::LcaSettings::M_D);

	if(lcaSettings.m_calculationMode == VICUS::LcaSettings::CM_Detailed) {
		for(unsigned int i=0; i < (unsigned int)ol.count(); ++i) {
			QCheckBox *cb = dynamic_cast<QCheckBox*>(ol[i]);
			bool ok;
			VICUS::LcaSettings::Module mod = static_cast<VICUS::LcaSettings::Module>(cb->property("category").toInt(&ok));
			if (!ok)
				throw IBK::Exception(IBK::FormatString("Could not set state of check-box"), FUNC_ID);
			bool isChecked = lcaSettings.m_flags[mod].isEnabled();
			cb->blockSignals(true);
			cb->setChecked(isChecked);
			cb->blockSignals(false);
		}
	}

	m_ui->groupBoxCatA->setEnabled(lcaSettings.m_calculationMode != VICUS::LcaSettings::CM_Simple);
	m_ui->groupBoxCatB->setEnabled(lcaSettings.m_calculationMode != VICUS::LcaSettings::CM_Simple);
	m_ui->groupBoxCatC->setEnabled(lcaSettings.m_calculationMode != VICUS::LcaSettings::CM_Simple);
	m_ui->groupBoxCatD->setEnabled(lcaSettings.m_calculationMode != VICUS::LcaSettings::CM_Simple);

//	m_ui->groupBoxLcaCalc->blockSignals(false);
//	m_ui->groupBoxLccSettings->blockSignals(false);
//	m_ui->groupBoxGeneral->blockSignals(false);

	VICUS::EpdDataset *epdCoal = nullptr;
	VICUS::EpdDataset *epdGas = nullptr;
	VICUS::EpdDataset *epdElectricity = nullptr;

	epdCoal = m_db->m_epdDatasets[lcaSettings.m_idUsage[VICUS::LcaSettings::UT_Coal]];
	if(epdCoal != nullptr)
		m_ui->lineEditEpdCoal->setText(QtExt::MultiLangString2QString(epdCoal->m_displayName));
	else
		m_ui->lineEditEpdCoal->setText("-");

	epdGas = m_db->m_epdDatasets[lcaSettings.m_idUsage[VICUS::LcaSettings::UT_Gas]];
	if(epdGas != nullptr)
		m_ui->lineEditEpdGas->setText(QtExt::MultiLangString2QString(epdGas->m_displayName));
	else
		m_ui->lineEditEpdGas->setText("-");

	epdElectricity = m_db->m_epdDatasets[lcaSettings.m_idUsage[VICUS::LcaSettings::UT_Electricity]];
	if(epdElectricity != nullptr)
		m_ui->lineEditEpdElectricity->setText(QtExt::MultiLangString2QString(epdElectricity->m_displayName));
	else
		m_ui->lineEditEpdElectricity->setText("-");

	m_ui->lineEditCoalPrice->setText( QString( "%1" ).arg( (double)lccSettings.m_intPara[VICUS::LccSettings::IP_CoalPrice].value / 100, 7, 'f', 2 ) );
	m_ui->lineEditElectricityPrice->setText( QString( "%1" ).arg( (double)lccSettings.m_intPara[VICUS::LccSettings::IP_ElectricityPrice].value / 100, 7, 'f', 2 ) );
	m_ui->lineEditGasPrice->setText( QString( "%1" ).arg( (double)lccSettings.m_intPara[VICUS::LccSettings::IP_GasPrice].value / 100, 7, 'f', 2 ) );

	m_ui->groupBoxCatA->blockSignals(false);
	m_ui->groupBoxCatB->blockSignals(false);
	m_ui->groupBoxCatC->blockSignals(false);
	m_ui->groupBoxCatD->blockSignals(false);

}


void SVLcaLccSettingsWidget::onModified(int modificationType, ModificationInfo *) {
	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::LcaLccModified:
			updateUi();
		break;
		default:;
	}
}


void SVLcaLccSettingsWidget::writeLcaDataToTxtFile(const IBK::Path &resultPath) {
	std::ofstream lcaStream(resultPath.str());
	// Write header
	lcaStream << "Category\tComponent type\tComponent name\tArea [m2]\tGWP (CO2-Äqu.) [kg/(m2a)\tODP (R11-Äqu.) [kg/(m2a)]\tPOCP (C2H4-Äqu.) [kg/(m2a)]\tAP (SO2-Äqu.) [kg/(m2a)]\tEP (PO4-Äqu.) [kg/(m2a)]" << std::endl;

	lcaStream << "Goal:\t\t\t\t24\t0.0000001010\t0.0063\t0.0662\t0.0086" << std::endl;


	writeDataToStream(lcaStream, "Category A - Production", VICUS::EpdDataset::C_CategoryA);
	// writeDataToStream(lcaStream, "Category A - Production", AggregatedComponentData::C_CategoryA);
	writeDataToStream(lcaStream, "Category C - Disposal", VICUS::EpdDataset::C_CategoryC);
	writeDataToStream(lcaStream, "Category D - Deposit", VICUS::EpdDataset::C_CategoryD);

	lcaStream.close();

}


void SVLcaLccSettingsWidget::calculateTotalLcaDataForComponents() {
	// Go through all used components.
	// Go through all used material layers in components.

	for(std::map<unsigned int, AggregatedComponentData>::iterator itAggregatedComp = m_compIdToAggregatedData.begin();
		itAggregatedComp != m_compIdToAggregatedData.end(); ++itAggregatedComp)
	{
		if(m_idComponentEpdUndefined.find(itAggregatedComp->first) != m_idComponentEpdUndefined.end())
			continue; // we skip components when there are epds not defined

		double area = itAggregatedComp->second.m_area;
		const VICUS::Component *comp = itAggregatedComp->second.m_component;

		if(comp == nullptr)
			continue;

		qDebug() << QString::fromStdString(comp->m_displayName.string());
		qDebug() << comp->m_idConstruction;

		if(comp == nullptr)
			continue;

		const VICUS::Construction *con = m_db->m_constructions[comp->m_idConstruction];
		if(con == nullptr)
			continue;

		for(const VICUS::MaterialLayer &matLayer : con->m_materialLayers) {
			const VICUS::Material &mat = *m_db->m_materials[matLayer.m_idMaterial];
			const VICUS::EpdDataset *epdCatA = m_db->m_epdDatasets[mat.m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryA]];
			const VICUS::EpdDataset *epdCatB = m_db->m_epdDatasets[mat.m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryB]];
			const VICUS::EpdDataset *epdCatC = m_db->m_epdDatasets[mat.m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryA]];
			const VICUS::EpdDataset *epdCatD = m_db->m_epdDatasets[mat.m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryD]];

			// We have to think about renewing our materials as well
			// If no lifetime is defined, we take for now 50 years
			double lifeTime;
			if(matLayer.m_para[VICUS::MaterialLayer::P_LifeTime].empty())
				lifeTime = 50;
			else
				lifeTime = matLayer.m_para[VICUS::MaterialLayer::P_LifeTime].get_value();

			const VICUS::LcaSettings &lcaSettings = project().m_lcaSettings;

			double renewingFactor = lcaSettings.m_para[VICUS::LcaSettings::P_TimePeriod].get_value() / lifeTime;


			// We do the unit conversion and handling to get all our reference units correctly managed
			if(epdCatA != nullptr)
				itAggregatedComp->second.m_totalEpdData[VICUS::EpdDataset::C_CategoryA]
						= epdCatA->scaleByFactor( renewingFactor *
							SVLcaLccResultsWidget::conversionFactorEpdReferenceUnit(epdCatA->m_referenceUnit,
															 mat, matLayer.m_para[VICUS::MaterialLayer::P_Thickness].get_value("m"), area));
			if(epdCatB != nullptr) // no renewing period scaling since it is already normated for 1 a
				itAggregatedComp->second.m_totalEpdData[VICUS::EpdDataset::C_CategoryB]
						= epdCatB->scaleByFactor(
							SVLcaLccResultsWidget::conversionFactorEpdReferenceUnit(epdCatB->m_referenceUnit,
															 mat, matLayer.m_para[VICUS::MaterialLayer::P_Thickness].get_value("m"), area));
			if(epdCatC != nullptr)
				itAggregatedComp->second.m_totalEpdData[VICUS::EpdDataset::C_CategoryC]
						= epdCatC->scaleByFactor( renewingFactor *
							SVLcaLccResultsWidget::conversionFactorEpdReferenceUnit(epdCatC->m_referenceUnit,
															 mat, matLayer.m_para[VICUS::MaterialLayer::P_Thickness].get_value("m"), area));
			if(epdCatD != nullptr)
				itAggregatedComp->second.m_totalEpdData[VICUS::EpdDataset::C_CategoryD]
						= epdCatD->scaleByFactor( renewingFactor *
							SVLcaLccResultsWidget::conversionFactorEpdReferenceUnit(epdCatD->m_referenceUnit,
															 mat, matLayer.m_para[VICUS::MaterialLayer::P_Thickness].get_value("m"), area));

		}
	}
}


void SVLcaLccSettingsWidget::resetLcaData() {
	m_compIdToAggregatedData.clear();
	m_typeToAggregatedCompData.clear();
	m_idComponentEpdUndefined.clear();
}


void SVLcaLccSettingsWidget::on_pushButtonImportOkoebaudat_clicked() {
	FUNCID(SVDBEPDEditWidget::on_pushButtonImportOkoebaudat_clicked);

	IBK::Path path(m_ui->filepathOekoBauDat->filename().toStdString());

	if(!path.isValid()) {
		QMessageBox::warning(this, tr("Select ÖKOBAUDAT csv-file"), tr("Please select first a csv-file with valid ÖKOBAUDAT data."));
		return;
	}

	try {
		importOkoebauDat(path);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("%1\nCould not import EPD-Database.").arg(ex.what()), FUNC_ID);
	}
}


void SVLcaLccSettingsWidget::on_comboBoxCalculationMode_currentIndexChanged(int mode) {
	VICUS::LcaSettings lcaSettings = project().m_lcaSettings;
	VICUS::LcaSettings::CalculationMode cm = (VICUS::LcaSettings::CalculationMode)mode;

	lcaSettings.m_calculationMode = cm;
	m_ui->comboBoxCertificationSystem->setEnabled(cm == VICUS::LcaSettings::CM_Detailed);

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", lcaSettings, project().m_lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_comboBoxCertificationSystem_currentIndexChanged(int certiSystem) {
	VICUS::LcaSettings lcaSettings = project().m_lcaSettings;
	lcaSettings.m_certificationSystem = (VICUS::LcaSettings::CertificationSytem)certiSystem;

	switch (lcaSettings.m_certificationSystem) {
		case (VICUS::LcaSettings::CS_BNB) :	lcaSettings.m_certificationModules = VICUS::LcaSettings::CT_BNB;  break;
		case (VICUS::LcaSettings::NUM_CS) : break;
	}

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", lcaSettings, project().m_lccSettings);
	undo->push();
}



void SVLcaLccSettingsWidget::on_pushButtonAreaDetection_clicked() {
	double area = 0;

	for(const VICUS::Building &b : project().m_buildings) {
		for(const VICUS::BuildingLevel &bl : b.m_buildingLevels) {
			for(const VICUS::Room &r : bl.m_rooms) {
				for(const VICUS::Surface &s : r.m_surfaces) {

					VICUS::Component *comp = m_db->m_components[s.m_componentInstance->m_idComponent];
					if(comp == nullptr)
						continue;

					VICUS::Component::ComponentType ct = comp->m_type;
					// qDebug() << "Surface name:" << s.m_displayName;
					if( ct == VICUS::Component::CT_Ceiling || ct == VICUS::Component::CT_FloorToAir || ct == VICUS::Component::CT_FloorToCellar
							|| ct == VICUS::Component::CT_FloorToGround || ct == VICUS::Component::CT_Ceiling ) {
						const IBKMK::Vector3D &n = s.geometry().normal();
						double angle = IBKMK::angleBetweenVectorsDeg(n, IBKMK::Vector3D(0,0,-1) );
						// qDebug() << "Angle between vertical and surface: " << angle;
						if(angle < 5) {
							// qDebug() << "Surface added.";
							area += s.geometry().area();
						}
					}
				}
			}
		}
	}

	VICUS::LcaSettings lcaSettings = project().m_lcaSettings;
	VICUS::KeywordList::setParameter(lcaSettings.m_para, "LcaSettings::para_t", VICUS::LcaSettings::P_NetUsageArea, area);

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", lcaSettings, project().m_lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_pushButtonCalculate_clicked() {

	const VICUS::LcaSettings &lcaSettings = project().m_lcaSettings;
	const VICUS::LccSettings &lccSettings = project().m_lccSettings;

	try {
			// TODO : kWh/a  -> Energy/Time -> Power     --> J/s is SI base unit

			// get annual consumptions in kWh/a

			double coalConsumption			= lccSettings.m_para[VICUS::LccSettings::P_CoalConsumption].get_value("kWh/a");
			double gasConsumption			= lccSettings.m_para[VICUS::LccSettings::P_GasConsumption].get_value("kWh/a");
			double electricityConsumption	= lccSettings.m_para[VICUS::LccSettings::P_ElectricityConsumption].get_value("kWh/a");

	//		double totalEnergyCost =  gasConsumption * m_lccSettings->m_intPara[VICUS::LccSettings::IP_GasPrice].value
	//								+ electricityConsumption * m_lccSettings->m_intPara[VICUS::LccSettings::IP_ElectricityPrice].value
	//								+ coalConsumption * m_lccSettings->m_intPara[VICUS::LccSettings::IP_CoalPrice].value ;
	//		totalEnergyCost /= 100.0; // Mind we store our Prices in cent --> convert to € by /100

			calculateLCA();

			unsigned int numberOfYears = lcaSettings.m_para[VICUS::LcaSettings::P_TimePeriod].get_value("a");
			std::vector<double> investCost(numberOfYears, 0.0);

			m_resultsWidget->setup();
			m_resultsWidget->setLcaResults(m_typeToAggregatedCompData, m_compIdToAggregatedData, VICUS::EpdDataset::C_CategoryA, lcaSettings, investCost);
			m_resultsWidget->setUsageResults(lcaSettings, gasConsumption, electricityConsumption, coalConsumption);
			m_resultsWidget->setLcaResults(m_typeToAggregatedCompData, m_compIdToAggregatedData, VICUS::EpdDataset::C_CategoryC, lcaSettings, investCost);
			m_resultsWidget->setLcaResults(m_typeToAggregatedCompData, m_compIdToAggregatedData, VICUS::EpdDataset::C_CategoryD, lcaSettings, investCost);

			// Mind: cost values are in ct/kWh and we convert to EUR/kWh
			m_resultsWidget->setCostResults(lccSettings, lcaSettings,
					electricityConsumption * lccSettings.m_intPara[VICUS::LccSettings::IP_ElectricityPrice].value / 100.0,
					coalConsumption * lccSettings.m_intPara[VICUS::LccSettings::IP_CoalPrice].value / 100.0,
					gasConsumption * lccSettings.m_intPara[VICUS::LccSettings::IP_GasPrice].value / 100.0,
					investCost);

			m_ui->tabWidget->widget(3)->setEnabled(true);
			m_ui->tabWidget->setCurrentIndex(3);
		}
		catch (IBK::Exception &ex) {
			QMessageBox::critical(this, tr("Error in LCA Calculcation"), tr("Could not calculcate LCA. See Error below.\n%1").arg(ex.what()));
		}
}


void SVLcaLccSettingsWidget::on_lineEditArea_editingFinishedSuccessfully() {
	if (!m_ui->lineEditArea->isValid())
		return;

	VICUS::LcaSettings lcaSettings = project().m_lcaSettings;
	VICUS::KeywordList::setParameter(lcaSettings.m_para, "LcaSettings::para_t", VICUS::LcaSettings::P_NetUsageArea, m_ui->lineEditArea->value());

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", lcaSettings, project().m_lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_lineEditPriceIncreaseGeneral_editingFinishedSuccessfully() {
	if (!m_ui->lineEditPriceIncreaseGeneral->isValid())
		return;

	VICUS::LccSettings lccSettings = project().m_lccSettings;
	VICUS::KeywordList::setParameter(lccSettings.m_para, "LccSettings::para_t", VICUS::LccSettings::P_PriceIncreaseGeneral, m_ui->lineEditPriceIncreaseGeneral->value());

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCC", project().m_lcaSettings, lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_lineEditGasConsumption_editingFinishedSuccessfully() {
	if (!m_ui->lineEditGasConsumption->isValid())
		return;

	VICUS::LccSettings lccSettings = project().m_lccSettings;
	VICUS::KeywordList::setParameter(lccSettings.m_para, "LccSettings::para_t", VICUS::LccSettings::P_GasConsumption, m_ui->lineEditGasConsumption->value());

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCC", project().m_lcaSettings, lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_lineEditElectricityConsumption_editingFinishedSuccessfully() {
	if (!m_ui->lineEditElectricityConsumption->isValid())
		return;

	VICUS::LccSettings lccSettings = project().m_lccSettings;
	VICUS::KeywordList::setParameter(lccSettings.m_para, "LccSettings::para_t", VICUS::LccSettings::P_ElectricityConsumption, m_ui->lineEditElectricityConsumption->value());

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCC", project().m_lcaSettings, lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_lineEditCoalConsumption_editingFinishedSuccessfully() {
	if (!m_ui->lineEditCoalConsumption->isValid())
		return;

	VICUS::LccSettings lccSettings = project().m_lccSettings;
	VICUS::KeywordList::setParameter(lccSettings.m_para, "LccSettings::para_t", VICUS::LccSettings::P_CoalConsumption, m_ui->lineEditCoalConsumption->value());

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCC", project().m_lcaSettings, lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_lineEditPriceIncreaseEnergy_editingFinishedSuccessfully() {
	if (!m_ui->lineEditPriceIncreaseEnergy->isValid())
		return;

	VICUS::LccSettings lccSettings = project().m_lccSettings;
	VICUS::KeywordList::setParameter(lccSettings.m_para, "LccSettings::para_t", VICUS::LccSettings::P_PriceIncreaseEnergy, m_ui->lineEditPriceIncreaseEnergy->value());

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCC", project().m_lcaSettings, lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_toolButtonSelectGas_clicked() {
	VICUS::LcaSettings lcaSettings = project().m_lcaSettings;

	// get EPD edit dialog from mainwindow
	SVDatabaseEditDialog * editDialog = SVMainWindow::instance().dbEpdEditDialog();

	unsigned int id = lcaSettings.m_idUsage[VICUS::LcaSettings::UT_Gas];

	unsigned int idGas = editDialog->select(id, false, "MJ", 5);
	if (idGas != VICUS::INVALID_ID && idGas != id) {
		lcaSettings.m_idUsage[VICUS::LcaSettings::UT_Gas] = idGas;
	}

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", lcaSettings, project().m_lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_toolButtonSelectElectricity_clicked() {
	VICUS::LcaSettings lcaSettings = project().m_lcaSettings;

	// get EPD edit dialog from mainwindow
	SVDatabaseEditDialog * editDialog = SVMainWindow::instance().dbEpdEditDialog();

	unsigned int id = lcaSettings.m_idUsage[VICUS::LcaSettings::UT_Electricity];

	unsigned int idElectricity = editDialog->select(id, false, "MJ", 5);
	if (idElectricity != VICUS::INVALID_ID && idElectricity != id) {
		lcaSettings.m_idUsage[VICUS::LcaSettings::UT_Electricity] = idElectricity;
	}

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", lcaSettings, project().m_lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_toolButtonSelectCoal_clicked() {
	VICUS::LcaSettings lcaSettings = project().m_lcaSettings;
	// get EPD edit dialog from mainwindow
	SVDatabaseEditDialog * editDialog = SVMainWindow::instance().dbEpdEditDialog();

	unsigned int id = lcaSettings.m_idUsage[VICUS::LcaSettings::UT_Coal];

	unsigned int idCoal = editDialog->select(id, false, "MJ", 5);
	if (idCoal != VICUS::INVALID_ID && idCoal != id) {
		lcaSettings.m_idUsage[VICUS::LcaSettings::UT_Coal] = idCoal;
	}

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", lcaSettings, project().m_lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_lineEditCoalPrice_editingFinishedSuccessfully() {
	if (!m_ui->lineEditCoalPrice->isValid())
		return;

	VICUS::LccSettings lccSettings = project().m_lccSettings;

	int newVal = (int)(m_ui->lineEditCoalPrice->value()*100);
	int oldVal = lccSettings.m_intPara[VICUS::LccSettings::IP_CoalPrice].value;

	if (newVal == oldVal)
		return;

	VICUS::KeywordList::setIntPara(lccSettings.m_intPara, "LccSettings::intPara_t", VICUS::LccSettings::IP_CoalPrice, newVal);
	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", project().m_lcaSettings, lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_lineEditGasPrice_editingFinishedSuccessfully() {
	if (!m_ui->lineEditGasPrice->isValid())
		return;

	VICUS::LccSettings lccSettings = project().m_lccSettings;

	int newVal = (int)(m_ui->lineEditGasPrice->value()*100);
	int oldVal = lccSettings.m_intPara[VICUS::LccSettings::IP_GasPrice].value;

	if (newVal == oldVal)
		return;

	VICUS::KeywordList::setIntPara(lccSettings.m_intPara, "LccSettings::intPara_t", VICUS::LccSettings::IP_GasPrice, newVal);

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", project().m_lcaSettings, lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_lineEditElectricityPrice_editingFinishedSuccessfully() {
	if (m_ui->lineEditElectricityPrice->isValid())
		return;

	VICUS::LccSettings lccSettings = project().m_lccSettings;

	int newVal = (int)(m_ui->lineEditElectricityPrice->value()*100);
	int oldVal = lccSettings.m_intPara[VICUS::LccSettings::IP_ElectricityPrice].value;

	if (newVal == oldVal)
		return;

	VICUS::KeywordList::setIntPara(lccSettings.m_intPara, "LccSettings::intPara_t", VICUS::LccSettings::IP_ElectricityPrice, newVal);

	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", project().m_lcaSettings, lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_spinBoxTimePeriod_valueChanged(int years) {
	VICUS::LcaSettings lcaSettings = project().m_lcaSettings;
	VICUS::KeywordList::setParameter(lcaSettings.m_para, "LcaSettings::para_t", VICUS::LcaSettings::P_TimePeriod, years);
	SVUndoModifyLcaLcc *undo = new SVUndoModifyLcaLcc("Modified LCA", lcaSettings, project().m_lccSettings);
	undo->push();
}


void SVLcaLccSettingsWidget::on_tabWidget_currentChanged(int index) {
}

