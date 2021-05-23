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

#include "SVDBWindowGlazingSystemEditWidget.h"
#include "ui_SVDBWindowGlazingSystemEditWidget.h"

#include <QSortFilterProxyModel>

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>

#include <IBK_physics.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Conversions.h>

#include <qwt_plot.h>
#include <qwt_math.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_text.h>
#include <qwt_counter.h>

#include "SVSettings.h"
#include "SVDBWindowGlazingSystemTableModel.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVStyle.h"

SVDBWindowGlazingSystemEditWidget::SVDBWindowGlazingSystemEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBWindowGlazingSystemEditWidget)
{
	m_ui->setupUi(this);
	m_ui->masterLayout->setMargin(4);

	// style the table widget

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Window Glazing System"));

	//header elements

	// set period table column sizes

	//add header to periods table
	m_ui->tableWidgetSHGC->setColumnCount(2);
	m_ui->tableWidgetSHGC->setRowCount(10);
	// Note: valid column is self-explanatory and does not need a caption
	m_ui->tableWidgetSHGC->setHorizontalHeaderLabels(QStringList() << tr("Angle") << tr("SHGC"));

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSHGC);

	m_ui->tableWidgetSHGC->setSortingEnabled(false);

	m_ui->tableWidgetSHGC->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	QFontMetrics fm(m_ui->tableWidgetSHGC->horizontalHeader()->font());

	int width = 100;
	m_ui->tableWidgetSHGC->setColumnWidth(0, width);

	m_ui->tableWidgetSHGC->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	for (int i = 10; i > 0; --i) {
		// we need the Angle to go from 0 ... 90 Deg
		// so we then make the iterator go from 0 ... 9
		int idx = i-1;

		m_ui->tableWidgetSHGC->setItem( i-1, 0, new QTableWidgetItem( QString::number(i*10) ) );
		m_ui->tableWidgetSHGC->setItem( i-1, 1, new QTableWidgetItem(""));

		m_ui->tableWidgetSHGC->item( i-1, 0)->setFlags(m_ui->tableWidgetSHGC->item(i-1,0)->flags() & ~Qt::ItemIsEditable);
		m_ui->tableWidgetSHGC->item( i-1, 0)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetSHGC->item( i-1, 1)->setTextAlignment(Qt::AlignCenter);
	}



//	unsigned int i=9;
//	while (true){
//		m_ui->tableWidgetSHGC->setItem((int)i, 0, new QTableWidgetItem(QString::number((i)*10)));
//		m_ui->tableWidgetSHGC->setItem((int)i, 1, new QTableWidgetItem(""));

//		if(i == 0)
//			break;
//		--i;
//	}

	m_ui->comboBoxType->blockSignals(true);
	for (int i=0; i<VICUS::WindowGlazingSystem::NUM_MT; ++i)
		m_ui->comboBoxType->addItem(VICUS::KeywordListQt::Keyword("WindowGlazingSystem::modelType_t", i), i);
	m_ui->comboBoxType->blockSignals(false);

	// QWT Plot is initialized
	QwtPlot * plt = m_ui->shgcPlot;
	plt->setAutoReplot(false);
	plt->setContentsMargins(5, 5, 5, 5);
	//plt->setMargin(5);
	plt->setAxisScale(QwtPlot::xBottom, 0, 90);
	plt->setAxisScale(QwtPlot::yLeft, 0, 1);


	// axes
	QwtText theAxisTitle(tr("Incident Angle [Deg]")); // no axis title for left diagram
	QFont f(theAxisTitle.font());
	f.setPointSize(9);
	theAxisTitle.setFont(f);
	plt->setAxisTitle(QwtPlot::xBottom, theAxisTitle);
	theAxisTitle.setText(tr("SHGC [-]"), QwtText::RichText);
	plt->setAxisTitle(QwtPlot::yLeft, theAxisTitle);

#if defined(Q_OS_MAC)
	// tick font
	f.setPointSize(10);
	QColor backgroundColor(240,240,240);
#else
	// tick font
	f.setPointSize(8);
	QColor backgroundColor(240,240,240);
#endif
	plt->setAxisFont(QwtPlot::xBottom, f);
	plt->setAxisFont(QwtPlot::yLeft, f);

	// background color
	plt->setCanvasBackground(backgroundColor);

	// grid
	QwtPlotGrid *grid = new QwtPlotGrid;
	grid->enableXMin(true);
	grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
	grid->setMinorPen(QPen(Qt::lightGray, 0 , Qt::DotLine));
	grid->attach(plt);

	// set up shgc curve
	m_shgcCurve = new QwtPlotCurve("");
	m_shgcCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
	m_shgcCurve->setPen(QPen(Qt::black));
	m_shgcCurve->setYAxis(QwtPlot::yLeft);
	m_shgcCurve->setZ(1);
	m_shgcCurve->setVisible(false);
	m_shgcCurve->attach(plt);

	std::vector<double> degVec;
	std::vector<double> plotSHGCVec;

	degVec.push_back(0);
	degVec.push_back(90);

	plotSHGCVec.push_back(60);
	plotSHGCVec.push_back(60);

	m_shgcCurve->setRawSamples(&degVec[0], &plotSHGCVec[0], (int)degVec.size() );

	m_ui->shgcPlot->replot();

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBWindowGlazingSystemEditWidget::~SVDBWindowGlazingSystemEditWidget() {
	delete m_ui;
}


void SVDBWindowGlazingSystemEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBWindowGlazingSystemTableModel*>(dbModel);
}


void SVDBWindowGlazingSystemEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers


	if (id == -1) {
		// disable all controls
		setEnabled(false);

		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());

		// property info fields
		m_ui->lineEditUValue->setText("");
		m_ui->lineEditSHGC->setText("");
		m_ui->comboBoxType->blockSignals(true);
		m_ui->comboBoxType->setCurrentIndex(VICUS::WindowGlazingSystem::NUM_MT);
		m_ui->comboBoxType->blockSignals(false);

		m_ui->pushButtonWindowColor->setColor(Qt::black);

		return;
	}
	// re-enable all controls
	setEnabled(true);
	m_current = const_cast<VICUS::WindowGlazingSystem *>(m_db->m_windowGlazingSystems[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	// now update the GUI controls

	// for built-ins, disable editing/make read-only
	bool isEditable = !m_current->m_builtIn;

	m_ui->lineEditUValue->setValue(m_current->m_para[VICUS::WindowGlazingSystem::P_ThermalTransmittance].get_value());

	m_ui->comboBoxType->blockSignals(true);
	if(m_current->m_modelType != VICUS::WindowGlazingSystem::NUM_MT){
		m_current->m_modelType = VICUS::WindowGlazingSystem::MT_Simple;
		modelModify();
		m_dbModel->setItemModified(m_current->m_id);
	}
	m_ui->comboBoxType->setCurrentIndex(m_current->m_modelType);
	m_ui->comboBoxType->blockSignals(false);


	if(m_current->m_modelType == VICUS::WindowGlazingSystem::MT_Simple){
		std::vector<double> degVec;
		std::vector<double> plotSHGCVec;
		degVec.resize(10);
		plotSHGCVec.resize(10);

		const IBK::LinearSpline &spline=m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values;

		if(!spline.empty()){
			for(unsigned int i=0; i<10; ++i){

				IBK::Unit unit = m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_xUnit;
				double val = spline.value(i * 10 / (unit == IBK::Unit("Deg") ? 1 : IBK::DEG2RAD));
				m_ui->tableWidgetSHGC->item(i,1)->setText(QString::number(val));
				// first we compose the vectors with data for the plot
				degVec[i] = i * 10;
				plotSHGCVec[i] = val*100;

			}
			// we update the plot
			m_shgcCurve->setSamples(&degVec[0], &plotSHGCVec[0], (int)degVec.size() );
			m_ui->shgcPlot->replot();
			m_ui->shgcPlot->repaint();
		}
	}
	else if(m_current->m_modelType == VICUS::WindowGlazingSystem::MT_Detailed){
		///TODO Stephan implement detailed model
	}


	m_ui->pushButtonWindowColor->blockSignals(true);
	m_ui->pushButtonWindowColor->setColor(m_current->m_color);
	m_ui->pushButtonWindowColor->blockSignals(false);


	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonWindowColor->setReadOnly(!isEditable);
	m_ui->lineEditSHGC->setReadOnly(!isEditable);
	m_ui->lineEditUValue->setReadOnly(!isEditable);
	m_ui->comboBoxType->setEnabled(isEditable);
	m_ui->toolButtonCreateSpline->setEnabled(false);	///TODO Dirk implement a function for SHGC

}

void SVDBWindowGlazingSystemEditWidget::on_lineEditName_editingFinished(){
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}

void SVDBWindowGlazingSystemEditWidget::modelModify() {
	m_db->m_windowGlazingSystems.m_modified = true;
}

void SVDBWindowGlazingSystemEditWidget::on_pushButtonWindowColor_colorChanged() {

	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonWindowColor->color()) {
		m_current->m_color = m_ui->pushButtonWindowColor->color();
		modelModify(); // tell model that we changed the data
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}

}


void SVDBWindowGlazingSystemEditWidget::on_lineEditSHGC_editingFinished(){
	Q_ASSERT(m_current != nullptr);
	//do nothing
	// only for button create SHGC ....
}

void SVDBWindowGlazingSystemEditWidget::on_lineEditUValue_editingFinished(){
	Q_ASSERT(m_current != nullptr);
	if(m_ui->lineEditUValue->isValid()){
		VICUS::KeywordList::setParameter(m_current->m_para, "WindowGlazingSystem::para_t", VICUS::WindowGlazingSystem::P_ThermalTransmittance, m_ui->lineEditUValue->value());
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
	}
}

void SVDBWindowGlazingSystemEditWidget::on_comboBoxType_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	// update database but only if different from original
	if (index != (int)m_current->m_modelType)
	{
		m_current->m_modelType = static_cast<VICUS::WindowGlazingSystem::modelType_t>(index);
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
	}
}
