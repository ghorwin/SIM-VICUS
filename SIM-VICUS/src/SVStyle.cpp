#include "SVStyle.h"

#include <QFile>
#include <QApplication>

#include <QPlainTextEdit>
#include <QLayout>
#include <QTableView>
#include <QHeaderView>

#include "SVSettings.h"

SVStyle * SVStyle::m_self = nullptr;

SVStyle & SVStyle::instance() {
	Q_ASSERT_X(m_self != nullptr, "[SVStyle::instance]", "You must create an instance of "
		"SVStyle before accessing SVStyle::instance()!");
	return *m_self;
}



SVStyle::SVStyle() {

	Q_ASSERT(m_self == nullptr);
	m_self = this;

	// customize application font
	unsigned int ps = SVSettings::instance().m_fontPointSize;
	if (ps != 0) {
		QFont f(qApp->font());
		f.setPointSize((int)ps);
		qApp->setFont(f);
	}

	/// \todo anyone up to creating a "dark" style?

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

	m_alternativeBackgroundBright = "#fff4b8";
	m_alternativeBackgroundDark = "#ffe49d";
	m_alternativeBackgroundText = "#760000";
	m_readOnlyEditFieldBackground = "#d6e9ff";
	m_alternativeReadOnlyEditFieldBackground = "#b5d8ff";
	m_errorEditFieldBackground = "#ff7777";
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
	v->setSelectionBehavior(QAbstractItemView::SelectRows);
	v->setSelectionMode(QAbstractItemView::SingleSelection);
	v->setSortingEnabled(true);
	v->sortByColumn(0, Qt::AscendingOrder);
	QFont f = v->font();
	f.setPointSizeF(f.pointSizeF()*0.8);
	v->setFont(f);
	v->horizontalHeader()->setFont(f);
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
		/// TODO : Stephan: adjust theme color members
	}
	else if ( theme == SVSettings::TT_White && fileWhite.exists()) {
		fileWhite.open(QFile::ReadOnly);
		m_styleSheet = QLatin1String(fileWhite.readAll());
		qApp->setStyleSheet(m_styleSheet);
		/// TODO : Stephan: adjust theme color members
	}
	else {
		// clear style sheet for default style.
		qApp->setStyleSheet("");
	}
}

