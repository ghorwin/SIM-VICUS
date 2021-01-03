#include "SVSimulationLocationOptions.h"
#include "ui_SVSimulationLocationOptions.h"

#include <NANDRAD_Location.h>

#include "SVSettings.h"
#include "SVClimateDataTableModel.h"
#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBModelDelegate.h"
#include "SVClimateDataSortFilterProxyModel.h"

SVSimulationLocationOptions::SVSimulationLocationOptions(QWidget *parent, NANDRAD::Location & location) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationLocationOptions),
	m_location(&location)
{
	m_ui->setupUi(this);

	// source model
	m_climateDataModel = SVSettings::instance().climateDataTableModel();

	// proxy model
	m_filterModel = new SVClimateDataSortFilterProxyModel(this);
	m_filterModel->setSourceModel(m_climateDataModel);

	// set proxy model into table
	m_ui->tableViewClimateFiles->setModel(m_filterModel);

	SVDBModelDelegate * delegate = new SVDBModelDelegate(this, Role_BuiltIn);
	m_ui->tableViewClimateFiles->setItemDelegate(delegate);
	SVStyle::formatDatabaseTableView(m_ui->tableViewClimateFiles);
}


SVSimulationLocationOptions::~SVSimulationLocationOptions() {
	delete m_ui;
}


void SVSimulationLocationOptions::updateUi() {
	m_ui->tableViewClimateFiles->resizeColumnsToContents();

	if (m_location->m_climateFileName.isValid()) {
		// is the referenced file in the climate database?
		QModelIndex idx;
		for (int i=0, count = m_climateDataModel->rowCount(QModelIndex()); i< count; ++i) {
			QModelIndex curIdx = m_climateDataModel->index(i, 0);
			IBK::Path p(m_climateDataModel->data(curIdx, Role_FilePath).toString().toStdString());
			if (p == m_location->m_climateFileName) {
				idx = curIdx;
				break;
			}
		}
		if (idx.isValid()) {
			// convert to proxy-index
			// if not visible, reset all filters and convert to proxy index again
			// select row
			m_ui->radioButtonFromDB->setChecked(true);
			m_ui->filepathClimateDataFile->blockSignals(true);
			m_ui->filepathClimateDataFile->setFilename("");
			m_ui->filepathClimateDataFile->blockSignals(false);
		}
		else {
			m_ui->filepathClimateDataFile->blockSignals(true);
			m_ui->filepathClimateDataFile->setFilename(QString::fromStdString(m_location->m_climateFileName.str()));
			m_ui->filepathClimateDataFile->blockSignals(false);
		}
	}
	else {
		m_ui->radioButtonFromDB->setChecked(true);
	}
	on_radioButtonFromDB_toggled(m_ui->radioButtonFromDB->isChecked());
}


void SVSimulationLocationOptions::on_radioButtonFromDB_toggled(bool checked) {
	m_ui->tableViewClimateFiles->setEnabled(checked);
	m_ui->labelTextFilter->setEnabled(checked);
	m_ui->lineEditTextFilter->setEnabled(checked);
	m_ui->filepathClimateDataFile->setEnabled(!checked);
}


void SVSimulationLocationOptions::on_lineEditTextFilter_editingFinished() {
	// update the filter text
	m_filterModel->setFilterWildcard(m_ui->lineEditTextFilter->text().trimmed());
}


void SVSimulationLocationOptions::on_lineEditTextFilter_textChanged(const QString &arg1) {
	m_filterModel->setFilterWildcard(arg1);
}
