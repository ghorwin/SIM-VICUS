#include "SVDBDuplicatesDialog.h"
#include "ui_SVDBDuplicatesDialog.h"

#include <QtExt_Conversions.h>

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

	SVStyle::formatDatabaseTableView(m_ui->tableWidget);
	m_ui->tableWidget->setSortingEnabled(false);
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
				case SVDatabase::DT_BoundaryConditions:			item->setText(tr("BoundaryConditions")); break;
				case SVDatabase::DT_Components:					item->setText(tr("Components"));
					left = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_components[duplicates.m_idFirst]->m_displayName) ).arg(duplicates.m_idFirst);
					right = tr("%1 [%2]").arg( QtExt::MultiLangString2QString(db.m_components[duplicates.m_idSecond]->m_displayName) ).arg(duplicates.m_idSecond);
					break;
				case SVDatabase::DT_SubSurfaceComponents:		item->setText(tr("SubSurfaceComponents")); break;
				case SVDatabase::DT_SurfaceHeating:				item->setText(tr("SurfaceHeating")); break;
				case SVDatabase::DT_Pipes:						item->setText(tr("Materials")); break;
				case SVDatabase::DT_Fluids:						item->setText(tr("Materials")); break;
				case SVDatabase::DT_NetworkComponents:			item->setText(tr("Materials")); break;
				case SVDatabase::DT_NetworkControllers:			item->setText(tr("Materials")); break;
				case SVDatabase::DT_SubNetworks:				item->setText(tr("Materials")); break;
				case SVDatabase::DT_Schedules:					item->setText(tr("Materials")); break;
				case SVDatabase::DT_InternalLoads:				item->setText(tr("Materials")); break;
				case SVDatabase::DT_ZoneControlThermostat:		item->setText(tr("Materials")); break;
				case SVDatabase::DT_ZoneControlShading:			item->setText(tr("Materials")); break;
				case SVDatabase::DT_ZoneControlNaturalVentilation:	item->setText(tr("Materials")); break;
				case SVDatabase::DT_ZoneIdealHeatingCooling:	item->setText(tr("Materials")); break;
				case SVDatabase::DT_VentilationNatural:			item->setText(tr("Materials")); break;
				case SVDatabase::DT_Infiltration:				item->setText(tr("Materials")); break;
				case SVDatabase::DT_ZoneTemplates:				item->setText(tr("Materials")); break;
				case SVDatabase::NUM_DT:;// just to make compiler happy
			}
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(Qt::UserRole, i); // item(row,0) holds DB element type as data
			m_ui->tableWidget->setItem(rows+(int)j, 0, item);

			item = new QTableWidgetItem(left);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(Qt::UserRole, duplicates.m_idFirst); // item(row,1) holds DB element ID of first element
			m_ui->tableWidget->setItem(rows+(int)j, 1, item);

			item = new QTableWidgetItem(right);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(Qt::UserRole, duplicates.m_idSecond); // item(row,1) holds DB element ID of second element
			m_ui->tableWidget->setItem(rows+(int)j, 2, item);
		}
		rows += dupInfos[i].size();
	}

	m_ui->groupBox->setVisible(false);

	exec();
}
