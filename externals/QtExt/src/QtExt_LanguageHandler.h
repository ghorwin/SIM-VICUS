/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

*/

#ifndef QtExt_LanguageHandlerH
#define QtExt_LanguageHandlerH

#include <QString>
#include <QCoreApplication>

class QTranslator;

namespace QtExt {

/*! Central class that handles language switching.

	Uses Qt::Directories to find translation files.

	\code
	// example usage

	// first configure language handler with application settings
	LanguageHandler::setup("IBK", "PostProc 2.0", "PostProc2");

	// install german translation
	LanguageHandler::instance()->installTranslator("de");

	// loads language file 'PostProc2_de.qm'
	\endcode
*/
class LanguageHandler {
	Q_DISABLE_COPY(LanguageHandler)
public:
	/*! Returns an instance of the language handler singleton. */
	static LanguageHandler & instance();

	/*! Destructor, removes language handler objects. */
	~LanguageHandler();

	/*! Initializes the language handler with application-specific constants. */
	static void setup(const QString & organization, const QString & program,
					  const QString & languageFilePrefix);

	/*! Returns current language ID. */
	static QString langId();

	/*! Sets the language Id in the settings object. */
	static void setLangId(QString id);

	/*! Installs the translator identified by langId and stores the
		language ID in the program settings. */
	void installTranslator(QString langId);

private:
	static QString		m_organization;
	static QString		m_program;
	static QString		m_languageFilePrefix;

	/*! The translater for the strings of the program itself. */
	QTranslator * applicationTranslator;
	/*! The translater for strings of the standard dialogs and other Qt library
		messages. */
	QTranslator * systemTranslator;

	/*! Upon construction the translator objects are created and
		the last used translation setting is installed. */
	LanguageHandler();

};

} // namespace QtExt


#endif // QtExt_LanguageHandlerH
