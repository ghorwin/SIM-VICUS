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
#include <QClipboard>

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>

#include <IBK_physics.h>
#include <IBK_UnitVector.h>

#include <QtExt_LanguageHandler.h>
#include <SV_Conversions.h>

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

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditSHGC0->setup(0, 1, tr("SHGC for vertical incidence angle must be between 0...1."), false, true);
	m_ui->lineEditUValue->setup(0, 100, tr("U-value of glazing system must be > 0."), false, true);

	m_ui->tableWidgetSHGC->blockSignals(true);
	m_ui->tableWidgetSHGC->setColumnCount(2);
	m_ui->tableWidgetSHGC->setRowCount(1);
	m_ui->tableWidgetSHGC->blockSignals(false);
	// Note: valid column is self-explanatory and does not need a caption
	m_ui->tableWidgetSHGC->setHorizontalHeaderLabels(QStringList() << tr("Angle [Deg]") << tr("SHGC [---]"));

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSHGC);

	m_ui->tableWidgetSHGC->setSortingEnabled(false);

	m_ui->tableWidgetSHGC->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	QFontMetrics fm(m_ui->tableWidgetSHGC->horizontalHeader()->font());

	int width = 100;
	m_ui->tableWidgetSHGC->setColumnWidth(0, width);
	m_ui->tableWidgetSHGC->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

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
	QwtText theAxisTitle(tr("Incidence Angle [Deg]")); // no axis title for left diagram
	QFont f(theAxisTitle.font());
	f.setPointSize(9);
	theAxisTitle.setFont(f);
	plt->setAxisTitle(QwtPlot::xBottom, theAxisTitle);
	theAxisTitle.setText(tr("SHGC [---]"), QwtText::RichText);
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
	if (SVSettings::instance().m_theme == SVSettings::TT_Dark) {
		grid->setMajorPen(QPen(Qt::lightGray, 0, Qt::DashLine));
		grid->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
	}
	else {
		grid->setMajorPen(QPen(Qt::gray, 0, Qt::DashLine));
		grid->setMinorPen(QPen(Qt::lightGray, 0 , Qt::DotLine));
	}
	grid->attach(plt);

	// set up shgc curve
	m_shgcCurve = new QwtPlotCurve("bla");
	m_shgcCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
	if (SVSettings::instance().m_theme == SVSettings::TT_Dark)
		m_shgcCurve->setPen( QColor(255,135,0) );
	else
		m_shgcCurve->setPen( QColor(110,58,0) );
	m_shgcCurve->setYAxis(QwtPlot::yLeft);
//	m_shgcCurve->setZ(1);
	m_shgcCurve->setVisible(true);
	m_shgcCurve->attach(plt);

	plt->setAutoReplot(true);
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
		m_ui->lineEditSHGC0->setText("");
		m_ui->comboBoxType->blockSignals(true);
		m_ui->comboBoxType->setCurrentIndex(VICUS::WindowGlazingSystem::NUM_MT);
		m_ui->comboBoxType->blockSignals(false);

		m_ui->pushButtonColor->setColor(Qt::black);
		m_ui->shgcPlot->setVisible(false);

		m_ui->toolButtonCreateSpline->setEnabled(false);
		m_ui->toolButtonImportSplineFromClipboard->setEnabled(false);

		return;
	}
	// re-enable all controls
	setEnabled(true);
	m_ui->shgcPlot->setVisible(true);
	m_current = const_cast<VICUS::WindowGlazingSystem *>(m_db->m_windowGlazingSystems[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	// now update the GUI controls

	// for built-ins, disable editing/make read-only
	bool isEditable = !m_current->m_builtIn;

	m_ui->lineEditName->setString(m_current->m_displayName);

	m_ui->comboBoxType->blockSignals(true);
	if (m_current->m_modelType == VICUS::WindowGlazingSystem::NUM_MT) {
		m_current->m_modelType = VICUS::WindowGlazingSystem::MT_Simple;
		modelModify();
	}
	m_ui->comboBoxType->setCurrentIndex(m_current->m_modelType);
	m_ui->comboBoxType->blockSignals(false);

	// parameters may not be given or invalid, we transfer it anyway
	m_ui->lineEditUValue->setValue(m_current->m_para[VICUS::WindowGlazingSystem::P_ThermalTransmittance].value);

	// create default SHGC-spline, if not existent or invalid
	if (m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_name.empty() ||
		m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values.size() < 2 ||
		!m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values.valid() ||
		m_current->SHGC() <= 0)
	{
		createDefaultSHGCSpline();
	}
	// we now have a guaranteed valid spline
	m_ui->lineEditSHGC0->setValue(m_current->SHGC());

	m_ui->tableWidgetSHGC->blockSignals(true);
	if (m_current->m_modelType == VICUS::WindowGlazingSystem::MT_Simple) {

		const IBK::LinearSpline &spline=m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values;
		const std::vector<double> & degVec = spline.x();
		const std::vector<double> & plotSHGCVec = spline.y();

		m_ui->tableWidgetSHGC->setRowCount(degVec.size());

		for (int i = 0; i < m_ui->tableWidgetSHGC->rowCount(); ++i) {
			QTableWidgetItem * item = new QTableWidgetItem( QString("%L1").arg(degVec[i], 0, 'g') );
			item->setFlags(item->flags() & ~Qt::ItemIsEditable);

			m_ui->tableWidgetSHGC->setItem( i, 0, item);
			item = new QTableWidgetItem( QString("%L1").arg(plotSHGCVec[i], 0, 'g') );
			item->setFlags(item->flags() & ~Qt::ItemIsEditable);
			m_ui->tableWidgetSHGC->setItem( i, 1, item);

			m_ui->tableWidgetSHGC->item( i, 0)->setTextAlignment(Qt::AlignCenter);
			m_ui->tableWidgetSHGC->item( i, 1)->setTextAlignment(Qt::AlignCenter);
		}

		// we update the plot
		m_shgcCurve->setSamples(degVec.data(), plotSHGCVec.data(), (int)degVec.size() );
		m_ui->shgcPlot->setVisible(true);
	}
	m_ui->tableWidgetSHGC->blockSignals(false);

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
	m_ui->lineEditSHGC0->setReadOnly(!isEditable);
	m_ui->lineEditUValue->setReadOnly(!isEditable);
	m_ui->comboBoxType->setEnabled(isEditable);
	m_ui->toolButtonCreateSpline->setEnabled(isEditable);
	m_ui->toolButtonImportSplineFromClipboard->setEnabled(isEditable);
}


void SVDBWindowGlazingSystemEditWidget::on_lineEditName_editingFinished(){
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBWindowGlazingSystemEditWidget::modelModify() {
	m_db->m_windowGlazingSystems.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}


void SVDBWindowGlazingSystemEditWidget::createDefaultSHGCSpline() {
	IBK_ASSERT(m_current != nullptr);
	std::vector<double> angles;
	std::vector<double> values;
	for (unsigned int i=0; i<10; ++i)
		angles.push_back(i*10); // in Deg

	values.push_back(1); // 0 deg
	values.push_back(1);
	values.push_back(1);
	values.push_back(1);
	values.push_back(0.98);
	values.push_back(0.94);
	values.push_back(0.86);
	values.push_back(0.69);
	values.push_back(0.37);
	values.push_back(0); // 90 deg

	// scale by nominal SHGC
	for (double & v : values)
		v *= 0.75; // default for double-glazing layer

	m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_name = "SHGC";
	m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_xUnit.set("Deg");
	m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_yUnit.set("---");
	m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values.setValues(angles, values);
	modelModify();
}


void SVDBWindowGlazingSystemEditWidget::on_pushButtonColor_colorChanged() {

	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify(); // tell model that we changed the data
	}
}


void SVDBWindowGlazingSystemEditWidget::on_lineEditSHGC0_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	// compute scale factor
	Q_ASSERT(!m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_name.empty() &&
		m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values.valid() );

	double newSHGC0 = m_ui->lineEditSHGC0->value();
	double oldSHGC0 = m_current->SHGC(); // ensured to be > 0

	double scaleFactor = newSHGC0/oldSHGC0;
	std::vector<double> newSHGCVals(m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values.y());
	for (double & v : newSHGCVals)
		v *= scaleFactor;
	std::vector<double> angles(m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values.x());
	m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values.setValues(angles, newSHGCVals);
	modelModify(); // tell model that we changed the data
	updateInput((int)m_current->m_id);
}


void SVDBWindowGlazingSystemEditWidget::on_lineEditUValue_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);
	VICUS::KeywordList::setParameter(m_current->m_para, "WindowGlazingSystem::para_t", VICUS::WindowGlazingSystem::P_ThermalTransmittance, m_ui->lineEditUValue->value());
	modelModify(); // tell model that we changed the data
}


void SVDBWindowGlazingSystemEditWidget::on_comboBoxType_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	// update database but only if different from original
	if (index != (int)m_current->m_modelType) {
		m_current->m_modelType = static_cast<VICUS::WindowGlazingSystem::modelType_t>(index);
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
	}
}


void SVDBWindowGlazingSystemEditWidget::on_toolButtonImportSplineFromClipboard_clicked() {
	// extract data from clipboard
	// get content of clip board
	QString data = qApp->clipboard()->text();
	if (data.isEmpty()) {
		QMessageBox::critical(this, tr("Cannot paste schedule data"), tr("No data on clipboard"));
		return;
	}
	// first replace all , with .
	std::replace(data.begin(), data.end(), ',', '.');
	QTextStream strm(&data);
	double angle, val;
	strm >> angle >> val;
	if (strm.status() != QTextStream::Ok) {
		QMessageBox::critical(this, tr("Cannot paste SHGC data"),
							  tr("Invalid format, expected table with two columns of numbers, separated by white-space character(s), without header line."));
		return;
	}

	if (!IBK::near_equal(angle, 0)) {
		QMessageBox::critical(this, tr("Cannot paste SHGC data"),
							  tr("Invalid data, expected 0 Deg in first row and first column."));
		return;
	}
	std::vector<double> angles;
	std::vector<double> values;
	while (strm.status() == QTextStream::Ok) {
		angles.push_back(angle);
		values.push_back(val);
		strm >> angle >> val;
	}

	if (angles.size() < 2) {
		QMessageBox::critical(this, tr("Cannot paste SHGC data"),
							  tr("Invalid data, expected at least two rows."));
		return;
	}
	if (!IBK::near_equal(angles.back(), 90)) {
		QMessageBox::critical(this, tr("Cannot paste SHGC data"),
							  tr("Invalid data, expected 90 Deg in last row and first column."));
		return;
	}
	for (const double & v : values) {
		if (v < 0 || v > 1) {
			QMessageBox::critical(this, tr("Cannot paste SHGC data"),
								  tr("Invalid data, SHGC values between 0 and 1."));
			return;
		}
	}

	m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values.setValues(angles, values);
	modelModify();
	updateInput((int)m_current->m_id);
}


void SVDBWindowGlazingSystemEditWidget::on_toolButtonCreateSpline_clicked() {
	createDefaultSHGCSpline();
	updateInput((int)m_current->m_id);
}
