#include "SVDBDuplicatesDialog.h"
#include "ui_SVDBDuplicatesDialog.h"

#include <QtExt_Conversions.h>
#include <QHeaderView>

#include <IBK_StringUtils.h>

#include <tinyxml.h>

#include "SVSettings.h"
#include "SVStyle.h"

SVDBDuplicatesDialog::SVDBDuplicatesDialog(QWidget *parent) :
	QDialog(parent),
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


void SVDBDuplicatesDialog::removeDuplicates(SVDatabase::DatabaseTypes dbType) {
	// populate table

	const SVDatabase & db = SVSettings::instance().m_db;

	std::vector< std::vector<SVDatabase::DuplicateInfo> > dupInfos(SVDatabase::NUM_DT);
	db.determineDuplicates(dupInfos);

	if (dbType != SVDatabase::NUM_DT) {
		// remove all but the selected DB type
		for (unsigned int i=0; i<SVDatabase::NUM_DT; ++i) {
			if (i == dbType) continue;
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
			switch ((SVDatabase::DatabaseTypes)i) {
				case SVDatabase::DT_Materials:					item->setText(tr("Materials"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_materials[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_materials[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Constructions:				item->setText(tr("Constructions"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_constructions[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_constructions[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_Windows:					item->setText(tr("Windows")); break;
				case SVDatabase::DT_WindowGlazingSystems:		item->setText(tr("WindowGlazingSystems")); break;
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
				case SVDatabase::DT_SurfaceHeating:				item->setText(tr("SurfaceHeating")); break;
				case SVDatabase::DT_Pipes:						item->setText(tr("Pipes")); break;
				case SVDatabase::DT_Fluids:						item->setText(tr("Fluids")); break;
				case SVDatabase::DT_NetworkComponents:			item->setText(tr("NetworkComponents")); break;
				case SVDatabase::DT_NetworkControllers:			item->setText(tr("NetworkControllers")); break;
				case SVDatabase::DT_SubNetworks:				item->setText(tr("SubNetworks")); break;
				case SVDatabase::DT_Schedules:					item->setText(tr("Schedules")); break;
				case SVDatabase::DT_InternalLoads:				item->setText(tr("InternalLoads")); break;
				case SVDatabase::DT_ZoneControlThermostat:		item->setText(tr("ZoneControlThermostat")); break;
				case SVDatabase::DT_ZoneControlShading:			item->setText(tr("ZoneControlShading")); break;
				case SVDatabase::DT_ZoneControlNaturalVentilation:	item->setText(tr("ZoneControlNaturalVentilation")); break;
				case SVDatabase::DT_ZoneIdealHeatingCooling:	item->setText(tr("ZoneIdealHeatingCooling")); break;
				case SVDatabase::DT_VentilationNatural:			item->setText(tr("VentilationNatural")); break;
				case SVDatabase::DT_Infiltration:				item->setText(tr("Infiltration")); break;
				case SVDatabase::DT_ZoneTemplates:				item->setText(tr("ZoneTemplates")); break;
				case SVDatabase::NUM_DT:;// just to make compiler happy
			}
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(Qt::UserRole, i); // item(row,0) holds DB element type as data
			m_ui->tableWidget->setItem(rows+(int)j, 0, item);

			item = new QTableWidgetItem(left);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(Qt::UserRole, duplicates.m_idFirst); // item(row,1) holds DB element ID of first element
			if (duplicates.m_identical)
				item->setBackground(QColor("#d0e0ff"));
			m_ui->tableWidget->setItem(rows+(int)j, 1, item);

			item = new QTableWidgetItem(right);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(Qt::UserRole, duplicates.m_idSecond); // item(row,1) holds DB element ID of second element
			if (duplicates.m_identical)
				item->setBackground(QColor("#d0e0ff"));
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

	exec();
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


void SVDBDuplicatesDialog::onCurrentRowChanged(const QModelIndex & current, const QModelIndex & /*previous*/) {
	m_ui->groupBox->setVisible(true);
	// take currently selected items, access them and generate their diffs
	int currentRow = current.row();
	SVDatabase::DatabaseTypes type = (SVDatabase::DatabaseTypes)m_ui->tableWidget->item(currentRow, 0)->data(Qt::UserRole).toInt();
	unsigned int leftID = m_ui->tableWidget->item(currentRow, 1)->data(Qt::UserRole).toUInt();
	unsigned int rightID = m_ui->tableWidget->item(currentRow, 2)->data(Qt::UserRole).toUInt();
	QString xmlLeft, xmlRight;
	const SVDatabase & db = SVSettings::instance().m_db;
	switch (type) {
		case SVDatabase::DT_Materials:
			xmlLeft = dumpXML(*db.m_materials[leftID]);
			xmlRight = dumpXML(*db.m_materials[rightID]);
		break;
		case SVDatabase::DT_Constructions:
			xmlLeft = dumpXML(*db.m_constructions[leftID]);
			xmlRight = dumpXML(*db.m_constructions[rightID]);
		break;
		break;
		case SVDatabase::DT_Windows:
		break;
		case SVDatabase::DT_WindowGlazingSystems:
		break;
		case SVDatabase::DT_BoundaryConditions:
		break;
		case SVDatabase::DT_Components:
		break;
		case SVDatabase::DT_SubSurfaceComponents:
		break;
		case SVDatabase::DT_SurfaceHeating:
		break;
		case SVDatabase::DT_Pipes:
		break;
		case SVDatabase::DT_Fluids:
		break;
		case SVDatabase::DT_NetworkComponents:
		break;
		case SVDatabase::DT_NetworkControllers:
		break;
		case SVDatabase::DT_SubNetworks:
		break;
		case SVDatabase::DT_Schedules:
		break;
		case SVDatabase::DT_InternalLoads:
		break;
		case SVDatabase::DT_ZoneControlThermostat:
		break;
		case SVDatabase::DT_ZoneControlShading:
		break;
		case SVDatabase::DT_ZoneControlNaturalVentilation:
		break;
		case SVDatabase::DT_ZoneIdealHeatingCooling:
		break;
		case SVDatabase::DT_VentilationNatural:
		break;
		case SVDatabase::DT_Infiltration:
		break;
		case SVDatabase::DT_ZoneTemplates:
		break;
		case SVDatabase::NUM_DT:
		break;
	}

	// generate diff and color output

	// split and create vector of lines
	std::vector<std::string> linesLeft = processXML(xmlLeft.toStdString());
	std::vector<std::string> linesRight = processXML(xmlRight.toStdString());

	std::string encodedLeft = IBK::convertXml2Html( IBK::join(linesLeft, '\n') );
	std::string encodedRight = IBK::convertXml2Html(  IBK::join(linesRight, '\n') );

	const char * const htmlPrefix = "<html><body><pre style=\"font-size:9pt;\">";
	const char * const htmlSuffix = "</pre></body></html>";

	QString formattedHtmlLeft = htmlPrefix + QString::fromStdString(encodedLeft) + htmlSuffix;
	QString formattedHtmlRight = htmlPrefix + QString::fromStdString(encodedRight) + htmlSuffix;

	m_ui->textEditLeft->setHtml(formattedHtmlLeft);
	m_ui->textEditRight->setHtml(formattedHtmlRight);
}
