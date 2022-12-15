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

#include "SVPluginManager.h"
#include "ui_SVPluginManager.h"

#include <QDesktopServices>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QPixmap>

#include "SVConstants.h"
#include "SVSettings.h"
#include "SVStyle.h"

extern const char * const INSTALLED_PLUGIN_TABLE_TEMPLATE;
extern const char * const AVAILABLE_PLUGIN_TABLE_TEMPLATE;
extern const char * const PLUGIN_HTML_TEMPLATE;

SVPluginManager::SVPluginManager(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPluginManager),
	m_networkManager(new QNetworkAccessManager),
	m_pageType(PT_Installed)
{
	m_ui->setupUi(this);
	connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(downloadFinished(QNetworkReply*)));
	connect(m_ui->textBrowser, SIGNAL(anchorClicked(QUrl)),
			this, SLOT(onAnchorClicked(QUrl)));
}


SVPluginManager::~SVPluginManager() {
	delete m_ui;
	delete m_networkManager;
}


void SVPluginManager::updatePage() {
	QString htmlPage;

	// TODO : compose page based on current page type and available data


	// finally set welcome page in textbrowser
	m_ui->textBrowser->setHtml(htmlPage);
}


void SVPluginManager::onAnchorClicked( const QUrl & link ) {
	// if anchor starts with "pfile:" we emit the "open project" signal
	if (link.toString().startsWith("install:")) {
		QString pluginID = link.toString();
		pluginID = pluginID.right(pluginID.length()-QString("install:").size());
		// TODO : download and install plugin
		return;
	}
	// if anchor starts with "pexample:" we emit the "open example" signal
	else if (link.toString().startsWith("update:")) {
		QString pluginID = link.toString();
		pluginID = pluginID.right(pluginID.length()-QString("update:").size());
		// TODO : download and update plugin
		return;
	}
	// if anchor starts with "ptemplate:" we emit the "open template" signal
	else if (link.toString().startsWith("remove:")) {
		QString pluginID = link.toString();
		pluginID = pluginID.right(pluginID.length()-QString("remove:").size());
		// TODO : remove plugin
		return;
	}
	// must be an external webpage link, call desktop services to open webbrowser
	QDesktopServices::openUrl(link);
}


void SVPluginManager::on_toolInstalledPlugins_clicked() {
	// switch to page with installed plugins
	m_pageType = PT_Installed;
	updatePage();
}


void SVPluginManager::on_toolAvailablePlugins_clicked() {
	// update page, trigger async download of not-yet-cached images
	m_pageType = PT_Available;
	updatePage();
}


void SVPluginManager::downloadFinished(QNetworkReply *reply) {
	if (reply->error()) {
		// TODO : show error message, if we were downloading the repo-file
	}
	else {
		QByteArray raw = reply->readAll();
		// extract text for current language
		QString json = QString::fromUtf8(raw);
		bool languageFound = false;

		// TODO : decode Json file
	}

	reply->deleteLater();
	// this will trigger a welcome page update (but no further download attempt)
	updatePage();
}


const char * const PLUGIN_HTML_TEMPLATE =
		"<html>\n"
		"<head>\n"
		"<style type=\"text/css\">\n"
		"body     { font-size: medium; color: ${STYLE_TEXT_COLOR}; background-color: ${STYLE_BACKGROUND_COLOR} }\n"
		"a        { color: ${STYLE_LINKTEXT_COLOR}; text-decoration:none }\n"
		"a:hover  { color: ${STYLE_LINKTEXT_HOVER_COLOR}; background-color: ${STYLE_LINKTEXT_HOVER_BACKGROUND_COLOR}; }\n"
		"p        { font-size: medium; text-align: justify; margin-top:0px; margin-bottom:8px;   }\n"
		"h1       { font-size: large; color: ${STYLE_H1_COLOR}; font-weight:bold; \n"
		"           text-decoration: none; margin-top:15px; margin-bottom:15px }\n"
		"h2       { font-size: medium; color: ${STYLE_H2_COLOR}; font-weight:bold; margin-top:15px; margin-bottom:6px }\n"
		"h3       { font-size: medium; color: ${STYLE_H3_COLOR}; font-weight:bold; margin-top:10px; margin-bottom:2px}\n"
		"table    { font-size: medium }\n"
		"b        { color: black }\n"
		"pre      { font-size: small; font-family: monospace;courier }\n"
		"li       { text-align: justify }\n"
		"</style>\n"
		"\n"
		"</head>\n"
		"<body>\n"
		"<table border=\"0\" cellspacing=\"10\" cellpadding=\"0\" width=\"100%\">\n"
		"<tr valign=\"top\">\n"
		"<td>\n"
		"<p>\n"
		"<hr>\n"
		"${RECENT_PROJECT_FILELIST}\n"
		"\n"
		"</td>\n"
		"<td width=30>&nbsp;</td>\n"
		"<td width=\"20%\">\n"
		"${SIDE_BAR}\n"
		"</td>\n"
		"\n"
		"</tr>\n"
		"</table>\n"
		"\n"
		"</body>\n"
		"</html>\n"
		"\n";

const char * const INSTALLED_PLUGIN_TABLE_TEMPLATE =
		"<table border=\"0\" cellspacing=\"2\" cellpadding=\"0\">\n"
		"<tr valign=center><th width=\"${THUMBNAILSIZE}\" rowspan=\"3\">${IMG_FILENAME}</th><th align=left>${PROJECT_FILENAME}</th></tr>\n"
		"<tr valign=center><td align=left><a href=\"pfile:${PROJECT_FULL_PATH}\">${PROJECT_FULL_PATH}</a></td></tr>\n"
		"<tr valign=top><td align=justify>${PROJECT_DESCRIPTION}</td></tr>\n"
		"</table>\n"
		"<br>\n"
		"\n";

const char * const AVAILABLE_PLUGIN_TABLE_TEMPLATE =
		"<table border=\"0\" cellspacing=\"2\" cellpadding=\"0\">\n"
		"<tr valign=center><th width=\"${THUMBNAILSIZE}\" rowspan=\"3\">${IMG_FILENAME}</th><th align=left>${PROJECT_FILENAME}</th></tr>\n"
		"<tr valign=center><td align=left><a href=\"pfile:${PROJECT_FULL_PATH}\">${PROJECT_FULL_PATH}</a></td></tr>\n"
		"<tr valign=top><td align=justify>${PROJECT_DESCRIPTION}</td></tr>\n"
		"</table>\n"
		"<br>\n"
		"\n";



