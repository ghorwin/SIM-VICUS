/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVSimulationPerformanceOptions.h"
#include "ui_SVSimulationPerformanceOptions.h"

#include "SVProjectHandler.h"
#include "SVUndoModifySolverParameter.h"

#include <limits>

#include <NANDRAD_KeywordList.h>
#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_SolverParameter.h>

#include <VICUS_Project.h>


SVSimulationPerformanceOptions::SVSimulationPerformanceOptions(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationPerformanceOptions)
{
	m_ui->setupUi(this);
	layout()->setContentsMargins(0,0,0,0);

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

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVSimulationPerformanceOptions::onModified);
}


SVSimulationPerformanceOptions::~SVSimulationPerformanceOptions() {
	delete m_ui;
}


void SVSimulationPerformanceOptions::updateUi() {

	const NANDRAD::SolverParameter &solverParams = project().m_solverParameter;

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

	// set enable states

	bool haveIterative = solverParams.m_lesSolver == NANDRAD::SolverParameter::LES_GMRES ||
			solverParams.m_lesSolver == NANDRAD::SolverParameter::LES_BiCGStab ||
			solverParams.m_lesSolver == NANDRAD::SolverParameter::NUM_LES;

	bool haveLES = (solverParams.m_integrator != NANDRAD::SolverParameter::I_ExplicitEuler);
	m_ui->comboBoxLesSolver->setEnabled(haveLES);
	m_ui->lineEditMaxKry->setEnabled(haveLES);
	m_ui->lineEditIterative->setEnabled(haveLES);
	m_ui->comboBoxPreCond->setEnabled(haveLES);
	m_ui->lineEditPreILU->setEnabled(haveLES);

	if (!haveLES)
		haveIterative = false;
	m_ui->comboBoxPreCond->setEnabled(haveIterative);
	m_ui->lineEditPreILU->setEnabled(haveIterative);


	// set values

	m_ui->lineEditNonLin->setFromParameter(
		solverParams.m_para[NANDRAD::SolverParameter::P_NonlinSolverConvCoeff], IBK::Unit("---"));

	m_ui->lineEditIterative->setFromParameter(
		solverParams.m_para[NANDRAD::SolverParameter::P_IterativeSolverConvCoeff], IBK::Unit("---"));

	if (solverParams.m_intPara[NANDRAD::SolverParameter::IP_MaxOrder].empty())
		m_ui->lineEditMaxOrder->setText("");
	else
		m_ui->lineEditMaxOrder->setValue(solverParams.m_intPara[NANDRAD::SolverParameter::IP_MaxOrder].value);

	if (solverParams.m_intPara[NANDRAD::SolverParameter::IP_MaxKrylovDim].empty())
		m_ui->lineEditMaxKry->setText("");
	else
		m_ui->lineEditMaxKry->setValue(solverParams.m_intPara[NANDRAD::SolverParameter::IP_MaxKrylovDim].value);

	if (solverParams.m_para[NANDRAD::SolverParameter::P_MinTimeStep].name.empty())
		m_ui->lineEditMinDT->clear();
	else
		m_ui->lineEditMinDT->setFromParameter(
			solverParams.m_para[NANDRAD::SolverParameter::P_MinTimeStep]);

	if (solverParams.m_para[NANDRAD::SolverParameter::P_InitialTimeStep].name.empty())
		m_ui->lineEditInitialDT->clear();
	else
		m_ui->lineEditInitialDT->setFromParameter(
			solverParams.m_para[NANDRAD::SolverParameter::P_InitialTimeStep]);

	if (solverParams.m_para[NANDRAD::SolverParameter::P_RelTol].empty())
		m_ui->lineEditRelTol->clear();
	else
		m_ui->lineEditRelTol->setFromParameter(
					solverParams.m_para[NANDRAD::SolverParameter::P_RelTol], IBK::Unit("---"));

	if (solverParams.m_integrator == NANDRAD::SolverParameter::NUM_I)
		m_ui->comboBoxIntegrator->setCurrentIndex(NANDRAD::SolverParameter::I_CVODE);
	else
		m_ui->comboBoxIntegrator->setCurrentIndex( solverParams.m_integrator);

	if (solverParams.m_lesSolver == NANDRAD::SolverParameter::NUM_LES)
		m_ui->comboBoxLesSolver->setCurrentIndex(NANDRAD::SolverParameter::LES_KLU);
	else
		m_ui->comboBoxLesSolver->setCurrentIndex( solverParams.m_lesSolver);

	if (solverParams.m_preconditioner == NANDRAD::SolverParameter::NUM_PRE)
		m_ui->comboBoxPreCond->setCurrentIndex(NANDRAD::SolverParameter::PRE_ILU);
	else
		m_ui->comboBoxPreCond->setCurrentIndex( solverParams.m_preconditioner);

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
}


void SVSimulationPerformanceOptions::currentIndexChanged(int /*index*/) {

	NANDRAD::SolverParameter solverParams = project().m_solverParameter;

	solverParams.m_integrator = (NANDRAD::SolverParameter::integrator_t)m_ui->comboBoxIntegrator->currentIndex();
	solverParams.m_lesSolver = (NANDRAD::SolverParameter::lesSolver_t)m_ui->comboBoxLesSolver->currentIndex();
	solverParams.m_preconditioner = (NANDRAD::SolverParameter::precond_t)m_ui->comboBoxPreCond->currentIndex();

	SVUndoModifySolverParameter *undo = new SVUndoModifySolverParameter("modified solver parameter", solverParams, project().m_simulationParameter);
	undo->push();
}


void SVSimulationPerformanceOptions::onModified(int modificationType, ModificationInfo *) {
	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::SolverParametersModified:
			updateUi();
		break;
		default:;
	}
}


void SVSimulationPerformanceOptions::parameterEditingFinished(int paraEnum, const QtExt::ValidatingLineEdit *edit) {

	NANDRAD::SolverParameter solverParams = project().m_solverParameter;

	if (edit->isValid() && edit->text().trimmed().isEmpty()) {
		// clear parameter
		if (solverParams.m_para[paraEnum].name.empty())
			return; // parameter not changed, no undo action needed
		// clear parameter
		solverParams.m_para[paraEnum] = IBK::Parameter();
	}
	else {
		double val = edit->value();
		// update parameter value
		NANDRAD::KeywordList::setParameter(solverParams.m_para, "SolverParameter::para_t", paraEnum, val);
	}

	SVUndoModifySolverParameter *undo = new SVUndoModifySolverParameter("modified solver parameter", solverParams, project().m_simulationParameter);
	undo->push();
}


void SVSimulationPerformanceOptions::intParameterEditingFinished(int paraEnum, const QtExt::ValidatingLineEdit *edit) {

	NANDRAD::SolverParameter solverParams = project().m_solverParameter;

	if (edit->isValid() && edit->text().trimmed().isEmpty()) {
		// clear parameter
		if (solverParams.m_intPara[paraEnum].name.empty())
			return; // parameter not changed, no undo action needed
		// clear parameter
		solverParams.m_intPara[paraEnum] = IBK::IntPara();
	}
	else {
		int val = (int)edit->value();
		// update parameter value
		solverParams.m_intPara[paraEnum] = IBK::IntPara(
												NANDRAD::KeywordList::Keyword("SolverParameter::intPara_t", paraEnum),
												val);
	}

	SVUndoModifySolverParameter *undo = new SVUndoModifySolverParameter("modified solver parameter", solverParams, project().m_simulationParameter);
	undo->push();
}


void SVSimulationPerformanceOptions::on_lineEditMaxOrder_editingFinishedSuccessfully() {
	intParameterEditingFinished(NANDRAD::SolverParameter::IP_MaxOrder, m_ui->lineEditMaxOrder);
}


void SVSimulationPerformanceOptions::on_lineEditNonLin_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::P_NonlinSolverConvCoeff, m_ui->lineEditNonLin);
}


void SVSimulationPerformanceOptions::on_lineEditMaxKry_editingFinishedSuccessfully() {
	intParameterEditingFinished(NANDRAD::SolverParameter::IP_MaxKrylovDim, m_ui->lineEditMaxKry);
}


void SVSimulationPerformanceOptions::on_lineEditIterative_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::P_IterativeSolverConvCoeff, m_ui->lineEditIterative);
}


void SVSimulationPerformanceOptions::on_lineEditPreILU_editingFinishedSuccessfully() {
	intParameterEditingFinished(NANDRAD::SolverParameter::IP_PreILUWidth, m_ui->lineEditPreILU);
}


void SVSimulationPerformanceOptions::on_lineEditInitialDT_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::P_InitialTimeStep, m_ui->lineEditInitialDT);
}


void SVSimulationPerformanceOptions::on_lineEditMinDT_editingFinishedSuccessfully() {
	parameterEditingFinished(NANDRAD::SolverParameter::P_MinTimeStep, m_ui->lineEditMinDT);
}

void SVSimulationPerformanceOptions::on_lineEditRelTol_editingFinished() {
	parameterEditingFinished(NANDRAD::SolverParameter::P_RelTol, m_ui->lineEditRelTol);
}
