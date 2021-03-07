#include "SVStyle.h"

#include <QFile>
#include <QApplication>

#include <QPlainTextEdit>
#include <QLayout>
#include <QTableView>
#include <QTreeView>
#include <QHeaderView>
#include <QDebug>
#if QT_VERSION >= 0x050a00
#include <QRandomGenerator>
#endif

#include <QtExt_Style.h>

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
	QFont f;
	f.setPointSizeF(f.pointSizeF()*0.8);
	v->setFont(f);
	v->horizontalHeader()->setFont(f); // Note: on Linux/Mac this won't work until Qt 5.11.1 - this was a bug between Qt 4.8...5.11.1
}


void SVStyle::formatDatabaseTreeView(QTreeView * v) {
	v->header()->setMinimumSectionSize(19);
	v->setSelectionBehavior(QAbstractItemView::SelectRows);
	v->setSelectionMode(QAbstractItemView::SingleSelection);
	v->setAlternatingRowColors(true);
	v->setSortingEnabled(true);
	v->sortByColumn(0, Qt::AscendingOrder);
	QFont f;
	f.setPointSizeF(f.pointSizeF()*0.8);
	v->setFont(f);
	v->header()->setFont(f); // Note: on Linux/Mac this won't work until Qt 5.11.1 - this was a bug between Qt 4.8...5.11.1
}

void SVStyle::formatWelcomePage(QString & htmlCode) {
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
			htmlCode.replace("${STYLE_BACKGROUND_COLOR}", "#FFFFFF");
			htmlCode.replace("${STYLE_LINKTEXT_COLOR}", "#0053A6");
			htmlCode.replace("${STYLE_LINKTEXT_HOVER_COLOR}", "#1C7DEF");
			htmlCode.replace("${STYLE_LINKTEXT_HOVER_BACKGROUND_COLOR}", qApp->palette().color(QPalette::Background).name());
			htmlCode.replace("${STYLE_H1_COLOR}", "#003264");
			htmlCode.replace("${STYLE_H2_COLOR}", "#0069A8");
			htmlCode.replace("${STYLE_H3_COLOR}", "#00660F");
		} break;
	}

}

void SVStyle::setStyle(SVSettings::ThemeType theme) {
	QFile file(":/qdarkstyle/style.qss");
	QFile fileWhite(":/qdarkstyle/whitestyle.qss");

	if ( theme == SVSettings::TT_Dark && file.exists()) {
		file.open(QFile::ReadOnly);
		m_styleSheet = QLatin1String(file.readAll());
		qApp->setStyleSheet(m_styleSheet);
		// set specific background/font colors
		m_alternativeBackgroundBright				= "#73580e";
		m_alternativeBackgroundDark					= "#57430b";
		m_alternativeBackgroundText					= "#ffedce";
		m_readOnlyEditFieldBackground				= "#5f7da0";
		m_alternativeReadOnlyEditFieldBackground	= "#7f94ab";
		m_errorEditFieldBackground					= "#ab4e4e";

		m_logProgressText							= "#c0c0c0";
		m_logErrorText								= "#ff2222";
		m_logWarningText							= "#f0dc00";
		m_logDebugText								= "#39b1d9";

		// now adjust the style settings for QtExt components
		QtExt::Style::EditFieldBackground = "#212124";
		QtExt::Style::AlternativeEditFieldBackground = "#3a3b3f";
		QtExt::Style::ErrorEditFieldBackground = "#ab4e4e";
		QtExt::Style::ReadOnlyEditFieldBackground = "#5f7da0";

		QtExt::Style::AlternativeBackgroundBright = "#73580e";
		QtExt::Style::AlternativeBackgroundDark = "#57430b";
		QtExt::Style::AlternativeBackgroundText = "#ffedce";
	}
	else if ( theme == SVSettings::TT_White && fileWhite.exists()) {
		fileWhite.open(QFile::ReadOnly);
		m_styleSheet = QLatin1String(fileWhite.readAll());
		qApp->setStyleSheet(m_styleSheet);

		// set specific background/font colors
		m_alternativeBackgroundBright				= "#fff4b8";
		m_alternativeBackgroundDark					= "#ffe49d";
		m_alternativeBackgroundText					= "#760000";
		m_readOnlyEditFieldBackground				= "#d6e9ff";
		m_alternativeReadOnlyEditFieldBackground	= "#b5d8ff";
		m_errorEditFieldBackground					= "#ff7777";

		m_logProgressText							= "#202020";
		m_logErrorText								= "#ab0000";
		m_logWarningText							= "#b17d00";
		m_logDebugText								= "#39b1d9";

		QtExt::Style::EditFieldBackground						= "#f9f6c8";
		QtExt::Style::AlternativeEditFieldBackground			= "#f9ffd8";
		QtExt::Style::ErrorEditFieldBackground					= "#ff7777";
		QtExt::Style::ReadOnlyEditFieldBackground				= "#d6e9ff";

		QtExt::Style::AlternativeBackgroundBright				= "#fff4b8";
		QtExt::Style::AlternativeBackgroundDark					= "#ffe49d";
		QtExt::Style::AlternativeBackgroundText					= "#760000";

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
#if QT_VERSION >= 0x050a00
	m_randomColorHueValue += QRandomGenerator().generateDouble()*0.4;
	s = QRandomGenerator().generateDouble()*4;
	s = (s*2 + 2)/10;
#else
	m_randomColorHueValue += double(qrand())/RAND_MAX*0.4;
	s = std::floor(double(qrand())/RAND_MAX*4);
	s = (s*2 + 2)/10;
#endif
	m_randomColorHueValue += 0.05;

	if (m_randomColorHueValue > 1)
		m_randomColorHueValue -= 1;

	QColor col = QColor::fromHsvF(m_randomColorHueValue, s, 1);
	return col;
}

