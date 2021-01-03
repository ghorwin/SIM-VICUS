#include "SVSimulationPerformanceOptions.h"
#include "ui_SVSimulationPerformanceOptions.h"

#include <limits>

#include <NANDRAD_KeywordList.h>
#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_SolverParameter.h>


SVSimulationPerformanceOptions::SVSimulationPerformanceOptions(QWidget *parent, NANDRAD::SolverParameter & solverParams) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationPerformanceOptions),
	m_solverParams(&solverParams)
{
	m_ui->setupUi(this);

	m_ui->gridLayout->setMargin(0);

	for (int i=0; i<NANDRAD::SolverParameter::NUM_I; ++i )
		m_ui->comboBoxIntegrator->addItem( QString("%1 [%2]")
										   .arg(NANDRAD::KeywordListQt::Description( "SolverParameter::integrator_t", i ))
										   .arg(QString::fromStdString( NANDRAD::KeywordList::Keyword( "SolverParameter::integrator_t", i ))  ), i );

	for (int i=0; i<NANDRAD::SolverParameter::NUM_LES; ++i )
		m_ui->comboBoxLesSolver->addItem(  QString("%1 [%2]")
										   .arg(NANDRAD::KeywordListQt::Description( "SolverParameter::lesSolver_t", i ))
										   .arg(QString::fromStdString( NANDRAD::KeywordList::Keyword( "SolverParameter::lesSolver_t", i )) ), i );

	for (int i=0; i<NANDRAD::SolverParameter::NUM_PRE; ++i )
		m_ui->comboBoxPreCond->addItem(   QString("%1 [%2]")
										  .arg(NANDRAD::KeywordListQt::Description( "SolverParameter::precond_t", i ))
										  .arg(QString::fromStdString( NANDRAD::KeywordList::Keyword( "SolverParameter::precond_t", i )) ), i );

	m_ui->lineEditMaxOrder->setup(1,5,NANDRAD::KeywordListQt::Description("SolverParameter::para_t",NANDRAD::SolverParameter::IP_MaxOrder), true, true);
	m_ui->lineEditMaxOrder->setAcceptOnlyInteger(true);
	m_ui->lineEditMaxOrder->setEmptyAllowed(true, tr("auto","as in automatic"));
	m_ui->lineEditNonLin->setup(1e-10,1, NANDRAD::KeywordListQt::Description("SolverParameter::para_t",NANDRAD::SolverParameter::P_NonlinSolverConvCoeff) );
	m_ui->lineEditNonLin->setEmptyAllowed(true, tr("auto","as in automatic"));
	m_ui->lineEditIterative->setup(1e-10, 1, NANDRAD::KeywordListQt::Description("SolverParameter::para_t",NANDRAD::SolverParameter::P_IterativeSolverConvCoeff) );
	m_ui->lineEditIterative->setEmptyAllowed(true, tr("auto","as in automatic"));
	m_ui->lineEditMaxKry->setup(1, 100000, NANDRAD::KeywordListQt::Description("SolverParameter::para_t",NANDRAD::SolverParameter::IP_MaxKrylovDim), true);
	m_ui->lineEditMaxKry->setEmptyAllowed(true, tr("auto","as in automatic"));
	m_ui->lineEditMaxKry->setAcceptOnlyInteger(true);
	m_ui->lineEditPreILU->setup(1, 100000, NANDRAD::KeywordListQt::Description("SolverParameter::para_t",NANDRAD::SolverParameter::IP_PreILUWidth), true);
	m_ui->lineEditPreILU->setEmptyAllowed(true, tr("auto","as in automatic"));
	m_ui->lineEditPreILU->setAcceptOnlyInteger(true);

	m_ui->lineEditInitialDT->setup(m_ui->comboBoxInitialDT, IBK::Unit("s"),
								   0, std::numeric_limits<double>::max(),
								   NANDRAD::KeywordListQt::Description("SolverParameter::para_t",NANDRAD::SolverParameter::P_InitialTimeStep) );
	m_ui->lineEditMinDT->setup(m_ui->comboBoxMinDT, IBK::Unit("s"),
							   0, std::numeric_limits<double>::max(), NANDRAD::KeywordListQt::Description("SolverParameter::para_t",NANDRAD::SolverParameter::P_MinTimeStep) );

	connect(m_ui->comboBoxIntegrator, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
	connect(m_ui->comboBoxLesSolver, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
	connect(m_ui->comboBoxPreCond, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
}


SVSimulationPerformanceOptions::~SVSimulationPerformanceOptions() {
	delete m_ui;
}


void SVSimulationPerformanceOptions::updateUi() {

	m_ui->comboBoxIntegrator->blockSignals(true);
	m_ui->comboBoxLesSolver->blockSignals(true);
	m_ui->comboBoxPreCond->blockSignals(true);

	m_ui->lineEditNonLin->blockSignals(true);
	m_ui->lineEditIterative->blockSignals(true);
	m_ui->lineEditMaxOrder->blockSignals(true);
	m_ui->lineEditMaxKry->blockSignals(true);
	m_ui->lineEditPreILU->blockSignals(true);
	m_ui->lineEditMinDT->blockSignals(true);
	m_ui->lineEditInitialDT->blockSignals(true);

	// generate defaults
	NANDRAD::SolverParameter s;
	s.initDefaults();

	m_ui->lineEditNonLin->setFromParameter(
		m_solverParams->m_para[NANDRAD::SolverParameter::P_NonlinSolverConvCoeff], IBK::Unit("---"));
	m_ui->lineEditIterative->setFromParameter(
		m_solverParams->m_para[NANDRAD::SolverParameter::P_IterativeSolverConvCoeff], IBK::Unit("---"));
	m_ui->lineEditMaxOrder->setFromParameter(
		m_solverParams->m_para[NANDRAD::SolverParameter::IP_MaxOrder], IBK::Unit("---"));
	m_ui->lineEditMaxKry->setFromParameter(
		m_solverParams->m_para[NANDRAD::SolverParameter::IP_MaxKrylovDim], IBK::Unit("---"));
	if (m_solverParams->m_para[NANDRAD::SolverParameter::P_MinTimeStep].name.empty())
		m_ui->lineEditMinDT->setFromParameter( s.m_para[NANDRAD::SolverParameter::P_MinTimeStep]);
	else
		m_ui->lineEditMinDT->setFromParameter(
			m_solverParams->m_para[NANDRAD::SolverParameter::P_MinTimeStep]);
	if (m_solverParams->m_para[NANDRAD::SolverParameter::P_InitialTimeStep].name.empty())
		m_ui->lineEditInitialDT->setFromParameter(
			s.m_para[NANDRAD::SolverParameter::P_InitialTimeStep]);
	else
		m_ui->lineEditInitialDT->setFromParameter(
			m_solverParams->m_para[NANDRAD::SolverParameter::P_InitialTimeStep]);

	m_ui->comboBoxIntegrator->setCurrentIndex( m_solverParams->m_integrator);
	m_ui->comboBoxLesSolver->setCurrentIndex( m_solverParams->m_lesSolver);
	m_ui->comboBoxPreCond->setCurrentIndex( m_solverParams->m_preconditioner);

	m_ui->comboBoxIntegrator->blockSignals(false);
	m_ui->comboBoxLesSolver->blockSignals(false);
	m_ui->comboBoxPreCond->blockSignals(false);

	m_ui->lineEditNonLin->blockSignals(false);
	m_ui->lineEditIterative->blockSignals(false);
	m_ui->lineEditMaxOrder->blockSignals(false);
	m_ui->lineEditMaxKry->blockSignals(false);
	m_ui->lineEditPreILU->blockSignals(false);
	m_ui->lineEditMinDT->blockSignals(false);
	m_ui->lineEditInitialDT->blockSignals(false);

	// special handling for CVODE integrator
	m_ui->checkBoxDisableKinsolLineSearch->setEnabled(m_solverParams->m_integrator == NANDRAD::SolverParameter::I_CVODE);
	m_ui->checkBoxDisableKinsolLineSearch->blockSignals(true);
	m_ui->checkBoxDisableKinsolLineSearch->setChecked( m_solverParams->m_flag[NANDRAD::SolverParameter::F_KinsolDisableLineSearch].isEnabled());
	m_ui->checkBoxDisableKinsolLineSearch->blockSignals(false);

	// update enabled-state of widgets
	currentIndexChanged(0);
}


void SVSimulationPerformanceOptions::currentIndexChanged(int /*index*/) {

	m_solverParams->m_integrator = (NANDRAD::SolverParameter::integrator_t)m_ui->comboBoxIntegrator->currentIndex();
	m_solverParams->m_lesSolver = (NANDRAD::SolverParameter::lesSolver_t)m_ui->comboBoxLesSolver->currentIndex();
	m_solverParams->m_preconditioner = (NANDRAD::SolverParameter::precond_t)m_ui->comboBoxPreCond->currentIndex();

	// special handling for CVODE integrator
	m_ui->checkBoxDisableKinsolLineSearch->setEnabled(m_solverParams->m_integrator == NANDRAD::SolverParameter::I_CVODE);

	bool haveIterative = m_solverParams->m_lesSolver == NANDRAD::SolverParameter::LES_GMRES ||
			m_solverParams->m_lesSolver == NANDRAD::SolverParameter::LES_BiCGStab ||
			m_solverParams->m_lesSolver == NANDRAD::SolverParameter::NUM_LES;

	m_ui->lineEditIterative->setEnabled(haveIterative);
	m_ui->lineEditMaxKry->setEnabled(haveIterative);
	m_ui->lineEditPreILU->setEnabled(haveIterative);
	m_ui->comboBoxPreCond->setEnabled(haveIterative);
}


void SVSimulationPerformanceOptions::on_lineEditMaxOrder_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::IP_MaxOrder, m_ui->lineEditMaxOrder);
}


void SVSimulationPerformanceOptions::on_lineEditNonLin_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::P_NonlinSolverConvCoeff, m_ui->lineEditNonLin);
}


void SVSimulationPerformanceOptions::on_lineEditMaxKry_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::IP_MaxKrylovDim, m_ui->lineEditMaxKry);
}


void SVSimulationPerformanceOptions::on_lineEditIterative_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::P_IterativeSolverConvCoeff, m_ui->lineEditIterative);
}


void SVSimulationPerformanceOptions::on_lineEditPreILU_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::IP_PreILUWidth, m_ui->lineEditPreILU);
}


void SVSimulationPerformanceOptions::on_lineEditInitialDT_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::P_InitialTimeStep, m_ui->lineEditInitialDT);
}


void SVSimulationPerformanceOptions::on_lineEditMinDT_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::P_MinTimeStep, m_ui->lineEditMinDT);
}


void SVSimulationPerformanceOptions::on_checkBoxDisableKinsolLineSearch_stateChanged(int arg1) {
	m_solverParams->m_flag[NANDRAD::SolverParameter::F_KinsolDisableLineSearch].set(
				NANDRAD::KeywordList::Keyword("SolverParameter::flag_t", NANDRAD::SolverParameter::F_KinsolDisableLineSearch), true);
}


void SVSimulationPerformanceOptions::parameterEditingFinished(int paraEnum, const QtExt::ValidatingLineEdit *edit) {
	// create copy of solver parameter object

	if (edit->isValid() && edit->text().trimmed().isEmpty()) {
		// clear parameter
		if (m_solverParams->m_para[paraEnum].name.empty())
			return; // parameter not changed, no undo action needed
		// clear parameter
		m_solverParams->m_para[paraEnum] = IBK::Parameter();
	}
	else {
		double val = edit->value();
		// update parameter value
		NANDRAD::KeywordList::setParameter(m_solverParams->m_para, "SolverParameter::para_t", paraEnum, val);
	}
}
