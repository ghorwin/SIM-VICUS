#include "SVDBDuplicatesDialog.h"
#include "ui_SVDBDuplicatesDialog.h"

#include <SVConversions.h>
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
		case SVDatabase::DT_Materials:				dbItem(db.m_materials, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_Constructions:			dbItem(db.m_constructions, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_Windows:				dbItem(db.m_windows, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_WindowGlazingSystems:	dbItem(db.m_windowGlazingSystems, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_BoundaryConditions:		dbItem(db.m_boundaryConditions, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_Components:				dbItem(db.m_components, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_SubSurfaceComponents:	dbItem(db.m_subSurfaceComponents, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_SurfaceHeating:			dbItem(db.m_surfaceHeatings, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_Pipes:					dbItem(db.m_pipes, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_Fluids:					dbItem(db.m_fluids, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_NetworkComponents:		dbItem(db.m_networkComponents, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_NetworkControllers:		dbItem(db.m_networkControllers, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_SubNetworks:			dbItem(db.m_subNetworks, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_SupplySystems:			dbItem(db.m_supplySystems, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_Schedules:				dbItem(db.m_schedules, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_InternalLoads:			dbItem(db.m_internalLoads, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_ZoneControlThermostat:	dbItem(db.m_zoneControlThermostat, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_ZoneControlShading:		dbItem(db.m_zoneControlShading, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_ZoneControlNaturalVentilation:	dbItem(db.m_zoneControlVentilationNatural, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_ZoneIdealHeatingCooling:	dbItem(db.m_zoneIdealHeatingCooling, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_VentilationNatural:		dbItem(db.m_ventilationNatural, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_Infiltration:			dbItem(db.m_infiltration, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_ZoneTemplates:			dbItem(db.m_zoneTemplates, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_EpdDatasets:			dbItem(db.m_epdDatasets, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_AcousticTemplates:		dbItem(db.m_acousticTemplates, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_AcousticBoundaryConditions: dbItem(db.m_acousticBoundaryConditions, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_AcousticSoundAbsorptions:	dbItem(db.m_acousticSoundAbsorptions, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::DT_AcousticSoundProtectionTemplates:	dbItem(db.m_acousticSoundProtectionTemplates, leftID, rightID, dbElemLeft, dbElemRight); break;
		case SVDatabase::NUM_DT:
		break;
	}

	// get XML description of tag and enabled/disable "take" buttons
	Q_ASSERT(dbElemLeft != nullptr);
	xmlLeft = dumpXML(*dbElemLeft);
	m_ui->pushButtonTakeRight->setEnabled(!dbElemLeft->m_builtIn); // if left is a built-in, we disable the "take right" button
	Q_ASSERT(dbElemRight != nullptr);
	xmlRight = dumpXML(*dbElemRight);
	m_ui->pushButtonTakeLeft->setEnabled(!dbElemRight->m_builtIn); // if right is a built-in, we disable the "take left" button
	// generate diff and color output

	// split and create vector of lines
	std::vector<std::string> linesLeft = processXML(xmlLeft.toStdString());
	std::vector<std::string> linesRight = processXML(xmlRight.toStdString());

	IBK::Differ<std::string> diff(linesLeft, linesRight);
	diff.diff();

	std::string encodedLeft;
	std::string encodedRight;

	const char * LINE_EQUAL_SPAN = "<span style=\"font-size:9pt;color:#606060\">";
	const char * LINE_INSERTED_SPAN = "<span style=\"font-size:9pt;color:#20c050\">";
	const char * LINE_REMOVED_SPAN = "<span style=\"font-size:9pt;color:#c04040\">";
	const char * LINE_DIFF_SPAN = "<span style=\"color:#202080;background-color:#d0d0ff\">";
	const char * CHAR_DIFF_SPAN = "<span style=\"color:#ffffff;background-color:#2020a0\">";

	if (SVSettings::instance().m_theme == SVSettings::TT_Dark) {
		LINE_EQUAL_SPAN = "<span style=\"font-size:9pt;color:#d0d0d0\">";
		LINE_INSERTED_SPAN = "<span style=\"font-size:9pt;color:#40ff70\">";
		LINE_REMOVED_SPAN = "<span style=\"font-size:9pt;color:#ff7060\">";
	}

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

#if 1

					std::vector<char> lineLeft(leftStr.begin(), leftStr.end());
					std::vector<char> lineRight(rightStr.begin(), rightStr.end());
					IBK::Differ<char> lineDiff(lineLeft, lineRight);
					lineDiff.diff();

					std::string mergedLineLeft;
					std::string mergedLineRight;

					bool leftModMode = false;
					bool rightModMode = false;
					// we process now all change markers
					for (unsigned int j=0, chcount = lineDiff.resultObj().size(); j<chcount; ++j) {
						switch (lineDiff.resultOperation()[j]) {
							case IBK::DifferOpEqual : {
								// leave left modification mode?
								if (leftModMode) {
									// if we had already a "different string" span, close it and start a new
									if (!mergedLineLeft.empty()) {
										mergedLineLeft += "</span>";
									}
									mergedLineLeft += LINE_DIFF_SPAN;
									leftModMode = false;
								}

								// leave right modification mode?
								if (rightModMode) {
									// if we had already a "different string" span, close it and start a new
									if (!mergedLineRight.empty()) {
										mergedLineRight += "</span>";
									}
									mergedLineRight += LINE_DIFF_SPAN;
									rightModMode = false;
								}

								// append character
								mergedLineLeft += IBK::convertXml2Html(std::string(1,lineDiff.resultObj()[j]));
								mergedLineRight += IBK::convertXml2Html(std::string(1,lineDiff.resultObj()[j]));
							} break;

							case IBK::DifferOpInsert : {
								// turn on right modification mode?
								if (!rightModMode) {
									if (!mergedLineRight.empty()) {
										mergedLineRight += "</span>";
									}
									mergedLineRight += CHAR_DIFF_SPAN;
									rightModMode = true;
								}
								mergedLineRight += IBK::convertXml2Html(std::string(1,lineDiff.resultObj()[j]));
							} break;

							case IBK::DifferOpRemove : {
								// turn on left modification mode?
								if (!leftModMode) {
									if (!mergedLineLeft.empty()) {
										mergedLineLeft += "</span>";
									}
									mergedLineLeft += CHAR_DIFF_SPAN;
									leftModMode = true;
								}
								mergedLineLeft += IBK::convertXml2Html(std::string(1,lineDiff.resultObj()[j]));
							} break;

						} // switch
					}
					// if we had some content in the string, close the span token
					if (!mergedLineLeft.empty())
						mergedLineLeft += "</span>";
					if (!mergedLineRight.empty())
						mergedLineRight += "</span>";
					encodedLeft = LINE_DIFF_SPAN + mergedLineLeft + "<br>";
					encodedRight = LINE_DIFF_SPAN + mergedLineRight + "<br>";
#else
					encodedLeft += "<span style=\"color:#2020a0;background-color:#c0c0ff\">" + IBK::convertXml2Html(diff.resultObj()[i]) + "</span><br>";
					encodedRight += "<span style=\"color:#2020a0;background-color:#c0c0ff\">" + IBK::convertXml2Html(diff.resultObj()[i+1]) + "</span><br>";
#endif
					++i; // skip over second marker as well
					continue;
				}
			}
		}
		switch (diff.resultOperation()[i]) {
			case IBK::DifferOpEqual :
				encodedLeft += LINE_EQUAL_SPAN + IBK::convertXml2Html(diff.resultObj()[i]) + "</span><br>";
				encodedRight += LINE_EQUAL_SPAN + IBK::convertXml2Html(diff.resultObj()[i]) + "</span><br>";
			break;
			case IBK::DifferOpInsert :
				encodedLeft += LINE_INSERTED_SPAN + IBK::convertXml2Html(diff.resultObj()[i]) + "</span><br>";
				encodedRight += "<br>";
			break;
			case IBK::DifferOpRemove :
				encodedLeft += "<br>";
				encodedRight += LINE_REMOVED_SPAN + IBK::convertXml2Html(diff.resultObj()[i]) + "</span><br>";
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
			if (i == m_dbType)
				continue;
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
				case SVDatabase::DT_Windows:
					item->setText(tr("Windows"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_windows[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_windows[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_WindowGlazingSystems:
					item->setText(tr("WindowGlazingSystems"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_windowGlazingSystems[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_windowGlazingSystems[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_BoundaryConditions:
					item->setText(tr("BoundaryConditions"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_boundaryConditions[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_boundaryConditions[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Components:
					item->setText(tr("Components"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_components[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_components[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_SubSurfaceComponents:
					item->setText(tr("SubSurfaceComponents"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_subSurfaceComponents[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_subSurfaceComponents[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_SurfaceHeating:
					item->setText(tr("SurfaceHeating"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_surfaceHeatings[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_surfaceHeatings[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Pipes:
					item->setText(tr("Pipes"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_pipes[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_pipes[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Fluids:
					item->setText(tr("Fluids"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_fluids[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_fluids[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_NetworkComponents:
					item->setText(tr("NetworkComponents"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_networkComponents[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_networkComponents[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_NetworkControllers:
					item->setText(tr("NetworkControllers"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_networkControllers[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_networkControllers[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_SubNetworks:
					item->setText(tr("SubNetworks"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_subNetworks[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_subNetworks[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_SupplySystems:
					item->setText(tr("SupplySystems"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_supplySystems[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_supplySystems[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Schedules:
					item->setText(tr("Schedules"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_schedules[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_schedules[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_InternalLoads:
					item->setText(tr("InternalLoads"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_internalLoads[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_internalLoads[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_ZoneControlThermostat:
					item->setText(tr("ZoneControlThermostat"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlThermostat[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlThermostat[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_ZoneControlShading:
					item->setText(tr("ZoneControlShading"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlShading[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlShading[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_ZoneControlNaturalVentilation:
					item->setText(tr("ZoneControlNaturalVentilation"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlVentilationNatural[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneControlVentilationNatural[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_ZoneIdealHeatingCooling:
					item->setText(tr("ZoneIdealHeatingCooling"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneIdealHeatingCooling[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneIdealHeatingCooling[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_VentilationNatural:
					item->setText(tr("VentilationNatural"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_ventilationNatural[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_ventilationNatural[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Infiltration:
					item->setText(tr("Infiltration"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_infiltration[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_infiltration[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_ZoneTemplates:
					item->setText(tr("ZoneTemplates"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneTemplates[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_zoneTemplates[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_AcousticBoundaryConditions:
					item->setText(tr("AcousticBoundaryConditions"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_acousticBoundaryConditions[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_acousticBoundaryConditions[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_EpdDatasets:
					item->setText(tr("EpdDatasets"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_epdDatasets[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_epdDatasets[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_AcousticTemplates:
					item->setText(tr("AcousticTemplates"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_acousticTemplates[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_acousticTemplates[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_AcousticSoundProtectionTemplates:
					item->setText(tr("AcousticSoundProtectionTemplates"));
                                        left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_acousticSoundProtectionTemplates[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
                                        right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_acousticSoundProtectionTemplates[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_AcousticSoundAbsorptions:
					item->setText(tr("AcousticSoundAbsorptions"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_acousticSoundAbsorptions[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_acousticSoundAbsorptions[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::NUM_DT:;// just to make compiler happy
				break;
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
