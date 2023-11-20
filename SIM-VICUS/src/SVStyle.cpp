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

#include "SVStyle.h"

#include <QFile>
#include <QApplication>

#include <QPlainTextEdit>
#include <QLayout>
#include <QTableView>
#include <QTreeView>
#include <QLineEdit>
#include <QHeaderView>
#include <QDebug>
#include <QColor>
#if QT_VERSION >= 0x050a00
#include <QRandomGenerator>
#endif

#include <QtExt_Style.h>
#include <QListView>

#include "SVSettings.h"

SVStyle * SVStyle::m_self = nullptr;

double SVStyle::m_randomColorHueValue = 0.1;

SVStyle & SVStyle::instance() {
	Q_ASSERT_X(m_self != nullptr, "[SVStyle::instance]", "You must create an instance of "
		"SVStyle before accessing SVStyle::instance()!");
	return *m_self;
}


SVStyle::SVStyle() {

	Q_ASSERT(m_self == nullptr);
	m_self = this;

#if QT_VERSION < 0x050a00
	qsrand(time(nullptr));
#endif


	// customize application font
	unsigned int ps = SVSettings::instance().m_fontPointSize;
	if (ps != 0) {
		QFont f(qApp->font());
		f.setPointSize((int)ps);
		qApp->setFont(f);
	}

	/// \todo Stephan: identify all style/layout/icon/graphical elements that need to be
	///       specific to either the dark or bright style. Everything not handled in
	///       the style sheet, needs to be in SVStyle-member variables (like the colors).

	// *** Style Customization ***

	SVSettings & s = SVSettings::instance();
	setStyle(s.m_theme);


#ifdef Q_OS_MACX
	m_fontMonoSpace = "Monaco";
	m_fontMonoSpacePointSize = 12;
#elif defined(Q_OS_UNIX)
	m_fontMonoSpace = "monospace";
	m_fontMonoSpacePointSize = 9;
#endif // Q_OS_UNIX
#ifdef Q_OS_WIN
	m_fontMonoSpace = "Courier New";
	m_fontMonoSpacePointSize = 9;
#endif // Q_OS_WIN

}


void SVStyle::formatPlainTextEdit(QPlainTextEdit * textEdit) const {

	// customize log window
	QFont f;
	f.setFamily(m_fontMonoSpace);
	f.setPointSize((int)m_fontMonoSpacePointSize);
	textEdit->setFont(f);

	textEdit->setContextMenuPolicy(Qt::NoContextMenu);
	textEdit->setReadOnly(true);
	textEdit->setUndoRedoEnabled(false);
	textEdit->setWordWrapMode(QTextOption::NoWrap);
}


void SVStyle::formatWidgetWithLayout(QWidget * w) {
	// retrieve top-level layout
	QLayout * l = w->layout();
	// set margins to 0
	l->setMargin(0);

	// TODO : other customization or call formatWidget() function for basic styling
}


void SVStyle::formatDatabaseTableView(QTableView * v) {
	v->verticalHeader()->setDefaultSectionSize(19);
	v->verticalHeader()->setVisible(false);
	v->horizontalHeader()->setMinimumSectionSize(19);
	v->setSelectionBehavior(QAbstractItemView::SelectRows);
	v->setSelectionMode(QAbstractItemView::SingleSelection);
	v->setAlternatingRowColors(true);
	v->setSortingEnabled(true);
	v->sortByColumn(0, Qt::AscendingOrder);
#ifdef Q_OS_WIN
	// Since we have an applition-wide style sheet, we must specify our customizations also via stylesheet.
	// Since we only tweak font sizes, this works for both bright and dark style.
	QString headerStyleSheet = QString("QHeaderView::section:horizontal {font-weight:bold;}");
	v->horizontalHeader()->setStyleSheet(headerStyleSheet);
#else
	QFont f;
	int pointSize = int(f.pointSizeF()*0.8);

	// v->setFont(f);
	// v->horizontalHeader()->setFont(f); // Note: on Linux/Mac this won't work until Qt 5.11.1 - this was a bug between Qt 4.8...5.11.1

	// Since we have an applition-wide style sheet, we must specify our customizations also via stylesheet.
	// Since we only tweak font sizes, this works for both bright and dark style.
	QString headerStyleSheet = QString("QHeaderView::section:horizontal {font-size:%1pt; font-weight:bold;}").arg(pointSize);
	v->horizontalHeader()->setStyleSheet(headerStyleSheet);
	QString viewStyleSheet = QString("QTableView {font-size:%1pt;}").arg(pointSize);
	v->setStyleSheet(viewStyleSheet);
#endif
}


void SVStyle::formatListView(QListView * v) {
	v->setAlternatingRowColors(true);
#ifdef Q_OS_WIN
	// no need to adjust stylesheet on Windows
#else
	QFont f;
	int pointSize = int(f.pointSizeF()*0.8);
	QString viewStyleSheet = QString("QListView {font-size:%1pt;}").arg(pointSize);
	v->setStyleSheet(viewStyleSheet);
#endif
}

void SVStyle::formatDatabaseTreeView(QTreeView * v) {
	v->header()->setMinimumSectionSize(19);
	v->setSelectionBehavior(QAbstractItemView::SelectRows);
	v->setSelectionMode(QAbstractItemView::SingleSelection);
	v->setAlternatingRowColors(true);
	v->setSortingEnabled(false);
	v->setIndentation(19);
//	v->sortByColumn(0, Qt::AscendingOrder);
#if !defined(Q_OS_WIN)
	QFont f;
	int pointSize = int(f.pointSizeF()*0.8);

	// Since we have an applition-wide style sheet, we must specify our customizations also via stylesheet.
	// Since we only tweak font sizes, this works for both bright and dark style.
	QString headerStyleSheet = QString("QHeaderView::section {font-size:%1pt;}").arg(pointSize);
	v->header()->setStyleSheet(headerStyleSheet);
	QString viewStyleSheet = QString("QTreeView {font-size:%1pt;}").arg(pointSize);
	v->setStyleSheet(viewStyleSheet);
#endif
}

void SVStyle::setHtmlColors(QString & htmlCode) {
	switch (SVSettings::instance().m_theme) {
		case SVSettings::TT_Dark :
		{
			htmlCode.replace("${STYLE_TEXT_COLOR}", "#F0F0F0");
			htmlCode.replace("${STYLE_BACKGROUND_COLOR}", "#212124");
			htmlCode.replace("${STYLE_LINKTEXT_COLOR}", "#ffbf14");
			htmlCode.replace("${STYLE_LINKTEXT_HOVER_COLOR}", "#ffffff");
			htmlCode.replace("${STYLE_LINKTEXT_HOVER_BACKGROUND_COLOR}", "#19232D");
			htmlCode.replace("${STYLE_H1_COLOR}", "#ff7e16");
			htmlCode.replace("${STYLE_H2_COLOR}", "#ff5b1a");
			htmlCode.replace("${STYLE_H3_COLOR}", "#ff5b1a");
		} break;

		case SVSettings::TT_White :
		default:
		{
			htmlCode.replace("${STYLE_TEXT_COLOR}", qApp->palette().color(QPalette::Text).name());
			htmlCode.replace("${STYLE_BACKGROUND_COLOR}", qApp->palette().color(QPalette::Background).name());
			htmlCode.replace("${STYLE_LINKTEXT_COLOR}", "#0053A6");
			htmlCode.replace("${STYLE_LINKTEXT_HOVER_COLOR}", "#1C7DEF");
			htmlCode.replace("${STYLE_LINKTEXT_HOVER_BACKGROUND_COLOR}", qApp->palette().color(QPalette::Background).name());
			htmlCode.replace("${STYLE_H1_COLOR}", "#003264");
			htmlCode.replace("${STYLE_H2_COLOR}", "#0069A8");
			htmlCode.replace("${STYLE_H3_COLOR}", "#00660F");
		} break;
	}

}


void SVStyle::resizeTableColumnToContents(QTableView * v, int column, bool enlargeOnly) {
	int w = v->columnWidth(column);
	v->resizeColumnToContents(0);
	int w2 = v->columnWidth(column);
	if (enlargeOnly && w2 < w)
		v->setColumnWidth(column, w);
}


void SVStyle::setStyle(SVSettings::ThemeType theme) {
	QFile styleDark(":/qdarkstyle/style.qss");
	QFile styleWhite(":/qdarkstyle/whitestyle.qss");

	if ( theme == SVSettings::TT_Dark && styleDark.exists()) {

		styleDark.open(QFile::ReadOnly);
		m_styleSheet = QLatin1String(styleDark.readAll());
		qApp->setStyleSheet(m_styleSheet);
		// set specific background/font colors
		m_alternativeBackgroundBright				= "#73580e";
		m_alternativeBackgroundDark					= "#57430b";
		m_alternativeBackgroundText					= "#ffedce";

		m_readOnlyEditFieldBackground				= "#5f7da0";
		m_alternativeReadOnlyEditFieldBackground	= "#7f94ab";
		m_errorEditFieldBackground					= "#ab4e4e";

		m_userDBBackgroundDark						= "#012a4a";
		m_userDBBackgroundBright					= "#013a63";
		m_regularDBEntryColorDark					= "#e0e0e0";
		m_DBSelectionColor							= "#4a8522";

		m_logProgressText							= "#c0c0c0";
		m_logErrorText								= "#ff2222";
		m_logWarningText							= "#f0dc00";
		m_logDebugText								= "#39b1d9";

		m_defaultDrawingColor						= "#FFFFFF";

		// now adjust the style settings for QtExt components
		QtExt::Style::EditFieldBackground = "#212124";
		QtExt::Style::AlternativeEditFieldBackground = "#3a3b3f";
		QtExt::Style::ErrorEditFieldBackground = "#ab4e4e";
		QtExt::Style::ReadOnlyEditFieldBackground = "#5f7da0";

		QtExt::Style::AlternativeBackgroundBright = "#73580e";
		QtExt::Style::AlternativeBackgroundDark = "#57430b";
		QtExt::Style::AlternativeBackgroundText = "#ffedce";

		QtExt::Style::ToolBoxPageBackground		= "#212124";
		QtExt::Style::ToolBoxPageEdge			= "#3a3b3f";
	}
	else if ( theme == SVSettings::TT_White && styleWhite.exists()) {

		styleWhite.open(QFile::ReadOnly);
		m_styleSheet = QLatin1String(styleWhite.readAll());
		qApp->setStyleSheet(m_styleSheet);

		// set specific background/font colors
		m_alternativeBackgroundBright				= "#fff4b8";
		m_alternativeBackgroundDark					= "#ffe49d";
		m_alternativeBackgroundText					= "#760000";
		m_readOnlyEditFieldBackground				= "#d6e9ff";
		m_alternativeReadOnlyEditFieldBackground	= "#b5d8ff";
		m_errorEditFieldBackground					= "#ff7777";

		m_userDBBackgroundDark						= "#cddafd";
		m_userDBBackgroundBright					= "#dfe7fd";
		m_DBSelectionColor							= "#4a8522";

		m_logProgressText							= "#202020";
		m_logErrorText								= "#ab0000";
		m_logWarningText							= "#b17d00";
		m_logDebugText								= "#39b1d9";

		m_defaultDrawingColor						= "#000000";

		QtExt::Style::EditFieldBackground						= "#f9f6c8";
		QtExt::Style::AlternativeEditFieldBackground			= "#f9ffd8";
		QtExt::Style::ErrorEditFieldBackground					= "#ff7777";
		QtExt::Style::ReadOnlyEditFieldBackground				= "#d6e9ff";

		QtExt::Style::AlternativeBackgroundBright				= "#fff4b8";
		QtExt::Style::AlternativeBackgroundDark					= "#ffe49d";
		QtExt::Style::AlternativeBackgroundText					= "#760000";

		QtExt::Style::ToolBoxPageBackground						= "#ffffff";
		QtExt::Style::ToolBoxPageEdge							= "#f0f0f0";
	}
	else {
		// clear style sheet for default style.
		qApp->setStyleSheet("");
	}
	// Note: other, widget-specific adjustments, like change of action icons etc. is handled
	//       in the individual format update slots connected to the SVPreferencesPageStyle::styleChanged() slots.
}


QColor SVStyle::randomColor() {
	double s = 1;
	double v = 1;
	bool useS = true;
#if QT_VERSION >= 0x050a00
	m_randomColorHueValue += QRandomGenerator::global()->generateDouble()*0.2;
	s = (QRandomGenerator::global()->generateDouble()*3. + 7)/10.; // a value between 0.8 and 1
	v = (QRandomGenerator::global()->generateDouble()*3. + 7)/10.; // a value between 0.8 and 1
	useS = (QRandomGenerator::global()->generateDouble() < 0.5);

#else
	m_randomColorHueValue += double(qrand())/RAND_MAX*0.4;
	s = (qrand()*3./RAND_MAX + 7)/10.;
	v = (qrand()*1./RAND_MAX + 9)/10.;
#endif
	m_randomColorHueValue += 0.08;

	while (m_randomColorHueValue > 1)
		m_randomColorHueValue -= 1;

	QColor col;
	if (useS)
		col = QColor::fromHsvF(m_randomColorHueValue, s, 1);
	else
		col = QColor::fromHsvF(m_randomColorHueValue, 1, v);
	return col;
}

