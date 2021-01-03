#include "SVSimulationLocationOptions.h"
#include "ui_SVSimulationLocationOptions.h"

#include "SVSettings.h"
#include "SVClimateDataTableModel.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"

SVSimulationLocationOptions::SVSimulationLocationOptions(QWidget *parent, NANDRAD::Location & location) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationLocationOptions),
	m_location(&location)
{
	m_ui->setupUi(this);
	QAbstractTableModel  * model = SVSettings::instance().climateDataTableModel();
	m_ui->tableViewClimateFiles->setModel(model);

	SVDBModelDelegate * delegate = new SVDBModelDelegate(this, Role_BuiltIn);
	m_ui->tableViewClimateFiles->setItemDelegate(delegate);
	SVStyle::formatDatabaseTableView(m_ui->tableViewClimateFiles);
}


SVSimulationLocationOptions::~SVSimulationLocationOptions() {
	delete m_ui;
}


void SVSimulationLocationOptions::updateUi() {
	m_ui->tableViewClimateFiles->resizeColumnsToContents();

}
