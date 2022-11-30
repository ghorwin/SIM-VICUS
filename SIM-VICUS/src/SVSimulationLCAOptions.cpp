#include "SVSimulationLCAOptions.h"
#include "ui_SVSimulationLCAOptions.h"

#include <IBK_Parameter.h>
#include <IBK_FileReader.h>
#include <IBK_StopWatch.h>

#include <SVSettings.h>
#include <SVDatabase.h>
#include <SVDatabaseEditDialog.h>
#include <SVDBEpdTableModel.h>
#include <SVProjectHandler.h>

#include <VICUS_Project.h>

#include <QProgressDialog>

SVSimulationLCAOptions::SVSimulationLCAOptions(QWidget *parent, VICUS::LCASettings & settings) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationLCAOptions),
	m_lcaSettings(&settings)
{
	m_ui->setupUi(this);
	layout()->setMargin(0);

//	m_lcaSettings->initDefaults();

	m_ui->lineEditTimePeriod->setText(QString("%1").arg(m_lcaSettings->m_para[VICUS::LCASettings::P_TimePeriod].get_value("a")));
	m_ui->lineEditPriceIncrease->setText(QString("%1").arg(m_lcaSettings->m_para[VICUS::LCASettings::P_PriceIncrease].get_value("%")));

	m_ui->filepathOekoBauDat->setup("", true, true, tr("ÖKOBAUDAT container files (*.csv);;All files (*.*)"),
									SVSettings::instance().m_dontUseNativeDialogs);

	m_prj = SVProjectHandler::instance().project();
}

SVSimulationLCAOptions::~SVSimulationLCAOptions() {
	delete m_ui;
}

void SVSimulationLCAOptions::calculateLCA() {
#if 0
	FUNCID(LCA::calculateLCA);

	/*! Summarize all components with all constructions and material layers.
		Categorize all construction with their surface areas.
	*/

	/* Annahmen: diese Strukturen müssen umbenannt werden. */
	std::map<unsigned int, VICUS::Component>				m_dbComponents;
	std::map<unsigned int, VICUS::Construction>				m_dbConstructions;
	std::map<unsigned int, VICUS::Material>					m_dbOpaqueMaterials;
	std::map<unsigned int, VICUS::EPDDataset>				m_dbEPDs;


	struct MatEpd{
		VICUS::EPDDataset m_epdA;
		VICUS::EPDDataset m_epdB;
		VICUS::EPDDataset m_epdC;
		VICUS::EPDDataset m_epdD;
	};

	std::map<unsigned int, LCAComponentResult>		compRes;
	std::map<unsigned int, LCAComponentResult>		compResErsatz;

	//holds the data for each material
	std::map<unsigned int, MatEpd>					materialIdAndEpd;
	double netFloorArea = m_building.m_netFloorArea;

	/* Calculate all surface areas according to all components. */
	for (auto &bl : m_building.m_buildingLevels) {
		for (auto &r : bl.m_rooms) {
			for (auto &s : r.m_surfaces) {
				const VICUS::Surface &surf = s;

				// get component
				const VICUS::ComponentInstance * compInstance = s.m_componentInstance;
				if (compInstance != nullptr) {
					VICUS::Component comp = elementExists<VICUS::Component>(m_dbComponents, compInstance->m_idComponent,
													s.m_displayName.toStdString(),"Component", "surface");
					//save surface area
					compRes[comp.m_id].m_area += surf.geometry().area();
				}
				else {
					/// TODO : error handling if component instance pointer is empty (no component associated)
				}
			}
		}
	}

	//calculate all lca for each component
	for (auto &c : compRes) {
		const VICUS::Component &comp = m_dbComponents[c.first];

		//opaque construction
		if(comp.m_idConstruction != VICUS::INVALID_ID){
			//get construction
			///TODO Dirk baufähig gemacht müsste rückgängig gemacht werden
			VICUS::Construction constr;
//					elementExists<VICUS::Construction>(m_dbConstructions, comp.m_idOpaqueConstruction,
//													   comp.m_displayName.toStdString(),"Construction",
//													   "component");

			//calculate each construction
			for(auto l : constr.m_materialLayers){
				//check if material exists
				VICUS::Material mat =
						elementExists<VICUS::Material>(m_dbOpaqueMaterials, l.m_idMaterial,
														   constr.m_displayName.string(),
														   "Material",
														   "construction");

				//material exists already in the new user database
				if(materialIdAndEpd.find(mat.m_id) != materialIdAndEpd.end())
					continue;

				MatEpd &matEpd = materialIdAndEpd[mat.m_id];
				//check each material epd id
				for (auto idEpd : mat.m_idEpds) {
					if(idEpd == VICUS::INVALID_ID)
						continue;

					VICUS::EPDDataset epd = elementExists<VICUS::EPDDataset>(m_dbEPDs, idEpd,
																   mat.m_displayName.string(),
																   "EPD",
																   "material");

					//if we found the right dataset add values A1- A2
					if(epd.m_module == VICUS::EPDDataset::M_A1 ||
					   epd.m_module == VICUS::EPDDataset::M_A2 ||
					   epd.m_module == VICUS::EPDDataset::M_A1_A2||
					   epd.m_module == VICUS::EPDDataset::M_A3 ||
					   epd.m_module == VICUS::EPDDataset::M_A1_A3){
						//add all values in a category A
						for (unsigned int i=0;i< VICUS::EPDDataset::NUM_P; ++i) {
							IBK::Parameter para = epd.m_para[i];
							//...
							if(para.value != 0){
								matEpd.m_epdA.m_para[i].set(para.name,
															matEpd.m_epdA.m_para[i].get_value(para.unit())
															+ para.get_value(para.unit()),
															para.unit());
							}
						}
					}
					else if (epd.m_module == VICUS::EPDDataset::M_B6) {
						//add all values in a category B
						for (unsigned int i=0;i< VICUS::EPDDataset::NUM_P; ++i) {
							IBK::Parameter para = epd.m_para[i];
							//...
							if(para.value != 0){
								matEpd.m_epdB.m_para[i].set(para.name,
															matEpd.m_epdB.m_para[i].get_value(para.unit())
															+ para.get_value(para.unit()),
															para.unit());
							}
						}
					}
					else if (epd.m_module == VICUS::EPDDataset::M_C2 ||
							 epd.m_module == VICUS::EPDDataset::M_C2_C4 ||
							 epd.m_module == VICUS::EPDDataset::M_C3 ||
							 epd.m_module == VICUS::EPDDataset::M_C2_C3 ||
							 epd.m_module == VICUS::EPDDataset::M_C3_C4 ||
							 epd.m_module == VICUS::EPDDataset::M_C4) {
						//add all values in a category C
						for (unsigned int i=0;i< VICUS::EPDDataset::NUM_P; ++i) {
							IBK::Parameter para = epd.m_para[i];
							//...
							if(para.value != 0){
								matEpd.m_epdC.m_para[i].set(para.name,
															matEpd.m_epdC.m_para[i].get_value(para.unit())
															+ para.get_value(para.unit()),
															para.unit());
							}
						}
					}
					else if (epd.m_module == VICUS::EPDDataset::M_D) {
						//add all values in a category D
						for (unsigned int i=0;i< VICUS::EPDDataset::NUM_P; ++i) {
							IBK::Parameter para = epd.m_para[i];
							//...
							if(para.value != 0){
								matEpd.m_epdD.m_para[i].set(para.name,
															matEpd.m_epdD.m_para[i].get_value(para.unit())
															+ para.get_value(para.unit()),
															para.unit());
							}
						}
					}
				}
			}
		}
	}



	for (auto &e : compRes) {
		//Component result object
		LCAComponentResult &comp = e.second;
		unsigned int compId = e.first;

		//check if opaque construction is available
		if(m_dbComponents[compId].m_idConstruction != VICUS::INVALID_ID){
			const VICUS::Construction &constr = m_dbConstructions[m_dbComponents[compId].m_idConstruction];

			//get values for all material layers in each category of the lifecycle
			for(auto &l : constr.m_materialLayers){
				MatEpd &matEpd = materialIdAndEpd[l.m_idMaterial];
				double rho = m_dbOpaqueMaterials[l.m_idMaterial].m_para[VICUS::Material::P_Density].get_value("kg/m3");

//				addEpdMaterialToComponent(matEpd.m_epdA, comp, compResErsatz[compId],
//							   l.m_lifeCylce, l.m_thickness.get_value("m"),
//							   rho, 0, m_adjustment);

				addEpdMaterialToComponent(matEpd.m_epdB, comp, compResErsatz[compId],
							   0, l.m_thickness.get_value("m"),
							   rho, 1, m_adjustment);

//				addEpdMaterialToComponent(matEpd.m_epdC, comp, compResErsatz[compId],
//							   l.m_lifeCylce, l.m_thickness.get_value("m"),
//							   rho, 2, m_adjustment);

//				addEpdMaterialToComponent(matEpd.m_epdD, comp, compResErsatz[compId],
//							   l.m_lifeCylce, l.m_thickness.get_value("m"),
//							   rho, 3, m_adjustment);

			}

		}

	}
#endif
}


bool convertString2Val(double &val, const std::string &text, unsigned int row, unsigned int column) {
	try {
		val = IBK::string2val<double>(text);
	}  catch (IBK::Exception &ex) {
		IBK::IBK_Message(IBK::FormatString("%4\nCould not convert string '%1' of row %2 and column %3")
						 .arg(text).arg(row+1).arg(column+1).arg(ex.what()));
		return false;
	}
	return true;
}

void SVSimulationLCAOptions::importOkoebauDat(const IBK::Path & csvPath) {
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

	std::vector<VICUS::EpdDataset> dataSets;

	IBK::StopWatch timer;
	timer.start();

	for (unsigned int row = 0; row<dataLines.size(); ++row) {
		std::string &line = dataLines[row];
//		IBK::trim(line, ",");
//		IBK::trim(line, "\"");
//		IBK::trim(line, "MULTILINESTRING ((");
//		IBK::trim(line, "))");
		IBK::explode(line, tokens, ";", IBK::EF_KeepEmptyTokens);
		dataSets.push_back(VICUS::EpdDataset());
		VICUS::EpdDataset &epd = dataSets.back();

		// convert this vector to double and add it as a graph
		std::vector<std::vector<double> > polyLine;
		for (unsigned int col = 0; col < tokens.size(); ++col){
			std::string t = IBK::ANSIToUTF8String(tokens[col]);

			if(timer.difference() > 200) {
				dlg->setValue(row);
				qApp->processEvents();
			}

			if(dlg->wasCanceled()) {
				IBK::IBK_Message(IBK::FormatString("EPD-Import has been interrupted by user."));
				return;
			}

			// qDebug() << "Row: " << row << " Column: " << col << " Text: " << QString::fromStdString(t);

			if (t == "" || t == "not available")
				continue;

			switch (col) {
				// Not imported coloumns
				case ColVersion:
				case ColConformity:
				case ColCountryCode:
				case ColReferenceYear:
				case ColPublishedOn:
				case ColRegistrationNumber:
				case ColRegistrationBody:
				case ColUUIDOfThePredecessor:
				break;

				case ColUUID:				epd.m_uuid = QString::fromStdString(t);			break;
				case ColNameDe:				epd.m_displayName.setString(t, "De");			break;
				case ColNameEn:				epd.m_displayName.setString(t, "En");			break;
				case ColCategoryDe:			epd.m_category.setString(t, "De");				break;
				case ColCategoryEn:			epd.m_category.setString(t, "En");				break;
				case ColType: {
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


					epd.m_type = type;

				} break;
				case ColExpireYear:			epd.m_expireYear = QString::fromStdString(t);			break;
				case ColDeclarationOwner:	epd.m_manufacturer = QString::fromStdString(t);			break;
				case ColReferenceSize: {
					double val;
					if(!convertString2Val(val, t, row, col))
						continue;

					epd.m_referenceQuantity = val;
				} break;
				case ColReferenceUnit:		epd.m_referenceUnit = QString::fromStdString(t);		break;
				case ColURL:				epd.m_dataSource = QString::fromStdString(t);			break;

				case ColWeightPerUnitArea: {
					double val;
					if(!convertString2Val(val, t, row, col))
						continue;

					epd.m_para[VICUS::EpdDataset::P_AreaDensity].set("AreaDensity", val, IBK::Unit("kg/m2"));
				} break;
				case ColBulkDensity: {
					double val;
					if(!convertString2Val(val, t, row, col))
						continue;
					epd.m_para[VICUS::EpdDataset::P_DryDensity].set("DryDensity", val, IBK::Unit("kg/m3"));
				} break;
				case ColGWP: {
					double val;
					if(!convertString2Val(val, t, row, col))
						continue;

					epd.m_para[VICUS::EpdDataset::P_GWP].set("GWP", val, IBK::Unit("kg"));
				} break;
				case ColODP: {
					double val;
					if(!convertString2Val(val, t, row, col))
						continue;
					epd.m_para[VICUS::EpdDataset::P_ODP].set("ODP", val, IBK::Unit("kg"));
				} break;
				case ColPOCP: {
					double val;
					if(!convertString2Val(val, t, row, col))
						continue;
					epd.m_para[VICUS::EpdDataset::P_POCP].set("POCP", val, IBK::Unit("kg"));

				} break;
				case ColAP: {
					double val;
					if(!convertString2Val(val, t, row, col))
						continue;
					epd.m_para[VICUS::EpdDataset::P_AP].set("AP", val, IBK::Unit("kg"));
				} break;
				case ColEP: {
					double val;
					if(!convertString2Val(val, t, row, col))
						continue;
					epd.m_para[VICUS::EpdDataset::P_EP].set("EP", val, IBK::Unit("kg"));
				} break;
				case ColPENRT: {
					double val;
					if(!convertString2Val(val, t, row, col))
						continue;
					epd.m_para[VICUS::EpdDataset::P_PENRT].set("PENRT", val, IBK::Unit("W/mK"));
				} break;
				case ColPERT: {
					double val;
					if(!convertString2Val(val, t, row, col))
						continue;
					epd.m_para[VICUS::EpdDataset::P_PERT].set("PERT", val, IBK::Unit("W/mK"));
				} break;

				case ColModule: {
					VICUS::EpdDataset::Module module = VICUS::EpdDataset::NUM_M;

					if (t == "A1")
						module = VICUS::EpdDataset::M_A1;
					else if (t == "A2")
						module = VICUS::EpdDataset::M_A2;
					else if (t == "A3")
						module = VICUS::EpdDataset::M_A3;
					else if (t == "A1-A2")
						module = VICUS::EpdDataset::M_A1_A2;
					else if (t == "A1-A3")
						module = VICUS::EpdDataset::M_A1_A3;
					else if (t == "A4")
						module = VICUS::EpdDataset::M_A4;
					else if (t == "A5")
						module = VICUS::EpdDataset::M_A5;
					else if (t == "B1")
						module = VICUS::EpdDataset::M_B1;
					else if (t == "B2")
						module = VICUS::EpdDataset::M_B2;
					else if (t == "B3")
						module = VICUS::EpdDataset::M_B3;
					else if (t == "B4")
						module = VICUS::EpdDataset::M_B4;
					else if (t == "B5")
						module = VICUS::EpdDataset::M_B5;
					else if (t == "B6")
						module = VICUS::EpdDataset::M_B6;
					else if (t == "B7")
						module = VICUS::EpdDataset::M_B7;
					else if (t == "C1")
						module = VICUS::EpdDataset::M_C1;
					else if (t == "C2")
						module = VICUS::EpdDataset::M_C2;
					else if (t == "C2-C3")
						module = VICUS::EpdDataset::M_C2_C3;
					else if (t == "C2-C4")
						module = VICUS::EpdDataset::M_C2_C4;
					else if (t == "C3")
						module = VICUS::EpdDataset::M_C3;
					else if (t == "C3-C4")
						module = VICUS::EpdDataset::M_C3_C4;
					else if (t == "C4")
						module = VICUS::EpdDataset::M_C4;
					else if (t == "D")
						module = VICUS::EpdDataset::M_D;


					epd.m_module = module;

				} break;
			}
		}

	}

	dlg->setValue(dataLines.size());
	// Get pointer to our EPD edit widget
	SVDatabaseEditDialog *conEditDialog = SVMainWindow::instance().dbEpdEditDialog();
	Q_ASSERT(conEditDialog != nullptr);

	// now we import all Datasets
	dynamic_cast<SVDBEpdTableModel*>(conEditDialog->dbModel())->importDatasets(dataSets);
}


void SVSimulationLCAOptions::addComponentInstance(const VICUS::ComponentInstance & compInstance) {
	if(m_compIdToAggregatedData.find(compInstance.m_id) != m_compIdToAggregatedData.end())
		m_compIdToAggregatedData[compInstance.m_idComponent] = AggregatedComponentData(compInstance);
	else
		m_compIdToAggregatedData[compInstance.m_idComponent].addArea(compInstance);
}

void SVSimulationLCAOptions::analyseProjectComponents() {
	QStringList lifetime, cost, epd;

	for(const VICUS::ComponentInstance &ci : m_prj.m_componentInstances) {
		// Add CI to aggregated data map
		addComponentInstance(ci);

		const VICUS::Component &comp = *SVSettings::instance().m_db.m_components[ci.m_idComponent];
		const VICUS::Construction &con = *SVSettings::instance().m_db.m_constructions[comp.m_idConstruction];

		for(const VICUS::MaterialLayer &matLayer : con.m_materialLayers) {
			// Check Cost & lifetime of used Materials
			bool isLifetimeDefined = !matLayer.m_lifetime.empty();
			bool isCostDefined = !matLayer.m_cost.empty();
			// Check EPDs of used Materials
			const VICUS::Material &mat = *SVSettings::instance().m_db.m_materials[matLayer.m_idMaterial];

			bool isEPDDefined = !mat.m_epdCategorySet.isEmpty();

			if(!isLifetimeDefined)
				lifetime << QString::fromStdString(mat.m_displayName.string());
			if(!isCostDefined)
				cost << QString::fromStdString(mat.m_displayName.string());
			if(!isEPDDefined)
				epd << QString::fromStdString(mat.m_displayName.string());
		}
	}

	QString messageText(tr("Lifetime:\t%1\nCost:\t%2\nEPD:\t%3\n\nProceed and skip all components without needed Data?")
						 .arg(lifetime.join("\n\t\t")).arg(cost.join("\n\t\t")).arg(epd.join("\n\t\t")));
	if(QMessageBox::warning(this, "LCA/LCC Information is missing", messageText) == QMessageBox::Cancel)
		return;
}


void SVSimulationLCAOptions::on_pushButtonImportOkoebaudat_clicked() {
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


void SVSimulationLCAOptions::on_pushButtonLcaLcc_clicked() {
	analyseProjectComponents();
}

