/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#include "QtExt_LanguageHandler.h"
#include <QTranslator>
#include <QSettings>
#include <QCoreApplication>
#include <QLocale>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

#include <IBK_messages.h>
#include <IBK_FormatString.h>

#include "QtExt_Directories.h"

namespace QtExt {

QString		LanguageHandler::m_organization;
QString		LanguageHandler::m_program;
QString		LanguageHandler::m_languageFilePrefix;

LanguageHandler & LanguageHandler::instance() {
	static LanguageHandler myHandler;
	return myHandler;
}


LanguageHandler::LanguageHandler() :
	applicationTranslator(NULL),
	systemTranslator(NULL)
{
}


LanguageHandler::~LanguageHandler() {
	// get rid of old translators
	// at this time, the application object doesn't live anylonger, so we
	// can savely destruct the translator objects
	delete applicationTranslator; applicationTranslator = NULL;
	delete systemTranslator; systemTranslator = NULL;
}


void LanguageHandler::setup(const QString & organization, const QString & program,
							const QString & languageFilePrefix)
{
	m_organization = organization;
	m_program = program;
	m_languageFilePrefix = languageFilePrefix;
}


QString LanguageHandler::langId() {
	const char * const FUNC_ID = "[LanguageHandler::langId]";

	QSettings config(m_organization, m_program);
	QString langid = config.value("LangID", QString() ).toString();
	if (langid.isEmpty()) {
		// try to determine language id from OS
		QString localeName = QLocale::system().name();
		IBK::IBK_Message( IBK::FormatString("System locale: '%1'.\n").arg(localeName.toStdString()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		int pos = localeName.indexOf('_');
		if (pos != -1)
			localeName = localeName.left(pos);
		IBK::IBK_Message( IBK::FormatString("Translation required for locale: '%1'.\n").arg(localeName.toStdString()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		langid = localeName;

		if (langid.isEmpty())
			langid = "en";
	}
	return langid;
}


void LanguageHandler::setLangId(QString id) {
	QSettings config(m_organization, m_program);
	config.setValue("LangID", id );
}


void LanguageHandler::installTranslator(QString langId) {
	const char * const FUNC_ID = "[LanguageHandler::installTranslator]";

	// get rid of old translators
	if (applicationTranslator != NULL) {
		qApp->removeTranslator(applicationTranslator);
		delete applicationTranslator; applicationTranslator = NULL;
	}
	if (systemTranslator != NULL) {
		qApp->removeTranslator(systemTranslator);
		delete systemTranslator; systemTranslator = NULL;
	}


	// create new translators, unless we are using english
	if (langId == "en") {
		QSettings config(m_organization, m_program);
		config.setValue("LangID", langId);
		QLocale loc(QLocale::English);
		loc.setNumberOptions(QLocale::OmitGroupSeparator | QLocale::RejectGroupSeparator);
		QLocale::setDefault(loc);
		return;
	}

	QString translationFilePath = QtExt::Directories::translationsFilePath(langId);
	QString qtTranslationFilePath = QtExt::Directories::qtTranslationsFilePath(langId);

	IBK::IBK_Message( IBK::FormatString("App translation file path = '%1'.\n").arg(translationFilePath.toStdString()),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Qt translation file path  = '%1'.\n").arg(qtTranslationFilePath.toStdString()),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	systemTranslator = new QTranslator;

	// system translator first, filename is for example "qt_de"
	systemTranslator = new QTranslator;
	QFileInfo finfoQt(qtTranslationFilePath);
	if (finfoQt.exists() && systemTranslator->load(finfoQt.baseName(), finfoQt.dir().absolutePath())) {
		qApp->installTranslator(systemTranslator);
		IBK::IBK_Message( IBK::FormatString("Qt translation file loaded successfully\n"),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	else {
		IBK::IBK_Message( IBK::FormatString("Could not load system translator file: 'qt_%1'.\n").arg(langId.toStdString()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		// no translator found, remove it again
		delete systemTranslator;
		systemTranslator = nullptr;
	}

	applicationTranslator = new QTranslator;
	QFileInfo finfo(translationFilePath);
	if (finfo.exists() && applicationTranslator->load(finfo.baseName(), finfo.dir().absolutePath())) {
		IBK::IBK_Message( IBK::FormatString("Application translator loaded successfully\n"),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		qApp->installTranslator(applicationTranslator);
		// remember translator in settings
		QSettings config(m_organization, m_program);
		config.setValue("LangID", langId);
	}
	else {
		IBK::IBK_Message( IBK::FormatString("Could not load application translator file: '%1'.\n").arg(translationFilePath.toStdString()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		delete applicationTranslator;
		applicationTranslator = nullptr;
	}


	// now also set the corresponding locale settings (for number display etc.)
	if (langId == "de") {
		QLocale loc(QLocale::German);
		loc.setNumberOptions(QLocale::OmitGroupSeparator | QLocale::RejectGroupSeparator);
		QLocale::setDefault(loc);
	}
}

} // namespace QtExt
