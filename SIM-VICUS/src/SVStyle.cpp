#include "SVStyle.h"

#include <QFile>
#include <QApplication>

#include <QPlainTextEdit>
#include <QLayout>

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

	QFile file(":/style/VicusUIStyle.qss");
	if (file.exists()) {
		file.open(QFile::ReadOnly);
		m_styleSheet = QLatin1String(file.readAll());
		qApp->setStyleSheet(m_styleSheet);
	}

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

