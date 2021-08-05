#include "SVDBDuplicatesDialog.h"
#include "ui_SVDBDuplicatesDialog.h"

#include <QtExt_Conversions.h>
#include <QHeaderView>

#include <IBK_StringUtils.h>
#include <IBK_Differ.h>

#include <tinyxml.h>

#include "SVSettings.h"
#include "SVStyle.h"

SVDBDuplicatesDialog::SVDBDuplicatesDialog(QWidget *parent) :
	QDialog(parent
	#ifdef Q_OS_LINUX
				, Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint /*| Qt::WindowSystemMenuHint*/
	#endif
			),
	m_ui(new Ui::SVDBDuplicatesDialog)
{
	m_ui->setupUi(this);

	m_ui->tableWidget->setColumnCount(3);
	m_ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Database Element")
		<< tr("Duplicate #1") << tr("Duplicate #2"));

	m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

	SVStyle::formatDatabaseTableView(m_ui->tableWidget);
	m_ui->tableWidget->setSortingEnabled(false);

	connect(m_ui->tableWidget->selectionModel(), &QItemSelectionModel::currentRowChanged,
			this, &SVDBDuplicatesDialog::onCurrentRowChanged);
}


SVDBDuplicatesDialog::~SVDBDuplicatesDialog() {
	delete m_ui;
}


bool SVDBDuplicatesDialog::removeDuplicates(SVDatabase::DatabaseTypes dbType) {
	m_dbType = dbType;
	updateUi();
	m_dbModified = false;
	exec();

	return m_dbModified; // if true, the undo-history of the project will be cleared, because Undo after DB change is not possible
}


template <typename T>
QString dumpXML(const T & data) {
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "VicusData" );
	doc.LinkEndChild(root);
	data.writeXML(root);

	TiXmlPrinter printer;
	printer.SetIndent( "  " );

	doc.Accept( &printer );
	std::string xmltext = printer.CStr();

	return QString::fromStdString(xmltext);
}


std::vector<std::string> processXML(const std::string & xmlText) {
	// split and create vector of lines
	std::vector<std::string> lines = IBK::explode(xmlText, '\n');
	if (lines.size() < 3)
		return lines;
	// remove first 2 lines and last
	lines.erase(lines.begin(), lines.begin()+2);
	lines.erase(lines.end()-1, lines.end());

	// remove two levels of indentation in each string
	for (std::string & l : lines)
		l = l.substr(2);
	return lines;
}


template <typename T>
void dbItem(const VICUS::Database<T> & db, unsigned int idLeft, unsigned int idRight,
			const VICUS::AbstractDBElement * &left, const VICUS::AbstractDBElement * &right)
{
	left = db[idLeft];
	right = db[idRight];
}


void SVDBDuplicatesDialog::onCurrentRowChanged(const QModelIndex & current, const QModelIndex & /*previous*/) {
	Q_ASSERT(current.isValid());
	m_ui->groupBox->setVisible(true);
	// take currently selected items, access them and generate their diffs
	int currentRow = current.row();
	SVDatabase::DatabaseTypes type = (SVDatabase::DatabaseTypes)m_ui->tableWidget->item(currentRow, 0)->data(Qt::UserRole).toInt();
	unsigned int leftID = m_ui->tableWidget->item(currentRow, 1)->data(Qt::UserRole).toUInt();
	unsigned int rightID = m_ui->tableWidget->item(currentRow, 2)->data(Qt::UserRole).toUInt();
	QString xmlLeft, xmlRight;
	const SVDatabase & db = SVSettings::instance().m_db;
	const VICUS::AbstractDBElement * dbElemLeft = nullptr;
	const VICUS::AbstractDBElement * dbElemRight = nullptr;
	switch (type) {
		case SVDatabase::DT_Materials:		dbItem(db.m_materials, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_Constructions:	dbItem(db.m_constructions, leftID, rightID, dbElemLeft, dbElemRight); break;

		// TODO Katja
		case SVDatabase::DT_Windows:
			xmlLeft = dumpXML(*db.m_windows[leftID]);
			xmlRight = dumpXML(*db.m_windows[rightID]);
		break;
		case SVDatabase::DT_WindowGlazingSystems:
			xmlLeft = dumpXML(*db.m_windowGlazingSystems[leftID]);
			xmlRight = dumpXML(*db.m_windowGlazingSystems[rightID]);
		break;
		case SVDatabase::DT_BoundaryConditions:
			xmlLeft = dumpXML(*db.m_boundaryConditions[leftID]);
			xmlRight = dumpXML(*db.m_boundaryConditions[rightID]);
		break;
		case SVDatabase::DT_Components: dbItem(db.m_components, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_SubSurfaceComponents:
			xmlLeft = dumpXML(*db.m_subSurfaceComponents[leftID]);
			xmlRight = dumpXML(*db.m_subSurfaceComponents[rightID]);
		break;
		case SVDatabase::DT_SurfaceHeating:
			xmlLeft = dumpXML(*db.m_surfaceHeatings[leftID]);
			xmlRight = dumpXML(*db.m_surfaceHeatings[rightID]);
		break;
		case SVDatabase::DT_Pipes:
			xmlLeft = dumpXML(*db.m_pipes[leftID]);
			xmlRight = dumpXML(*db.m_pipes[rightID]);
		break;
		case SVDatabase::DT_Fluids:
			xmlLeft = dumpXML(*db.m_fluids[leftID]);
			xmlRight = dumpXML(*db.m_fluids[rightID]);
		break;
		case SVDatabase::DT_NetworkComponents:
			xmlLeft = dumpXML(*db.m_networkComponents[leftID]);
			xmlRight = dumpXML(*db.m_networkComponents[rightID]);
		break;
		case SVDatabase::DT_NetworkControllers:
			xmlLeft = dumpXML(*db.m_networkControllers[leftID]);
			xmlRight = dumpXML(*db.m_networkControllers[rightID]);
		break;
		case SVDatabase::DT_SubNetworks:
			xmlLeft = dumpXML(*db.m_subNetworks[leftID]);
			xmlRight = dumpXML(*db.m_subNetworks[rightID]);
		break;
		case SVDatabase::DT_Schedules:
			xmlLeft = dumpXML(*db.m_schedules[leftID]);
			xmlRight = dumpXML(*db.m_schedules[rightID]);
		break;
		case SVDatabase::DT_InternalLoads:
			xmlLeft = dumpXML(*db.m_internalLoads[leftID]);
			xmlRight = dumpXML(*db.m_internalLoads[rightID]);
		break;
		case SVDatabase::DT_ZoneControlThermostat:
			xmlLeft = dumpXML(*db.m_zoneControlThermostat[leftID]);
			xmlRight = dumpXML(*db.m_zoneControlThermostat[rightID]);
		break;
		case SVDatabase::DT_ZoneControlShading:
			xmlLeft = dumpXML(*db.m_zoneControlShading[leftID]);
			xmlRight = dumpXML(*db.m_zoneControlShading[rightID]);
		break;
		case SVDatabase::DT_ZoneControlNaturalVentilation:
			xmlLeft = dumpXML(*db.m_zoneControlVentilationNatural[leftID]);
			xmlRight = dumpXML(*db.m_zoneControlVentilationNatural[rightID]);
		break;
		case SVDatabase::DT_ZoneIdealHeatingCooling:
			xmlLeft = dumpXML(*db.m_zoneIdealHeatingCooling[leftID]);
			xmlRight = dumpXML(*db.m_zoneIdealHeatingCooling[rightID]);
		break;
		case SVDatabase::DT_VentilationNatural:
			xmlLeft = dumpXML(*db.m_ventilationNatural[leftID]);
			xmlRight = dumpXML(*db.m_ventilationNatural[rightID]);
		break;
		case SVDatabase::DT_Infiltration:
			xmlLeft = dumpXML(*db.m_infiltration[leftID]);
			xmlRight = dumpXML(*db.m_infiltration[rightID]);
		break;
		case SVDatabase::DT_ZoneTemplates:
			xmlLeft = dumpXML(*db.m_zoneTemplates[leftID]);
			xmlRight = dumpXML(*db.m_zoneTemplates[rightID]);
		break;
		case SVDatabase::NUM_DT:
		break;
	}

	// get XML description of tag and enabled/disable "take" buttons
	if (dbElemLeft != nullptr) {
		xmlLeft = dumpXML(*dbElemLeft);
		m_ui->pushButtonTakeRight->setEnabled(!dbElemLeft->m_builtIn); // if left is a built-in, we disable the "take right" button
	}
	if (dbElemRight != nullptr) {
		xmlRight = dumpXML(*dbElemRight);
		m_ui->pushButtonTakeLeft->setEnabled(!dbElemRight->m_builtIn); // if right is a built-in, we disable the "take left" button
	}

	// generate diff and color output

	// split and create vector of lines
	std::vector<std::string> linesLeft = processXML(xmlLeft.toStdString());
	std::vector<std::string> linesRight = processXML(xmlRight.toStdString());

	IBK::Differ<std::string> diff(linesLeft, linesRight);
	diff.diff();

	std::string encodedLeft;
	std::string encodedRight;

	for (unsigned int i=0, count = diff.resultObj().size(); i<count; ++i) {
		// check if this and the next line have opposite change markers
		// and if so, compute levenstein
		if (i+1 < count) {
			// need both insert (0) and remove (1) operations, doesn't matter if insert comes first
			if (diff.resultOperation()[i] + diff.resultOperation()[i+1] == 1) {
				std::string leftStr = diff.resultObj()[i];
				std::string rightStr = diff.resultObj()[i+1];
				// if both strings start with same XML token we treat them as the same
				std::string::size_type pos = leftStr.find_first_not_of(" \t");
				pos = leftStr.find_first_of(" \t", pos);
				if (pos == std::string::npos)
					pos = leftStr.size();
				if (rightStr.size() >= pos && leftStr.substr(0,pos) == rightStr.substr(0, pos)) {
//				int ld = IBK::levenshtein_distance(diff.resultObj()[i], diff.resultObj()[i+1]);
//				if (ld < (diff.resultObj()[i].length()*0.5) && diff.resultObj()[i].length() > 0) {
#if 1
					encodedLeft += "<span style=\"color:#2020a0;background-color:#c0c0ff\">" + IBK::convertXml2Html(diff.resultObj()[i]) + "</span><br>";
					encodedRight += "<span style=\"color:#2020a0;background-color:#c0c0ff\">" + IBK::convertXml2Html(diff.resultObj()[i+1]) + "</span><br>";
#else

					// found a conflicting line (i.e. same line edited in both files, which should be the most common case)
					// now determine the different chars in this line
					std::vector<char> lineLeft(diff.resultObj()[i].begin(), diff.resultObj()[i].end());
					std::vector<char> lineRight(diff.resultObj()[i+1].begin(), diff.resultObj()[i+1].end());
					IBK::Differ<char> lineDiff(lineLeft, lineRight);
					lineDiff.diff();
					bool equalMode = true;
					std::string mergedLineLeft;
					std::string mergedLineRight;
					for (unsigned int j=0, chcount = lineDiff.resultObj().size(); j<chcount; ++j) {
						if (lineDiff.resultOperation()[j] == IBK::DifferOpEqual) {
							// switch to equal mode?
							if (!equalMode) {
								// if we had already a "different string" span, close it and start a new
								if (!mergedLineLeft.empty()) {
									mergedLineLeft += "</span>";
									mergedLineRight += "</span>";
								}
								mergedLineLeft += "<span style=\"color:#a0a0ff;background-color:#ffffff\">";
								mergedLineRight += "<span style=\"color:#a0a0ff;background-color:#ffffff\">";
								equalMode = true;
							}
							mergedLineLeft += IBK::convertXml2Html(std::string(1,lineDiff.resultObj()[j]));
							mergedLineRight += IBK::convertXml2Html(std::string(1,lineDiff.resultObj()[j]));
						}
						else {
							// switch to different mode?
							if (equalMode) {
								// if we had already an "equal string" span, close it and start a new
								if (!mergedLineLeft.empty()) {
									mergedLineLeft += "</span>";
									mergedLineRight += "</span>";
								}
								mergedLineLeft += "<span style=\"color:#2020a0;background-color:#c0c0ff\">";
								mergedLineRight += "<span style=\"color:#2020a0;background-color:#c0c0ff\">";
								equalMode = false;
							}
							++j; // skip over next char as well
						}
						// for first char, add span header
						if (mergedLineLeft.empty()) {
							mergedLineLeft += "<span style=\"color:#a0a0ff;background-color:#ffffff\">";
							mergedLineRight += "<span style=\"color:#a0a0ff;background-color:#ffffff\">";
						}
						mergedLineLeft += IBK::convertXml2Html(std::string(1,lineDiff.resultObj()[j]));
						mergedLineRight += IBK::convertXml2Html(std::string(1,lineDiff.resultObj()[j+1]));
					}
					if (!mergedLine.empty())
						mergedLine += "</span>";

					encodedLeft += mergedLine + "<br>";
					encodedRight += mergedLine + "<br>";
					++i;
					continue;
#endif
					++i; // skip over second marker as well
					continue;
				}
			}
		}
		switch (diff.resultOperation()[i]) {
			case IBK::DifferOpEqual :
				encodedLeft += "<span style=\"font-size:9pt;color:#606060\">" + IBK::convertXml2Html(diff.resultObj()[i]) + "</span><br>";
				encodedRight += "<span style=\"font-size:9pt;color:#606060\">" + IBK::convertXml2Html(diff.resultObj()[i]) + "</span><br>";
			break;
			case IBK::DifferOpInsert :
				encodedLeft += "<span style=\"font-size:9pt;color:#20c050\">" + IBK::convertXml2Html(diff.resultObj()[i]) + "</span><br>";
				encodedRight += "<br>";
			break;
			case IBK::DifferOpRemove :
				encodedLeft += "<br>";
				encodedRight += "<span style=\"font-size:9pt;color:#c04040\">" + IBK::convertXml2Html(diff.resultObj()[i]) + "</span><br>";
			break;
		}
	}

	const char * const htmlPrefix = "<html><body><pre style=\"font-size:9pt;\">";
	const char * const htmlSuffix = "</pre></body></html>";

	QString formattedHtmlLeft = htmlPrefix + QString::fromStdString(encodedLeft) + htmlSuffix;
	QString formattedHtmlRight = htmlPrefix + QString::fromStdString(encodedRight) + htmlSuffix;

	m_ui->textEditLeft->setHtml(formattedHtmlLeft);
	m_ui->textEditRight->setHtml(formattedHtmlRight);
}


void SVDBDuplicatesDialog::on_pushButtonTakeLeft_clicked() {
	// get left db element ID
	int currentRow = m_ui->tableWidget->currentRow();
	Q_ASSERT(currentRow != -1);

	SVDatabase::DatabaseTypes type = (SVDatabase::DatabaseTypes)m_ui->tableWidget->item(currentRow, 0)->data(Qt::UserRole).toInt();
	unsigned int leftID = m_ui->tableWidget->item(currentRow, 1)->data(Qt::UserRole).toUInt();
	unsigned int rightID = m_ui->tableWidget->item(currentRow, 2)->data(Qt::UserRole).toUInt();

	SVSettings::instance().m_db.removeDBElement(type, rightID, leftID); // remove right, use left instead
	m_dbModified = true;


	// get current index
	updateUi();
	currentRow = std::min(m_ui->tableWidget->rowCount()-1, currentRow);
	m_ui->tableWidget->selectionModel()->blockSignals(true);
	if (currentRow != -1) {
		m_ui->tableWidget->selectRow(currentRow);
		onCurrentRowChanged(m_ui->tableWidget->currentIndex(), QModelIndex());
	}
	m_ui->tableWidget->selectionModel()->blockSignals(false);
}


void SVDBDuplicatesDialog::on_pushButtonTakeRight_clicked() {
	// get current index
	int currentRow = m_ui->tableWidget->currentRow();
	Q_ASSERT(currentRow != -1);

	SVDatabase::DatabaseTypes type = (SVDatabase::DatabaseTypes)m_ui->tableWidget->item(currentRow, 0)->data(Qt::UserRole).toInt();
	unsigned int leftID = m_ui->tableWidget->item(currentRow, 1)->data(Qt::UserRole).toUInt();
	unsigned int rightID = m_ui->tableWidget->item(currentRow, 2)->data(Qt::UserRole).toUInt();

	SVSettings::instance().m_db.removeDBElement(type, leftID, rightID); // remove left, use right instead
	m_dbModified = true;


	updateUi();
	currentRow = std::min(m_ui->tableWidget->rowCount()-1, currentRow);
	m_ui->tableWidget->selectionModel()->blockSignals(true);
	if (currentRow != -1) {
		m_ui->tableWidget->selectRow(currentRow);
		onCurrentRowChanged(m_ui->tableWidget->currentIndex(), QModelIndex());
	}
	m_ui->tableWidget->selectionModel()->blockSignals(false);
}


void SVDBDuplicatesDialog::updateUi() {
	// populate table

	const SVDatabase & db = SVSettings::instance().m_db;

	std::vector< std::vector<SVDatabase::DuplicateInfo> > dupInfos(SVDatabase::NUM_DT);
	db.determineDuplicates(dupInfos);

	if (m_dbType != SVDatabase::NUM_DT) {
		// remove all but the selected DB type
		for (unsigned int i=0; i<SVDatabase::NUM_DT; ++i) {
			if (i == m_dbType) continue;
			dupInfos[i].clear();
		}
	}

	m_ui->tableWidget->selectionModel()->blockSignals(true);
	// now populate table
	int rows = 0;
	for (unsigned int i=0; i<SVDatabase::NUM_DT; ++i) {
		m_ui->tableWidget->setRowCount(rows + (int)dupInfos[i].size());
		for (unsigned int j=0; j<dupInfos[i].size(); ++j) {
			const SVDatabase::DuplicateInfo & duplicates = dupInfos[i][j];
			QTableWidgetItem * item = new QTableWidgetItem;
			QString left, right;
			const VICUS::AbstractDBElement * dbElemLeft = nullptr;
			const VICUS::AbstractDBElement * dbElemRight = nullptr;
			switch ((SVDatabase::DatabaseTypes)i) {
				case SVDatabase::DT_Materials:
					item->setText(tr("Materials"));
					dbItem(db.m_materials, duplicates.m_idFirst, duplicates.m_idSecond, dbElemLeft, dbElemRight);
				break;
				case SVDatabase::DT_Constructions:
					item->setText(tr("Constructions"));
					dbItem(db.m_constructions, duplicates.m_idFirst, duplicates.m_idSecond, dbElemLeft, dbElemRight);
				break;

				// TODO : Katja
				case SVDatabase::DT_Windows:					item->setText(tr("Windows"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_windows[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_windows[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_WindowGlazingSystems:		item->setText(tr("WindowGlazingSystems"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_windowGlazingSystems[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_windowGlazingSystems[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_BoundaryConditions:			item->setText(tr("BoundaryConditions"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_boundaryConditions[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_boundaryConditions[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Components:					item->setText(tr("Components"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_components[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_components[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_SubSurfaceComponents:		item->setText(tr("SubSurfaceComponents"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_subSurfaceComponents[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_subSurfaceComponents[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_SurfaceHeating:				item->setText(tr("SurfaceHeating"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_surfaceHeatings[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_surfaceHeatings[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Pipes:						item->setText(tr("Pipes"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_pipes[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_pipes[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Fluids:						item->setText(tr("Fluids"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_fluids[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_fluids[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_NetworkComponents:			item->setText(tr("NetworkComponents"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_networkComponents[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_networkComponents[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_NetworkControllers:			item->setText(tr("NetworkControllers"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_networkControllers[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_networkControllers[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_SubNetworks:				item->setText(tr("SubNetworks"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_subNetworks[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_subNetworks[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Schedules:					item->setText(tr("Schedules"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_schedules[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_schedules[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_InternalLoads:				item->setText(tr("InternalLoads"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_internalLoads[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_internalLoads[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_ZoneControlThermostat:		item->setText(tr("ZoneControlThermostat"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlThermostat[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlThermostat[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_ZoneControlShading:			item->setText(tr("ZoneControlShading"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlShading[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlShading[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_ZoneControlNaturalVentilation:	item->setText(tr("ZoneControlNaturalVentilation"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlVentilationNatural[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlVentilationNatural[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_ZoneIdealHeatingCooling:	item->setText(tr("ZoneIdealHeatingCooling"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneIdealHeatingCooling[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneIdealHeatingCooling[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_VentilationNatural:			item->setText(tr("VentilationNatural"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_ventilationNatural[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_ventilationNatural[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Infiltration:				item->setText(tr("Infiltration"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_infiltration[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_infiltration[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_ZoneTemplates:				item->setText(tr("ZoneTemplates"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneTemplates[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneTemplates[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::NUM_DT:;// just to make compiler happy
			}

			if (dbElemLeft != nullptr) {
				left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(dbElemLeft->m_displayName) ).arg(duplicates.m_idFirst);
				right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(dbElemRight->m_displayName) ).arg(duplicates.m_idSecond);
			}
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(Qt::UserRole, i); // item(row,0) holds DB element type as data
			m_ui->tableWidget->setItem(rows+(int)j, 0, item);

			item = new QTableWidgetItem(left);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			// built-in items will be bold with special background
			if (dbElemLeft != nullptr && dbElemLeft->m_builtIn) {
				QBrush b;
				if (rows % 2 == 0)
					b = QBrush(SVStyle::instance().m_alternativeBackgroundDark);
				else
					b = QBrush(SVStyle::instance().m_alternativeBackgroundBright);
				item->setBackground(b);
				QFont f = item->font();
				f.setBold(true);
				item->setFont(f);
			}
			item->setData(Qt::UserRole, duplicates.m_idFirst); // item(row,1) holds DB element ID of first element
			if (duplicates.m_identical)
				item->setForeground(QColor("#3030e0"));
			m_ui->tableWidget->setItem(rows+(int)j, 1, item);

			item = new QTableWidgetItem(right);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			if (dbElemRight != nullptr && dbElemRight->m_builtIn) {
				QBrush b;
				if (rows % 2 == 0)
					b = QBrush(SVStyle::instance().m_alternativeBackgroundDark);
				else
					b = QBrush(SVStyle::instance().m_alternativeBackgroundBright);
				item->setBackground(b);
				QFont f = item->font();
				f.setBold(true);
				item->setFont(f);
			}
			item->setData(Qt::UserRole, duplicates.m_idSecond); // item(row,2) holds DB element ID of second element
			if (duplicates.m_identical)
				item->setForeground(QColor("#3030e0"));
			m_ui->tableWidget->setItem(rows+(int)j, 2, item);
		}
		rows += dupInfos[i].size();
	}

	m_ui->groupBox->setVisible(false);

	if (m_ui->tableWidget->rowCount() != 0) {
		m_ui->tableWidget->selectRow(0);
		onCurrentRowChanged(m_ui->tableWidget->currentIndex(), QModelIndex());
	}
	m_ui->tableWidget->selectionModel()->blockSignals(false);
}
