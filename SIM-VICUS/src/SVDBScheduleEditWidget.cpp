#include "SVDBScheduleEditWidget.h"
#include "ui_SVDBScheduleEditWidget.h"

#include <QDate>

#include "SVConstants.h"
#include "SVSettings.h"

#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <VICUS_Schedule.h>

#include <QtExt_LanguageHandler.h>
//#include <QtExt_DateTimeInputDialog.h>
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
	m_ui->tableWidgetPeriods->setColumnCount(3);
	// Note: valid column is self-explanatory and does not need a caption
	m_ui->tableWidgetPeriods->setHorizontalHeaderLabels(QStringList() << tr("Start date") << QString() << tr("Name"));

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetPeriods);

	m_ui->tableWidgetPeriods->setSortingEnabled(false);

	m_ui->widgetDailyCycleAndDayTypes->layout()->setMargin(0);
	m_ui->widgetPeriod->layout()->setMargin(0);
	m_ui->widgetDayTypes->layout()->setMargin(0);


	// initial state is "nothing selected"
	updateInput(-1);

	QList<int> sizes;
	int widthSchedule = 50;
	int availableWidth = m_ui->splitter->width();
	sizes << widthSchedule << (availableWidth - widthSchedule);
	m_ui->splitter->setSizes(sizes);


	// set period table column sizes

	m_ui->tableWidgetPeriods->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	QFontMetrics fm(m_ui->tableWidgetPeriods->horizontalHeader()->font());
	int width = fm.boundingRect(tr("Start date")).width();
	m_ui->tableWidgetPeriods->setColumnWidth(0, width);
	m_ui->tableWidgetPeriods->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	m_ui->tableWidgetPeriods->setColumnWidth(1, 24);
	m_ui->tableWidgetPeriods->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
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

	//set up all periods with name and day
	m_ui->tableWidgetPeriods->setRowCount(m_current->m_periods.size());
	for(unsigned int i=0; i<m_current->m_periods.size(); ++i){
		unsigned int startDay =m_current->m_periods[i].m_intervalStartDay;
		QTableWidgetItem *itemDate = new QTableWidgetItem(QDate::fromJulianDay(julianD+startDay).toString(tr("dd.MM.") ) );
		itemDate->setFlags(itemDate->flags() ^ Qt::ItemIsEditable);
		m_ui->tableWidgetPeriods->setItem(i,0,itemDate);
		m_ui->tableWidgetPeriods->setItem(i,2,new QTableWidgetItem(QtExt::MultiLangString2QString(m_current->m_periods[i].m_displayName)));
		m_ui->tableWidgetPeriods->setItem(i,1,new QTableWidgetItem());
		m_ui->tableWidgetPeriods->item(i,0)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
		/// TODO Dirk->Andreas wie bekomme ich das bild in die mitte? über textAlignment funktioniert es nicht
		m_ui->tableWidgetPeriods->item(i,1)->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
		m_ui->tableWidgetPeriods->item(i,2)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		if(!m_isEditable){
			m_ui->tableWidgetPeriods->item(i,1)->setFlags(m_ui->tableWidgetPeriods->item(i,1)->flags() ^ Qt::ItemIsEditable);
			m_ui->tableWidgetPeriods->item(i,2)->setFlags(m_ui->tableWidgetPeriods->item(i,1)->flags() ^ Qt::ItemIsEditable);
		}

		if(m_current->m_periods[i].isValid())
			m_ui->tableWidgetPeriods->item(i,1)->setData(Qt::DecorationRole, QIcon("://gfx/actions/16x16/ok.png"));
		else
			m_ui->tableWidgetPeriods->item(i,1)->setData(Qt::DecorationRole, QIcon("://gfx/actions/16x16/error.png"));


	}
	m_ui->tableWidgetPeriods->setCurrentCell(activeRow,1);
	on_tableWidgetPeriods_currentCellChanged(activeRow,0,0,0);
	//m_ui->tableWidgetPeriods->selectRow(activeRow);

	m_ui->tableWidgetPeriods->blockSignals(false);

	//is more than one period left
	//remove button activate
	m_ui->toolButtonRemovePeriode->setEnabled(m_current->m_periods.size()>1);
}


void SVDBScheduleEditWidget::selectDailyCycle() {
	// create first daily cycle if none exist yet
	if (m_currentInterval->m_dailyCycles.empty()) {
		m_currentInterval->m_dailyCycles.push_back(VICUS::DailyCycle());
		m_db->m_schedules.m_modified=true;
	}

	// block signals in all check boxes, set then enabled and unchecked
	for (QObject * w : m_ui->widgetDayTypes->children()) {
		QCheckBox * c = qobject_cast<QCheckBox *>(w);
		if (c != nullptr) {
			c->blockSignals(true);
			c->setEnabled(true);
			c->setChecked(false);
			///TODO Stephan SVStyle
			/// farbe anpassen der checkboxen
			//c->setPalette(QPalette(QPalette::WindowText, Qt::black) );
		}
	}

	QPalette pal(QPalette::WindowText, Qt::blue);

	for (unsigned int i=0; i< m_currentInterval->m_dailyCycles.size(); ++i){
		bool enabled = false;
		if(i==m_currentDailyCycleIndex)
			enabled = true && m_isEditable;

		for(unsigned int j=0; j<m_currentInterval->m_dailyCycles[i].m_dayTypes.size(); ++j){
			int dt = m_currentInterval->m_dailyCycles[i].m_dayTypes[j];
			switch (dt) {
				case NANDRAD::Schedule::ST_MONDAY:{
					m_ui->checkBoxMonday->setChecked(true);
					m_ui->checkBoxMonday->setEnabled(enabled);
//					m_ui->checkBoxMonday->setPalette(pal);
				} break;
				case NANDRAD::Schedule::ST_TUESDAY:{
					m_ui->checkBoxTuesday->setChecked(true);
					m_ui->checkBoxTuesday->setEnabled(enabled);
//					m_ui->checkBoxTuesday->setPalette(pal);
				} break;
				case NANDRAD::Schedule::ST_WEDNESDAY:{
					m_ui->checkBoxWednesday->setChecked(true);
					m_ui->checkBoxWednesday->setEnabled(enabled);
//					m_ui->checkBoxWednesday->setPalette(pal);
				} break;
				case NANDRAD::Schedule::ST_THURSDAY:{
					m_ui->checkBoxThursday->setChecked(true);
					m_ui->checkBoxThursday->setEnabled(enabled);
//					m_ui->checkBoxThursday->setPalette(pal);
				} break;
				case NANDRAD::Schedule::ST_FRIDAY:{
					m_ui->checkBoxFriday->setChecked(true);
					m_ui->checkBoxFriday->setEnabled(enabled);
//					m_ui->checkBoxFriday->setPalette(pal);
				} break;
				case NANDRAD::Schedule::ST_SATURDAY:{
					m_ui->checkBoxSaturday->setChecked(true);
					m_ui->checkBoxSaturday->setEnabled(enabled);
//					m_ui->checkBoxSaturday->setPalette(pal);
				} break;
				case NANDRAD::Schedule::ST_SUNDAY:{
					m_ui->checkBoxSunday->setChecked(true);
					m_ui->checkBoxSunday->setEnabled(enabled);
//					m_ui->checkBoxSunday->setPalette(pal);
				} break;
				case NANDRAD::Schedule::ST_HOLIDAY:{
					m_ui->checkBoxHoliday->setChecked(true);
					m_ui->checkBoxHoliday->setEnabled(enabled);
//					m_ui->checkBoxHoliday->setPalette(pal);
				} break;
			}
		}
	}

	// enable signals again in check boxes
	for (QObject * w : m_ui->widgetDayTypes->children()) {
		QCheckBox * c = qobject_cast<QCheckBox *>(w);
		if (c != nullptr)
			c->blockSignals(false);
	}

	// update current daily cycle data type and chartt

	VICUS::DailyCycle *dc = &m_currentInterval->m_dailyCycles[m_currentDailyCycleIndex];
	m_ui->widgetDailyCycle->updateInput( dc , m_db, m_isEditable);

	updateDailyCycleSelectButtons();

	//update daily cycle label
	m_ui->groupBoxDailyCycle->setTitle(tr("Daily schedule %1 of %2").arg(m_currentDailyCycleIndex+1).arg(m_currentInterval->m_dailyCycles.size()));
}


void SVDBScheduleEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->tableWidgetPeriods->blockSignals(true);
		m_ui->tableWidgetPeriods->setRowCount(0);
		m_ui->tableWidgetPeriods->blockSignals(false);
		for (QObject * w : m_ui->widgetDayTypes->children()) {
			QCheckBox * c = qobject_cast<QCheckBox *>(w);
			if (c != nullptr) {
				c->blockSignals(true);
				c->setChecked(false);
				c->blockSignals(false);
			}
		}
		m_ui->widgetDailyCycle->updateInput( nullptr , m_db, m_isEditable);

		return;
	}
	m_current = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) id ]);
	// we must a valid schedule pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->radioButtonLinear->setChecked(m_current->m_useLinearInterpolation);
	///TODO Annual Schedule ...

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

	// for built-ins, disable editing/make read-only
	m_isEditable = !m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(!m_isEditable);

	m_ui->toolButtonAddPeriod->setEnabled(m_isEditable);
	m_ui->toolButtonCopyPeriod->setEnabled(m_isEditable);
	m_ui->toolButtonRemovePeriode->setEnabled(m_isEditable);


}

void SVDBScheduleEditWidget::on_lineEditName_editingFinished()
{
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		m_db->m_schedules.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
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



void SVDBScheduleEditWidget::on_tableWidgetPeriods_currentCellChanged(int currentRow, int /* currentColumn*/, int /*previousRow*/, int /*previousColumn*/)
{
	Q_ASSERT(m_current != nullptr);
	Q_ASSERT(currentRow < m_ui->tableWidgetPeriods->rowCount() );

	m_rowIdx = currentRow;
	m_ui->widgetDailyCycleAndDayTypes->setEnabled(m_rowIdx >= 0);

	m_currentInterval = &m_current->m_periods[m_rowIdx];
	m_currentDailyCycleIndex = 0;

	selectDailyCycle();
}


void SVDBScheduleEditWidget::on_toolButtonBackward_clicked() {
	Q_ASSERT(m_currentDailyCycleIndex !=0);

	// if we just left a cycle, and this has no daytypes set, remove it
	if (m_currentDailyCycleIndex == m_currentInterval->m_dailyCycles.size()-1)
	{
		// we can delete the last cycle, if no daytypes are checked and if all
		// values are zero
		if (m_currentInterval->m_dailyCycles[m_currentDailyCycleIndex].m_dayTypes.empty() &&
			m_currentInterval->m_dailyCycles[m_currentDailyCycleIndex].m_values.size() == 1 &&
				m_currentInterval->m_dailyCycles[m_currentDailyCycleIndex].m_values[0] == 0.0)
		{
			m_currentInterval->m_dailyCycles.resize(m_currentDailyCycleIndex);
		}
	}
	--m_currentDailyCycleIndex;

	selectDailyCycle();
}


void SVDBScheduleEditWidget::on_toolButtonForward_clicked() {
	// we have two cases:
	// m_currentDailyCycleIndex points to the last daily cycle -> in this case we add a new daily cycle
	// otherwise we just switch to the next daily cycle

	//create a new daily cycle
	if (m_currentDailyCycleIndex == m_currentInterval->m_dailyCycles.size()-1) {
		m_db->m_schedules.m_modified  =true;
		m_currentInterval->m_dailyCycles.push_back(VICUS::DailyCycle());
	}
	++m_currentDailyCycleIndex;
	selectDailyCycle();
}


void SVDBScheduleEditWidget::on_toolButtonDeleteCurrentDailyCycle_clicked() {
	//if only one daily cycle exist we need a day type
	Q_ASSERT(!m_currentInterval->m_dailyCycles.empty());

	// ask user for confirmation to delete daily cycle
	if (QMessageBox::question(this, QString(), tr("Delete currently selected daily cycle?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;

	// remove current daily cycle - index won't change
	m_currentInterval->m_dailyCycles.erase(m_currentInterval->m_dailyCycles.begin() + m_currentDailyCycleIndex);
	if (m_currentDailyCycleIndex == m_currentInterval->m_dailyCycles.size())
		--m_currentDailyCycleIndex;

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





void SVDBScheduleEditWidget::updateDayTypes(const NANDRAD::Schedule::ScheduledDayType &dt, bool checked) {

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
	if (checked) {
		Q_ASSERT(idx==-1);
		dc.m_dayTypes.push_back(dayIdx);
	}
	//delete day type
	else {
		Q_ASSERT(idx!=-1);
		dc.m_dayTypes.erase(dc.m_dayTypes.begin()+idx);
	}

	updateDailyCycleSelectButtons();

	m_db->m_schedules.m_modified = true;

	// if schedule interval is valid -> green background
	onValidityInfoUpdated();
}


void SVDBScheduleEditWidget::updateDailyCycleSelectButtons() {
	// enable/disable arrow buttons

	m_ui->toolButtonBackward->setEnabled(m_currentDailyCycleIndex!=0);

	// enable forward button when:
	// - the currently edited daily cycle must have at least one extra day checked
	// - we edit the last daily cycle and we can add still one more (when the full set of days has not yet been checked)
	// - we edit not the last daily cycle

	// last daily cycle?
	if (m_currentDailyCycleIndex == m_currentInterval->m_dailyCycles.size()-1) {
		bool enableButton = false;
		// check that we have at least one new day checked
		for (QObject * w : m_ui->widgetDayTypes->children()) {
			QCheckBox * c = qobject_cast<QCheckBox *>(w);
			if (c != nullptr) {
				if (c->isEnabled() && c->isChecked()) {
					enableButton = true;
					break;
				}
			}
		}
		// any days free?
		enableButton = enableButton && !m_currentInterval->freeDayTypes().empty();
		// check all check boxes and if we find one that is enabled and checked we have a modified
		m_ui->toolButtonForward->setEnabled(enableButton);
	}
	else {
		// navigation forward is always possible
		m_ui->toolButtonForward->setEnabled(true);
	}

	// remove button requires more than one daily cycle
	m_ui->toolButtonDeleteCurrentDailyCycle->setEnabled(m_currentInterval->m_dailyCycles.size() > 1);
	onValidityInfoUpdated();
}

void SVDBScheduleEditWidget::on_checkBoxMonday_toggled(bool checked) {
	updateDayTypes(NANDRAD::Schedule::ST_MONDAY, checked);
}

void SVDBScheduleEditWidget::on_checkBoxTuesday_toggled(bool checked) {
	updateDayTypes(NANDRAD::Schedule::ST_TUESDAY, checked);
}

void SVDBScheduleEditWidget::on_checkBoxHoliday_toggled(bool checked) {
	updateDayTypes(NANDRAD::Schedule::ST_HOLIDAY, checked);
}

void SVDBScheduleEditWidget::on_checkBoxWednesday_toggled(bool checked) {
	updateDayTypes(NANDRAD::Schedule::ST_WEDNESDAY, checked);
}

void SVDBScheduleEditWidget::on_checkBoxThursday_toggled(bool checked) {
	updateDayTypes(NANDRAD::Schedule::ST_THURSDAY, checked);
}

void SVDBScheduleEditWidget::on_checkBoxFriday_toggled(bool checked) {
	updateDayTypes(NANDRAD::Schedule::ST_FRIDAY, checked);
}

void SVDBScheduleEditWidget::on_checkBoxSaturday_toggled(bool checked) {
	updateDayTypes(NANDRAD::Schedule::ST_SATURDAY, checked);
}

void SVDBScheduleEditWidget::on_checkBoxSunday_toggled(bool checked) {
	updateDayTypes(NANDRAD::Schedule::ST_SUNDAY, checked);
}




void SVDBScheduleEditWidget::on_tableWidgetPeriods_cellDoubleClicked(int row, int column) {
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
	periodStartDate = QtExt::DateTimeInputDialog::requestDate(tr("Modify start date of period"),
															  tr("Enter start date (dd.MM.):"), tr("dd.MM."),
															  &periodStartDate);

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
	updatePeriodTable(idx+1);
}


void SVDBScheduleEditWidget::on_pushButton_clicked() {
	m_ui->checkBoxMonday->setChecked(true);
	m_ui->checkBoxTuesday->setChecked(true);
	m_ui->checkBoxWednesday->setChecked(true);
	m_ui->checkBoxThursday->setChecked(true);
	m_ui->checkBoxFriday->setChecked(true);
}

void SVDBScheduleEditWidget::on_pushButton_2_clicked() {
	m_ui->checkBoxSaturday->setChecked(true);
	m_ui->checkBoxSunday->setChecked(true);
}


void SVDBScheduleEditWidget::on_radioButtonLinear_toggled(bool checked)
{
	if ( m_current == nullptr )
		return;

	m_current->m_useLinearInterpolation = (checked ? true : false);
}


void SVDBScheduleEditWidget::onValidityInfoUpdated() {
	Q_ASSERT(m_current != nullptr);
	// get index of currently edited item
	int currentIdx = m_ui->tableWidgetPeriods->currentRow(); // Must be != -1
	Q_ASSERT(currentIdx != -1);
	if (m_current->m_periods[currentIdx].isValid())
		m_ui->tableWidgetPeriods->item(currentIdx,1)->setData(Qt::DecorationRole, QIcon("://gfx/actions/16x16/ok.png"));
	else
		m_ui->tableWidgetPeriods->item(currentIdx,1)->setData(Qt::DecorationRole, QIcon("://gfx/actions/16x16/error.png"));

	// since this function is called whenever the data was added, we also need to inform the model about our modification
	m_dbModel->setItemModified(m_current->m_id);
}
