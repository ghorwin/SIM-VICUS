#include "SVDBScheduleEditWidget.h"
#include "ui_SVDBScheduleEditWidget.h"

#include <QDate>

#include "SVConstants.h"
#include "SVSettings.h"

#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <VICUS_Schedule.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_DateTimeInputDialog.h>

#include "SVDBScheduleTableModel.h"

SVDBScheduleEditWidget::SVDBScheduleEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBScheduleEditWidget)
{
	m_ui->setupUi(this);

	//add header to periods table
	m_ui->tableWidgetPeriods->setColumnCount(2);
	m_ui->tableWidgetPeriods->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Date")));
	m_ui->tableWidgetPeriods->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Name")));

	///TODO Dirk->Andreas wie bringe ich jetzt das Widget hier rein?


	///TODO Dirk
	//aufheben für später
	//muss in ein separaten Dialog ausgelagert werden
	{
		m_ui->comboBoxScheduleType->setVisible(false);
		m_ui->radioButtonConstant->setVisible(false);
		m_ui->radioButtonLinearInterpolation->setVisible(false);
		m_ui->labelScheduleType->setVisible(false);
		m_ui->labelScheduleType_2->setVisible(false);
	}

	// initial state is "nothing selected"
	updateInput(-1);
}



SVDBScheduleEditWidget::~SVDBScheduleEditWidget() {
	delete m_ui;
}

void SVDBScheduleEditWidget::setup(SVDatabase * db, SVDBScheduleTableModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}

void SVDBScheduleEditWidget::updatePeriodTable(){
	if(m_current->m_periods.empty()){
		m_current->m_periods.push_back(VICUS::ScheduleInterval());
		//set start day to first day of year (0)
		m_current->m_periods.front().m_intervalStartDay = 0;
	}

	//create a julian day to get the right date in dd.MM.
	qint64 julianD = QDate(2021,1,1).toJulianDay();

	//set up all periods with name and day
	m_ui->tableWidgetPeriods->setRowCount(m_current->m_periods.size());
	for(unsigned int i=0; i<m_current->m_periods.size(); ++i){
		///TODO Dirk->Andreas wo wird der Name der Periode gespeichert? Ich habe diesen jetzt im Scheduleinterval hinzugefügt. Ist das richtig?

		unsigned int startDay =m_current->m_periods[i].m_intervalStartDay;

		m_ui->tableWidgetPeriods->setItem(i,0,new QTableWidgetItem(QDate::fromJulianDay(julianD+startDay).toString("dd.MM.")));
		///TODO Dirk->Andreas wie gehen wir hier mit Multilanguage strings um oder soll das umgebaut werden?
		m_ui->tableWidgetPeriods->setItem(i,1,new QTableWidgetItem(QString::fromStdString(m_current->m_periods[i].m_displayName.string())));
	}
}


void SVDBScheduleEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = (id == -1 ? false : true);

	//set buttons
	//m_ui->toolButtonForward->setEnabled(false);
	//m_ui->toolButtonBackward->setEnabled(false);
	m_ui->toolButtonAddPeriod->setEnabled(isEnabled);
	m_ui->toolButtonCopyPeriod->setEnabled(isEnabled);
	m_ui->toolButtonRemovePeriode->setEnabled(isEnabled);

	m_ui->widgetDailyCycleAndDayTypes->setEnabled(false);

	//set checkboxes
//	m_ui->checkBoxMonday->setEnabled(false);
//	m_ui->checkBoxTuesday->setEnabled(false);
//	m_ui->checkBoxWednesday->setEnabled(false);
//	m_ui->checkBoxThursday->setEnabled(false);
//	m_ui->checkBoxFriday->setEnabled(false);
//	m_ui->checkBoxSaturday->setEnabled(false);
//	m_ui->checkBoxSunday->setEnabled(false);
//	m_ui->checkBoxHoliday->setEnabled(false);

	//set table views
//	m_ui->tableWidgetDayCycle->setEnabled(false);
	m_ui->tableWidgetPeriods->setEnabled(isEnabled);

	if (!isEnabled) {
		// clear input controls

		return;
	}

	m_current = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) id ]);

	// update table widget with periods
	// select first period -> call selectionChangedSlot() which sets up the remainder of the UI

	//period schedule
	if(m_current->m_annualSchedule.x().empty()){
		//check that this schedule has a period
		//if not create first period
		updatePeriodTable();
	}
	//annualSchedule
	else{

	}



/*


	VICUS::BoundaryCondition * bc = const_cast<VICUS::BoundaryCondition *>(m_db->m_boundaryConditions[(unsigned int)id]);
	m_current = bc;

	// now update the GUI controls
	m_ui->lineEditName->setString(bc->m_displayName);

	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(true);
	m_ui->comboBoxHeatTransferCoeffModelType->setCurrentIndex(m_ui->comboBoxHeatTransferCoeffModelType->findData(bc->m_heatConduction.m_modelType));
	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(false);
	// update model-specific input states
	on_comboBoxHeatTransferCoeffModelType_currentIndexChanged(m_ui->comboBoxHeatTransferCoeffModelType->currentIndex());

	m_ui->comboBoxLWModelType->blockSignals(true);
	m_ui->comboBoxLWModelType->setCurrentIndex(m_ui->comboBoxLWModelType->findData(bc->m_longWaveEmission.m_modelType));
	m_ui->comboBoxLWModelType->blockSignals(false);
	// update model-specific input states
	on_comboBoxLWModelType_currentIndexChanged(m_ui->comboBoxLWModelType->currentIndex());

	m_ui->comboBoxSWModelType->blockSignals(true);
	m_ui->comboBoxSWModelType->setCurrentIndex(m_ui->comboBoxSWModelType->findData(bc->m_solarAbsorption.m_modelType));
	m_ui->comboBoxSWModelType->blockSignals(false);
	// update model-specific input states
	on_comboBoxSWModelType_currentIndexChanged(m_ui->comboBoxSWModelType->currentIndex());

	m_ui->lineEditHeatTransferCoefficient->setValue(bc->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value);
	m_ui->lineEditSolarAbsorptionCoefficient->setValue(bc->m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].value);
	m_ui->lineEditLongWaveEmissivity->setValue(bc->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value);

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (bc->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
	m_ui->lineEditSolarAbsorptionCoefficient->setReadOnly(!isEditable);
	m_ui->lineEditLongWaveEmissivity->setReadOnly(!isEditable);
	m_ui->lineEditHeatTransferCoefficient->setReadOnly(!isEditable);
	m_ui->comboBoxHeatTransferCoeffModelType->setEnabled(isEditable);
	m_ui->comboBoxLWModelType->setEnabled(isEditable);
	m_ui->comboBoxSWModelType->setEnabled(isEditable);
*/
}

void SVDBScheduleEditWidget::on_toolButtonAddPeriod_clicked(){
	Q_ASSERT(m_current != nullptr);

	// request start date
	QDate initialDate(2021,1,1);
	QDate startDate = QtExt::DateTimeInputDialog::requestDate(tr("Select start date of period"), tr("Enter start date (dd.MM.):"), tr("dd.MM."), &initialDate);

	if(!startDate.isValid()){
		///TODO Fehlermeldung für den Nutzer ohne Exception oder mit?
		//The period is not valid. Action canceled.
	}

	// convert date to dayofyear
	unsigned int startDateInt = startDate.dayOfYear()-1;
	unsigned int idx=0;
	// check if such a period starting day has already been used, and if yes,
	for(unsigned int i=0; i<m_current->m_periods.size(); ++i){
		const VICUS::ScheduleInterval &schedInt = m_current->m_periods[i];
		if(schedInt.m_intervalStartDay == startDateInt) {
			///TODO Dirk Fehlermeldung
			//QMessageBox("Error", "A Period with this start day already exists.");

			//abort function do not enter a new scheduleInterval
			return;
		}
		//save index for later adding schedule interval
		if(schedInt.m_intervalStartDay < startDateInt)
			idx=i;
	}
	// show error message

	// now create a new ScheduleInverval and insert into vector at appropriate position (sorted) and
	VICUS::ScheduleInterval schedInt;
	schedInt.m_intervalStartDay = startDateInt;
	schedInt.m_displayName.setString("newInterval", "de");

	// get resulting index of new ScheduleInverval in vector
	m_current->m_periods.insert(m_current->m_periods.begin()+idx+1,schedInt);

	// update table widget
	updatePeriodTable();

	// select ScheduleInverval table row by ScheduleInverval index -> this will show the editor for the newly created schedule
	m_ui->tableWidgetPeriods->selectRow(idx+1);
}



void SVDBScheduleEditWidget::on_toolButtonRemovePeriode_clicked(){
	Q_ASSERT(m_current!=nullptr);

	int rowIdx = m_ui->tableWidgetPeriods->currentRow();

	//also look for one period
	if(m_current->m_periods.size()<=1){
		///TODO Dirk->Andreas
		// "The period cannot be deleted. One period must necessarily exist!"
		return;
	}

	//erase period
	if(rowIdx >= 0){
		//erase period
		m_current->m_periods.erase(m_current->m_periods.begin() + rowIdx);
		//if first period is erased then change startDay of the next period to 0
		if( rowIdx == 0)
			m_current->m_periods.front().m_intervalStartDay = 0;
		updatePeriodTable();
	}
	else{
		///TODO Dirk->Andreas kann das passieren und wie würde dann eine Fehlermeldung aussehen?
		//Fehler
	}

}



void SVDBScheduleEditWidget::on_tableWidgetPeriods_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
	Q_ASSERT(m_current == nullptr);

	m_rowIdx = currentRow;
	m_ui->widgetDailyCycleAndDayTypes->setEnabled(m_rowIdx >= 0);

	VICUS::ScheduleInterval &schedInt = m_current->m_periods[m_rowIdx];
	if(schedInt.m_dailyCycles.empty()){
		schedInt.m_dailyCycles.emplace_back(VICUS::DailyCycle());
	}

}
