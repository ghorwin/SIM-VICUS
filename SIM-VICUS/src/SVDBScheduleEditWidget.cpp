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
#include <QtExt_Conversions.h>

#include "SVDBScheduleTableModel.h"
#include "SVDBScheduleDailyCycleEditWidget.h"
#include "SVStyle.h"

SVDBScheduleEditWidget::SVDBScheduleEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBScheduleEditWidget)
{
	m_ui->setupUi(this);

	//add header to periods table
	m_ui->tableWidgetPeriods->setColumnCount(2);
	m_ui->tableWidgetPeriods->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Date")));
	m_ui->tableWidgetPeriods->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Name")));

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetPeriods);
	m_ui->tableWidgetPeriods->setSortingEnabled(false);

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

void SVDBScheduleEditWidget::updatePeriodTable(const int &activeRow){
	//int currRow = m_ui->tableWidgetPeriods->currentRow();
	m_ui->tableWidgetPeriods->blockSignals(true);

	//create a julian day to get the right date in dd.MM.
	qint64 julianD = QDate(2021,1,1).toJulianDay();

	///TODO Stephan
	//sort the periods

	//set up all periods with name and day
	m_ui->tableWidgetPeriods->setRowCount(m_current->m_periods.size());
	for(unsigned int i=0; i<m_current->m_periods.size(); ++i){
		unsigned int startDay =m_current->m_periods[i].m_intervalStartDay;
		QTableWidgetItem *itemDate = new QTableWidgetItem(QDate::fromJulianDay(julianD+startDay).toString(tr("dd.MM.") ) );
		itemDate->setFlags(itemDate->flags() ^ Qt::ItemIsEditable);
		m_ui->tableWidgetPeriods->setItem(i,0,itemDate);
		m_ui->tableWidgetPeriods->setItem(i,1,new QTableWidgetItem(QtExt::MultiLangString2QString(m_current->m_periods[i].m_displayName)));
	}
	///TODO reselect row default to row 0

	on_tableWidgetPeriods_currentCellChanged(0,0,0,0);
	if(!m_current->m_periods.empty())
		m_ui->tableWidgetPeriods->selectRow(0);
	m_ui->tableWidgetPeriods->blockSignals(false);

	on_tableWidgetPeriods_currentCellChanged(activeRow,0,0,0);

	//is more than one period left
	//remove button activate
	m_ui->toolButtonRemovePeriode->setEnabled(m_current->m_periods.size()>1);

}

void SVDBScheduleEditWidget::selectDailyCycle() {
	// create first daily cycle if none exist yet
	if(m_currentInterval->m_dailyCycles.empty())
		m_currentInterval->m_dailyCycles.push_back(VICUS::DailyCycle());

	// enable/disable arrow buttons based
	m_ui->toolButtonBackward->setEnabled(m_currentDailyCycleIndex!=0);

	// enable forward button:
	//
	if(m_currentDailyCycleIndex<m_currentInterval->m_dailyCycles.size()-1 || m_currentInterval->freeDayTypes().size()>0){
		m_ui->toolButtonBackward->setEnabled(m_currentDailyCycleIndex!=0);
	}

	// check/uncheck day checkbox (with blocked signals)
	m_ui->widgetDayTypes->blockSignals(true);

	//all button active and enabled
	m_ui->checkBoxMonday->setEnabled(true);
	m_ui->checkBoxTuesday->setEnabled(true);
	m_ui->checkBoxWednesday->setEnabled(true);
	m_ui->checkBoxThursday->setEnabled(true);
	m_ui->checkBoxFriday->setEnabled(true);
	m_ui->checkBoxSaturday->setEnabled(true);
	m_ui->checkBoxSunday->setEnabled(true);
	m_ui->checkBoxHoliday->setEnabled(true);
	m_ui->checkBoxMonday->setChecked(false);
	m_ui->checkBoxTuesday->setChecked(false);
	m_ui->checkBoxWednesday->setChecked(false);
	m_ui->checkBoxThursday->setChecked(false);
	m_ui->checkBoxFriday->setChecked(false);
	m_ui->checkBoxSaturday->setChecked(false);
	m_ui->checkBoxSunday->setChecked(false);
	m_ui->checkBoxHoliday->setChecked(false);

	for(unsigned int i=0; i< m_currentInterval->m_dailyCycles.size(); ++i){
		bool enabled = false;
		if(i==m_currentDailyCycleIndex)
			enabled = true;

		for(unsigned int j=0; j<m_currentInterval->m_dailyCycles[i].m_dayTypes.size(); ++j){
			int dt = m_currentInterval->m_dailyCycles[i].m_dayTypes[j];
			switch (dt) {
				case NANDRAD::Schedule::ST_MONDAY:{
					m_ui->checkBoxMonday->setChecked(true);
					m_ui->checkBoxMonday->setEnabled(enabled);
				} break;
				case NANDRAD::Schedule::ST_TUESDAY:{
					m_ui->checkBoxTuesday->setChecked(true);
					m_ui->checkBoxTuesday->setEnabled(enabled);
				} break;
				case NANDRAD::Schedule::ST_WEDNESDAY:{
					m_ui->checkBoxWednesday->setChecked(true);
					m_ui->checkBoxWednesday->setEnabled(enabled);
				} break;
				case NANDRAD::Schedule::ST_THURSDAY:{
					m_ui->checkBoxThursday->setChecked(true);
					m_ui->checkBoxThursday->setEnabled(enabled);
				} break;
				case NANDRAD::Schedule::ST_FRIDAY:{
					m_ui->checkBoxFriday->setChecked(true);
					m_ui->checkBoxFriday->setEnabled(enabled);
				} break;
				case NANDRAD::Schedule::ST_SATURDAY:{
					m_ui->checkBoxSaturday->setChecked(true);
					m_ui->checkBoxSaturday->setEnabled(enabled);
				} break;
				case NANDRAD::Schedule::ST_SUNDAY:{
					m_ui->checkBoxSunday->setChecked(true);
					m_ui->checkBoxSunday->setEnabled(enabled);
				} break;
				case NANDRAD::Schedule::ST_HOLIDAY:{
					m_ui->checkBoxHoliday->setChecked(true);
					m_ui->checkBoxHoliday->setEnabled(enabled);
				} break;
			}
		}
	}
	m_ui->widgetDayTypes->blockSignals(false);
	// update current daily cycle data type and chartt

	VICUS::DailyCycle *dc = &m_currentInterval->m_dailyCycles[m_currentDailyCycleIndex];
	m_ui->widgetDailyCycle->updateInput( dc , m_db);

	m_db->m_schedules.m_modified=true;
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
	// we must a valid schedule pointer
	Q_ASSERT(m_current != nullptr);

	///TODO Annual Schedule ...

	// update table widget with periods
	// select first period -> call selectionChangedSlot() which sets up the remainder of the UI

	//period schedule
	if(m_current->m_annualSchedule.x().empty()){

		//initialize period with one period
		if(m_current->m_periods.empty()){
			m_current->m_periods.push_back(VICUS::ScheduleInterval());
			m_db->m_schedules.m_modified=true;
		}
		m_ui->tableWidgetPeriods->selectRow(0);

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
	if ( m_currentInterval != nullptr)
		initialDate = initialDate.addDays(m_currentInterval->m_intervalStartDay+1);

	QDate startDate = QtExt::DateTimeInputDialog::requestDate(tr("Select start date of period"),
															  tr("Enter start date (dd.MM.):"), tr("dd.MM."), &initialDate);

	if(!startDate.isValid())
		return;		//The period is not valid. Action canceled.

	// convert date to dayofyear
	unsigned int startDateInt = startDate.dayOfYear()-1;
	unsigned int idx=0;
	// check if such a period starting day has already been used, and if yes,
	for(unsigned int i=0; i<m_current->m_periods.size(); ++i){
		const VICUS::ScheduleInterval &schedInt = m_current->m_periods[i];
		if(schedInt.m_intervalStartDay == startDateInt) {
			QMessageBox::critical(this,QString(), "A period with this start day already exists.");
			return;
		}
		//save index for later adding schedule interval
		if(schedInt.m_intervalStartDay < startDateInt)
			idx=i;
	}

	// now create a new ScheduleInverval and insert into vector at appropriate position (sorted) and
	VICUS::ScheduleInterval schedInt;
	schedInt.m_intervalStartDay = startDateInt;
	schedInt.m_displayName.setString("newInterval", "de");

	// get resulting index of new ScheduleInverval in vector
	m_current->m_periods.insert(m_current->m_periods.begin()+idx+1,schedInt);

	// update table widget
	updatePeriodTable(m_current->m_periods.size()-1 );

	// select ScheduleInverval table row by ScheduleInverval index -> this will show the editor for the newly created schedule
	m_ui->tableWidgetPeriods->selectRow(idx+1);
}



void SVDBScheduleEditWidget::on_toolButtonRemovePeriode_clicked(){
	Q_ASSERT(m_current!=nullptr);

	int rowIdx = m_ui->tableWidgetPeriods->currentRow();

	//erase period
	m_current->m_periods.erase(m_current->m_periods.begin() + rowIdx);
	//if first period is erased then change startDay of the next period to 0
	if( rowIdx == 0)
		m_current->m_periods.front().m_intervalStartDay = 0;
	updatePeriodTable();

}



void SVDBScheduleEditWidget::on_tableWidgetPeriods_currentCellChanged(int currentRow, int currentColumn, int /*previousRow*/, int /*previousColumn*/)
{
	Q_ASSERT(m_current != nullptr);

	///TODO Stephan
	Q_ASSERT(currentRow < m_ui->tableWidgetPeriods->rowCount() );

	m_rowIdx = currentRow;
	m_ui->widgetDailyCycleAndDayTypes->setEnabled(m_rowIdx >= 0);

	m_currentInterval = &m_current->m_periods[m_rowIdx];
	m_currentDailyCycleIndex = 0;

	selectDailyCycle();
}

bool SVDBScheduleEditWidget::isDayTypeChecked(){
	if(!((m_ui->checkBoxMonday->isChecked() && m_ui->checkBoxMonday->isEnabled()) ||
		(m_ui->checkBoxTuesday->isChecked() && m_ui->checkBoxTuesday->isEnabled()) ||
			(m_ui->checkBoxWednesday->isChecked() && m_ui->checkBoxWednesday->isEnabled()) ||
			(m_ui->checkBoxThursday->isChecked() && m_ui->checkBoxThursday->isEnabled()) ||
			(m_ui->checkBoxFriday->isChecked() && m_ui->checkBoxFriday->isEnabled()) ||
			(m_ui->checkBoxSaturday->isChecked() && m_ui->checkBoxSaturday->isEnabled()) ||
			(m_ui->checkBoxSunday->isChecked() && m_ui->checkBoxSunday->isEnabled()) ||
		 (m_ui->checkBoxHoliday->isChecked() && m_ui->checkBoxHoliday->isEnabled())))
		return false;

	return true;

}

bool SVDBScheduleEditWidget::deleteDailyCycle(){

	//if only one daily cycle exist we need a day type
	if(m_currentInterval->m_dailyCycles.size()==1){
		QMessageBox::critical(this,QString(), "Please check one or more day types for this daily cycle.");
		return false;
	}
	//check that on checkbox of day types is checked
	//if not return a message that the daily cycle is delete
		///TODO Dirk->Andreas wie frage ich jetzt ab mit übernahme bei ok des wertes

	if(isDayTypeChecked()){
		bool deleteAction = true;
		if(deleteAction){

			if(m_currentDailyCycleIndex!=0)
				--m_currentDailyCycleIndex;
			return true;
		}
	}

	return false;

}

void SVDBScheduleEditWidget::on_toolButtonBackward_clicked()
{
	Q_ASSERT(m_currentDailyCycleIndex !=0);
	//delete a daily cycle when no day type is selected and we have more than one daily cycle
	//set daily cycle index new
	if(!isDayTypeChecked() && m_currentInterval->m_dailyCycles.size()>1){
		///TODO Dirk->Andreas soll hier eine Abfrage rein ob der Tages-Zeitplan gelöscht werden soll?
		if(QMessageBox::critical(this, QString(), "Do you want to delete this daily cycle?") != QMessageBox::Ok)
			return;
		m_currentInterval->m_dailyCycles.erase(m_currentInterval->m_dailyCycles.begin()+m_currentDailyCycleIndex);
	}
	--m_currentDailyCycleIndex;
	selectDailyCycle();
}

void SVDBScheduleEditWidget::on_toolButtonForward_clicked() {
	if(m_currentDailyCycleIndex < m_currentInterval->m_dailyCycles.size()-1 ||
			m_currentInterval->freeDayTypes().size()>0){

		if(!isDayTypeChecked()){
			///TODO Dirk->Andreas soll hier eine Abfrage rein ob der Tages-Zeitplan gelöscht werden soll?
			if(QMessageBox::critical(this, QString(), "Do you want to delete this daily cycle?") != QMessageBox::Ok)
				return;
			m_currentInterval->m_dailyCycles.erase(m_currentInterval->m_dailyCycles.begin()+m_currentDailyCycleIndex);
		}
		else{
			//if last daily cycle is selected but we have unused day type
			//create a new daily cycle
			if(m_currentDailyCycleIndex == m_currentInterval->m_dailyCycles.size()-1){
				m_db->m_schedules.m_modified;
				m_currentInterval->m_dailyCycles.push_back(VICUS::DailyCycle());
			}
			++m_currentDailyCycleIndex;
		}
	}
	selectDailyCycle();
}

void SVDBScheduleEditWidget::on_tableWidgetPeriods_cellChanged(int row, int column) {
	size_t colIdx = (size_t)column;
	size_t schedIdx = (size_t)row;

	if ( colIdx ==0 )
		return; // we only want to set the display name to our data object

	QString periodName = m_ui->tableWidgetPeriods->item(schedIdx, colIdx)->text();
	m_current->m_periods[schedIdx].m_displayName.setString(periodName.toStdString(), "de");
}



void SVDBScheduleEditWidget::on_tableWidgetPeriods_cellClicked(int row, int column) {
	size_t colIdx = (size_t)column;
	size_t schedIdx = (size_t)row;

	Q_ASSERT( m_current->m_periods.size() > schedIdx );

	if ( colIdx == 1 )
		return; // we only want to set a new start date for an intervall

	if ( schedIdx == 0 )
		return; // we cannot change the start date of the first period

	// we cache our selected periode
	VICUS::ScheduleInterval periode = m_current->m_periods[(size_t)row];

	m_current->m_periods.erase(m_current->m_periods.begin()+row);

	// we take from the periods our selected and take the interval start day
	QDate periodStartDate(2021,1,1);

	unsigned int shift = periode.m_intervalStartDay;
	periodStartDate = periodStartDate.addDays(shift);
	periodStartDate = QtExt::DateTimeInputDialog::requestDate(tr("Modify start date of period"), tr("Enter start date (dd.MM.):"), tr("dd.MM."),									&periodStartDate);

	if ( !periodStartDate.isValid() ) {
		m_current->m_periods.insert(m_current->m_periods.begin()+row, periode);
		return; // no input has been done by user
	}

	// convert date to dayofyear
	unsigned int startDateInt = periodStartDate.dayOfYear()-1;
	unsigned int idx=0;
	// check if such a period starting day has already been used, and if yes,
	for(unsigned int i=0; i<m_current->m_periods.size(); ++i){
		const VICUS::ScheduleInterval &schedInt = m_current->m_periods[i];
		if(schedInt.m_intervalStartDay == startDateInt) {
			QMessageBox::critical(this,QString(), "A period with this start day already exists.");
			m_current->m_periods.insert(m_current->m_periods.begin()+row, periode);
			return;
		}
		//save index for later adding schedule interval
		if(schedInt.m_intervalStartDay < startDateInt)
			idx=i;
	}

	// set new start date
	periode.m_intervalStartDay = startDateInt;

	m_current->m_periods.insert(m_current->m_periods.begin()+idx+1, periode);

	// update table widget
	updatePeriodTable();

}

void SVDBScheduleEditWidget::updateDayTypes(const NANDRAD::Schedule::ScheduledDayType &dt, bool checked){
	VICUS::DailyCycle &dc = m_currentInterval->m_dailyCycles[m_currentDailyCycleIndex];
	//find current indx in vector
	int idx=-1;
	int dayIdx = (int)dt;
	for(unsigned int i=0; i<dc.m_dayTypes.size(); ++i) {
		if( dayIdx == dc.m_dayTypes[i]){
			idx=i;
			break;
		}
	}
	//add day type
	if(checked && idx==-1)
		dc.m_dayTypes.push_back(dayIdx);
	//delete day type
	else if(!checked && idx!=-1)
		dc.m_dayTypes.erase(dc.m_dayTypes.begin()+idx);
	else
		return;

	m_db->m_schedules.m_modified;
}


void SVDBScheduleEditWidget::on_checkBoxMonday_stateChanged(int arg1) {
	//0 unchecked
	//2 checked
	updateDayTypes(NANDRAD::Schedule::ST_MONDAY, arg1==2);
}

void SVDBScheduleEditWidget::on_checkBoxTuesday_stateChanged(int arg1) {
	updateDayTypes(NANDRAD::Schedule::ST_TUESDAY, arg1==2);
}

void SVDBScheduleEditWidget::on_checkBoxHoliday_stateChanged(int arg1) {
	updateDayTypes(NANDRAD::Schedule::ST_HOLIDAY, arg1==2);
}

void SVDBScheduleEditWidget::on_checkBoxWednesday_stateChanged(int arg1) {
	updateDayTypes(NANDRAD::Schedule::ST_WEDNESDAY, arg1==2);
}

void SVDBScheduleEditWidget::on_checkBoxThursday_stateChanged(int arg1) {
	updateDayTypes(NANDRAD::Schedule::ST_THURSDAY, arg1==2);
}

void SVDBScheduleEditWidget::on_checkBoxFriday_stateChanged(int arg1) {
	updateDayTypes(NANDRAD::Schedule::ST_WEDNESDAY, arg1==2);
}

void SVDBScheduleEditWidget::on_checkBoxSaturday_stateChanged(int arg1) {
	updateDayTypes(NANDRAD::Schedule::ST_SATURDAY, arg1==2);
}

void SVDBScheduleEditWidget::on_checkBoxSunday_stateChanged(int arg1) {
	updateDayTypes(NANDRAD::Schedule::ST_SUNDAY, arg1==2);
}
