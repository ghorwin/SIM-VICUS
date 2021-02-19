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

/*
SVDBScheduleEditWidget::SVDBScheduleEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBScheduleEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	m_ui->lineEditSolarAbsorptionCoefficient->setup(0, 1, tr("Solar Absorption (short wave)"), true, true);
	m_ui->lineEditLongWaveEmissivity->setup(0, 1, tr("Thermal Absorption (long wave)"), true, true);
	m_ui->lineEditHeatTransferCoefficient->setup(0.001, 500, tr("Thermal conductivity"), true, true);

	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(true);
	for (unsigned int i=0; i <= NANDRAD::InterfaceHeatConduction::NUM_MT; ++i)
		m_ui->comboBoxHeatTransferCoeffModelType->addItem(QString("%1 [%2]")
			.arg(NANDRAD::KeywordListQt::Description("InterfaceHeatConduction::modelType_t", (int)i))
			.arg(NANDRAD::KeywordListQt::Keyword("InterfaceHeatConduction::modelType_t", (int)i)), i);
	m_ui->comboBoxHeatTransferCoeffModelType->blockSignals(false);

	m_ui->comboBoxLWModelType->blockSignals(true);
	for(unsigned int i=0; i <= NANDRAD::InterfaceLongWaveEmission::NUM_MT; ++i)
		m_ui->comboBoxLWModelType->addItem(QString("%1 [%2]")
			.arg(NANDRAD::KeywordListQt::Description("InterfaceLongWaveEmission::modelType_t", (int)i))
			.arg(NANDRAD::KeywordListQt::Keyword("InterfaceLongWaveEmission::modelType_t", (int)i)), i);
	m_ui->comboBoxLWModelType->blockSignals(false);

	m_ui->comboBoxSWModelType->blockSignals(true);
	for(unsigned int i=0; i <= NANDRAD::InterfaceSolarAbsorption::NUM_MT; ++i)
		m_ui->comboBoxSWModelType->addItem(QString("%1 [%2]")
			.arg(NANDRAD::KeywordListQt::Description("InterfaceSolarAbsorption::modelType_t", (int)i))
			.arg(NANDRAD::KeywordListQt::Keyword("InterfaceSolarAbsorption::modelType_t", (int)i)), i);
	m_ui->comboBoxSWModelType->blockSignals(false);


	// initial state is "nothing selected"
	updateInput(-1);
}*/


SVDBScheduleEditWidget::SVDBScheduleEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBScheduleEditWidget)
{
	m_ui->setupUi(this);

	//add header to periods table
	m_ui->tableWidgetPeriods->setColumnCount(2);
	m_ui->tableWidgetPeriods->setHorizontalHeaderItem(0, new QTableWidgetItem("Date"));
	m_ui->tableWidgetPeriods->setHorizontalHeaderItem(1, new QTableWidgetItem("Name"));

	//add header to day cycle table
	m_ui->tableWidgetDayCycle->setColumnCount(2);
	m_ui->tableWidgetDayCycle->setHorizontalHeaderItem(0, new QTableWidgetItem("Time"));
	m_ui->tableWidgetDayCycle->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));

	//set all table items in day cycle
	m_ui->tableWidgetDayCycle->setRowCount(24);
	for(unsigned int i=0; i<24; ++i){
		QString time = i < 9 ? "0" + QString::number(i) : QString::number(i);
		time += ":00 - ";
		time += i+1 < 9 ? "0" + QString::number(i+1) : QString::number(i+1);
		time += ":00";

		m_ui->tableWidgetDayCycle->setItem(i,0, new QTableWidgetItem(time));
		m_ui->tableWidgetDayCycle->setItem(i,1, new QTableWidgetItem("---"));
	}

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



void SVDBScheduleEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = (id == -1 ? false : true);

	//set buttons
	m_ui->toolButtonForward->setEnabled(false);
	m_ui->toolButtonBackward->setEnabled(false);
	m_ui->toolButtonAddPeriod->setEnabled(isEnabled);
	m_ui->toolButtonCopyPeriod->setEnabled(isEnabled);
	m_ui->toolButtonRemovePeriode->setEnabled(isEnabled);
	m_ui->horizontalLayout_3->setEnabled(isEnabled);

	//set checkboxes
	m_ui->checkBoxMonday->setEnabled(false);
	m_ui->checkBoxTuesday->setEnabled(false);
	m_ui->checkBoxWednesday->setEnabled(false);
	m_ui->checkBoxThursday->setEnabled(false);
	m_ui->checkBoxFriday->setEnabled(false);
	m_ui->checkBoxSaturday->setEnabled(false);
	m_ui->checkBoxSunday->setEnabled(false);
	m_ui->checkBoxHoliday->setEnabled(false);

	//set table views
	m_ui->tableWidgetDayCycle->setEnabled(false);
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
		if(m_current->m_periods.empty()){
			m_current->m_periods.push_back(VICUS::ScheduleInterval());
			//set start day to first day of year (0)
			m_current->m_periods.front().m_intervalStartDay = 0;
		}
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

	// convert date to dayofyear
	unsigned int input = startDate.dayOfYear();
	// check if such a period starting day has already been used, and if yes,
	for(const VICUS::ScheduleInterval &schedInt : m_current->m_periods){
		if(schedInt.m_intervalStartDay == 0) {
			//QMessageBox("Error", "A Period with this start day already exists.");
		}
	}
	// show error message


	// now create a new ScheduleInverval and insert into vector at appropriate position (sorted) and
	// get resulting index of new ScheduleInverval in vector

	// update table widget

	// select ScheduleInverval table row by ScheduleInverval index -> this will show the editor for the newly created schedule
}


