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

#ifndef SVPluginManagerH
#define SVPluginManagerH

#include <QWidget>
#include <QUrl>

namespace Ui {
	class SVPluginManager;
}

class QNetworkAccessManager;
class QNetworkReply;

/*! The plugin-manager, shows installed and available plugins.

	When first created, the updateFromRepo() function is called, which downloads a list
	of plugins and their metadata from the server.

	updatePage() generates the html-code for all installed and available plugins. For installed plugins,
	the pixmaps generated/provided by the plugins are stored in a temporary location, where they are updated
	whenever the checksum changes.
	For available plugins, the referenced screenshots/icons are downloaded asynchronously when the "available
	plugin-page" is being shown.
*/
class SVPluginManager : public QWidget {
	Q_OBJECT
public:

	/*! Current page type. */
	enum PageType {
		PT_Installed,
		PT_Available
	};

	explicit SVPluginManager(QWidget *parent = nullptr);
	~SVPluginManager() override;

	/*! Updates the html-text. */
	void updatePage();

public slots:
	/*! Triggered when user clicks on a link in the html. Link prefix determines resulting action.
		Actions: "install:<plugin-id>", "update:<plugin-id>", "remove:<plugin-id>"
	*/
	void onAnchorClicked( const QUrl & link );

private slots:
	/*! Switch to page with installed plugins. */
	void on_toolInstalledPlugins_clicked();
	/*! Switch to page with available plugins. */
	void on_toolAvailablePlugins_clicked();
	/*! Triggered by network manager when request is complete. */
	void downloadFinished(QNetworkReply *reply);

private:
	Ui::SVPluginManager	*m_ui;

	QNetworkAccessManager	*m_networkManager;

	/*! Indicates which page should be shown. */
	PageType				m_pageType;
};

#endif // SVPluginManagerH
