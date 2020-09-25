#include "SVWelcomeScreen.h"
#include "ui_SVWelcomeScreen.h"

#include <QDesktopServices>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QPixmap>

#include <VICUS_Project.h>
#include <VICUS_Constants.h>

#include <IBK_Version.h>
#include <IBK_BuildFlags.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Directories.h>

#include "SVConstants.h"
#include "SVSettings.h"

extern const char * const HTML_TEMPLATE;
extern const char * const RECENT_PROJECT_TABLE_TEMPLATE;

SVWelcomeScreen::SVWelcomeScreen(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVWelcomeScreen),
	m_networkManager(new QNetworkAccessManager),
	m_autoUpdater(new QtExt::AutoUpdater(this)),
	m_pageType(PT_RecentFiles)
{
	m_ui->setupUi(this);
	connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(downloadFinished(QNetworkReply*)));
	connect(m_ui->textBrowser, SIGNAL(anchorClicked(QUrl)),
			this, SLOT(onAnchorClicked(QUrl)));
	connect(m_autoUpdater, SIGNAL(updateInfoRetrieved(int, QString)),
			this, SLOT(onUpdateInfoRetrieved(int, QString)) );
	m_autoUpdater->checkForUpdateInfo(UPDATE_FILE_URL,
									  VICUS::LONG_VERSION, false, QString(), QString());
}


SVWelcomeScreen::~SVWelcomeScreen() {
	delete m_ui;
}


void SVWelcomeScreen::updateWelcomePage() {
	QString welcomePageSideBar = QString("<p>&nbsp;<p><hr>"); // top-rule starts the side-bar column
	// add update info section
	if (!m_updateInfoNews.isEmpty())
		welcomePageSideBar.append(m_updateInfoNews);
	// download news content (only once per application start)
	if (m_welcomePageNews.isEmpty()) {
		QNetworkRequest request(QUrl(QString::fromLatin1(NEWS_FILE_URL)));
		/*QNetworkReply *reply = */m_networkManager->get(request);
	}
	// now create overall page
	QString htmlPage = HTML_TEMPLATE;
	// insert news
	if (m_welcomePageNews.isEmpty()) {
		welcomePageSideBar.append(QString("<i>%1</i>").arg(tr("Retrieving news content...")));
	}
	else {
		welcomePageSideBar.append(m_welcomePageNews);
	}
	htmlPage.replace("${SIDE_BAR}", welcomePageSideBar);
	htmlPage.replace("${RECENT_PROJECTS_LINK}", tr("Recently opened projects"));
	htmlPage.replace("${DEMO_CASES_LINK}", tr("Examples/Validation cases"));
	htmlPage.replace("${TEMPLATES_LINK}", tr("Project templates"));


	if (m_pageType == PT_RecentFiles) {
		// compose recent project file list table

		QString recentProjects = "<h1>" + tr("Recently opened projects") + "</h1><p>\n";
		for (int i=0; i<SVSettings::instance().m_recentProjects.count(); ++i) {
			QFileInfo finfo(SVSettings::instance().m_recentProjects[i]);
			QString projectInfoBlock = RECENT_PROJECT_TABLE_TEMPLATE;
			QString description;
			// check if file still exists
			if (finfo.exists()) {
				VICUS::Project pro;
				try {
					pro.readXML(IBK::Path(finfo.filePath().toStdString()));
				}
				catch (IBK::Exception &) {
					// error reading project file, missing permissions?
					pro = VICUS::Project(); // reset with empty instance
				}
				QString created;
				if (pro.m_nandradData.m_projectInfo.m_created.empty())
					created = "---";
				else
					created = QString::fromStdString(pro.m_nandradData.m_projectInfo.m_created);
				QString lastModified;
				if (pro.m_nandradData.m_projectInfo.m_lastEdited.empty())
					lastModified = "---";
				else
					lastModified = QString::fromStdString(pro.m_nandradData.m_projectInfo.m_lastEdited);
				description = tr("<i>Created: %1, Last modified: %2</i><br>%3")
									  .arg(created)
									  .arg(lastModified)
									  .arg(QString::fromStdString(pro.m_nandradData.m_projectInfo.m_comment));
			}
			else {
				description = tr("<i><font color=\"#800000\">Project not accessible</font></i> <a href=\"projectRemove:%1\">Remove %2</a>").arg( i ).arg( finfo.fileName() );
			}

			QString thumbPath = QtExt::Directories::userDataDir()  + "/thumbs/" + finfo.fileName() + ".png";

			QFileInfo thumbFileInfo(thumbPath);
			// check if file exists
			if (thumbFileInfo.exists())
				thumbPath = "<a href=\"projectFile:${PROJECT_FULL_PATH}\"><img src=\"" + thumbFileInfo.absoluteFilePath() + "\"></a>&nbsp;";
			else
				thumbPath = "&nbsp;";
			thumbPath = thumbPath.replace("${PROJECT_FULL_PATH}", finfo.filePath());

			projectInfoBlock = projectInfoBlock.replace("${THUMBNAILSIZE}", QString("%1").arg(SVSettings::instance().m_thumbNailSize+8));
			projectInfoBlock = projectInfoBlock.replace("${PROJECT_FILENAME}", finfo.fileName());
			projectInfoBlock = projectInfoBlock.replace("${PROJECT_FULL_PATH}", finfo.filePath());
			projectInfoBlock = projectInfoBlock.replace("${PROJECT_DESCRIPTION}", description);
			projectInfoBlock = projectInfoBlock.replace("${IMG_FILENAME}", thumbPath);
			recentProjects += projectInfoBlock;
		}
		htmlPage.replace("${RECENT_PROJECT_FILELIST}", recentProjects);
	}
	else if(m_pageType == PT_TemplateFiles) {
		QString templates = "<h1>" + tr("Project templates") + "</h1><p>\n";

		// process all project packages in template directory
		QDir templatesDir(QtExt::Directories::userDataDir()+"/DB_templates");

		QStringList templateFiles;
		SVSettings::recursiveSearch(templatesDir, templateFiles, QStringList() << "vicpac");

		// for each file, parse its header, extract comments, open thumbnail image and compose table with information
		foreach (QString fname, templateFiles) {
			QString projectInfoBlock = RECENT_PROJECT_TABLE_TEMPLATE;
			// for templates we use a special link handler
			projectInfoBlock.replace("projectFile:$", "projectTemplate:$");
			VICUS::Project pro;
			QFileInfo finfo(fname);
			try {
				pro.readXML(IBK::Path(finfo.filePath().toStdString()));
			}
			catch (IBK::Exception &) {
				// error reading project file, missing permissions?
				pro = VICUS::Project(); // reset with empty instance
			}
			QString created;
			if (pro.m_nandradData.m_projectInfo.m_created.empty())
				created = "---";
			else
				created = QString::fromStdString(pro.m_nandradData.m_projectInfo.m_created);
			QString description = QString::fromStdString(pro.m_nandradData.m_projectInfo.m_comment);

			projectInfoBlock = projectInfoBlock.replace("${PROJECT_FILENAME}", finfo.baseName());
			projectInfoBlock = projectInfoBlock.replace("${PROJECT_FULL_PATH}", finfo.filePath());
			projectInfoBlock = projectInfoBlock.replace("${PROJECT_DESCRIPTION}", description);

			// open thumbnail image for this project
			QString thumbPath = fname+".png";

			QFileInfo thumbFileInfo(thumbPath);
			QPixmap p;
			// check if file exists
			if (thumbFileInfo.exists() && p.load(thumbPath)) {
				/// \todo fix warning about bad resource loading
				thumbPath = "<a href=\"projectFile:${PROJECT_FULL_PATH}\"><img src=\"" + thumbFileInfo.absoluteFilePath() + "\"></a>&nbsp;";
				thumbPath = thumbPath.replace("${PROJECT_FULL_PATH}", finfo.filePath());
				projectInfoBlock = projectInfoBlock.replace("${THUMBNAILSIZE}", QString("%1").arg(p.width()+10));
			}
			else {
				thumbPath = "&nbsp;";
				projectInfoBlock = projectInfoBlock.replace("${THUMBNAILSIZE}", QString("%1").arg(SVSettings::instance().m_thumbNailSize+8));
			}

			projectInfoBlock = projectInfoBlock.replace("${IMG_FILENAME}", thumbPath);
			templates += projectInfoBlock;
		}

		htmlPage.replace("${RECENT_PROJECT_FILELIST}", templates);
	}
	else {
		QString recentProjects = "<h1>" + tr("Examples/Validation cases") + "</h1><p>\n";

		// process all files in examples directory
		QDir examplesDir(QtExt::Directories::databasesDir() + "/examples");

		QStringList exampleFiles;
		SVSettings::recursiveSearch(examplesDir, exampleFiles, QStringList() << "vicus");

		// for each file, parse its header, extract comments, open thumbnail image and compose table with information
		foreach (QString fname, exampleFiles) {
			QString projectInfoBlock = RECENT_PROJECT_TABLE_TEMPLATE;
			// for examples we use a special link handler
			projectInfoBlock.replace("projectFile:$", "projectExample:$");
			VICUS::Project pro;
			QFileInfo finfo(fname);
			try {
				pro.readXML(IBK::Path(finfo.filePath().toStdString()));
			}
			catch (IBK::Exception &) {
				// error reading project file, missing permissions?
				pro = VICUS::Project(); // reset with empty instance
			}
			QString created;
			if (pro.m_nandradData.m_projectInfo.m_created.empty())
				created = "---";
			else
				created = QString::fromStdString(pro.m_nandradData.m_projectInfo.m_created);
			QString description = QString::fromStdString(pro.m_nandradData.m_projectInfo.m_comment);

			QString parentDirName = QFileInfo(finfo.dir().absolutePath()).fileName();
			projectInfoBlock = projectInfoBlock.replace("${PROJECT_FILENAME}", parentDirName + " - " + finfo.fileName());
			projectInfoBlock = projectInfoBlock.replace("${PROJECT_FULL_PATH}", finfo.filePath());
			projectInfoBlock = projectInfoBlock.replace("${PROJECT_DESCRIPTION}", description);

			// open thumbnail image for this project
			QString thumbPath = fname+".png";

			QFileInfo thumbFileInfo(thumbPath);
			QPixmap p;
			// check if file exists
			if (thumbFileInfo.exists() && p.load(thumbPath)) {
				/// \todo fix warning about bad resource loading
				thumbPath = "<a href=\"projectFile:${PROJECT_FULL_PATH}\"><img src=\"" + thumbFileInfo.absoluteFilePath() + "\"></a>&nbsp;";
				thumbPath = thumbPath.replace("${PROJECT_FULL_PATH}", finfo.filePath());
				projectInfoBlock = projectInfoBlock.replace("${THUMBNAILSIZE}", QString("%1").arg(p.width()+10));
			}
			else {
				thumbPath = "&nbsp;";
				projectInfoBlock = projectInfoBlock.replace("${THUMBNAILSIZE}", QString("%1").arg(SVSettings::instance().m_thumbNailSize+8));
			}

			projectInfoBlock = projectInfoBlock.replace("${IMG_FILENAME}", thumbPath);
			recentProjects += projectInfoBlock;
		}

		htmlPage.replace("${RECENT_PROJECT_FILELIST}", recentProjects);
	}

	// finally set welcome page in textbrowser
	m_ui->textBrowser->setHtml(htmlPage);
}


void SVWelcomeScreen::onAnchorClicked( const QUrl & link ) {
	// if anchor starts with "projectFile:" we emit the "open project" signal
	if (link.toString().startsWith("projectFile:")) {
		QString fname = link.toString();
		fname = fname.right(fname.length()-10);
		emit openProject(fname);
		return;
	}
	// if anchor starts with "projectExample:" we emit the "open example" signal
	else if (link.toString().startsWith("projectExample:")) {
		QString fname = link.toString();
		fname = fname.right(fname.length()-10);
		emit openExample(fname);
		return;
	}
	// if anchor starts with "projectTemplate:" we emit the "open template" signal
	else if (link.toString().startsWith("projectTemplate:")) {
		QString fname = link.toString();
		fname = fname.right(fname.length()-11);
		emit openTemplate(fname);
		return;
	}
	else if (link.toString().startsWith("projectRemove:")) {

		// extract project index to delete and remove it from list
		QString index = link.toString();
		index = index.right(index.length()-9);
		bool ok;
		int ind = index.toInt(&ok);
		if (ok && ind < SVSettings::instance().m_recentProjects.size()){
			SVSettings::instance().m_recentProjects.removeAt(ind);
		}

		// refresh view afterwards
		updateWelcomePage();

		// refresh menue structure
		emit updateRecentList();

		return;
	}
	else if (link.toString().startsWith("update:")) {
		emit softwareUpdateRequested();
		return;
	}
	else if (link.toString().startsWith("page:")) {
		QString page = link.toString().right(link.toString().length()-5);
		if (page == "recentFiles") {
			m_pageType = PT_RecentFiles;
		}
		else if (page == "demoFiles") {
			m_pageType = PT_DemoFiles;
		}
		else if (page == "templateFiles") {
			m_pageType = PT_TemplateFiles;
		}
		updateWelcomePage();
		return;
	}
	// must be an external link, call desktop services to open webbrowser
	QDesktopServices::openUrl(link);
}


void SVWelcomeScreen::on_toolButtonNewProject_clicked() {
	emit newProjectClicked();
}

void SVWelcomeScreen::on_toolButtonOpenProject_clicked() {
	emit openProjectClicked();
}

void SVWelcomeScreen::downloadFinished(QNetworkReply *reply) {
	if (reply->error()) {
		m_welcomePageNews = QString("<i>%1</i>")
							.arg(tr("unable to download news content."));
	}
	else {
		QByteArray newsRaw = reply->readAll();
		// extract text for current language
		QString news = QString::fromUtf8(newsRaw);
		bool languageFound = false;
		if (news.indexOf("[lang:") != -1) {
			QStringList langTexts = news.split("[lang:", QString::SkipEmptyParts);
			for (int i=0; i<langTexts.count(); ++i) {
				QString langId = langTexts[i].left(2);
				if (langId == QtExt::LanguageHandler::instance().langId()) {
					news = langTexts[i].right(langTexts[i].count()-3);
					languageFound = true;
					break;
				}
			}
			if (!languageFound && langTexts.count()>0) {
				// language not found, default to the first language (should be english)
				news = langTexts[0].right(langTexts[0].count()-3);
			}
		}

		// store news text
		m_welcomePageNews = news;
	}

	reply->deleteLater();
	// this will trigger a welcome page update (but no further download attempt)
	updateWelcomePage();
}


void SVWelcomeScreen::onUpdateInfoRetrieved(int res, QString newVersion) {
	// generate update info text, if update information was successfully retrieved
	// first set current version text
	if (res == 0) {
		// if we have a newer version, add html text with update link
		if (IBK::Version::lesserVersionNumber(VICUS::LONG_VERSION, newVersion.toStdString())) {
			m_updateInfoNews = tr("<h1><font style=\"color:#a02020\">An update is available!</font></h1><p>");
			m_updateInfoNews += tr("Current version: <b>%1</b><br>").arg(VICUS::LONG_VERSION);
			m_updateInfoNews += tr("New Version: <b><font style=\"color:#800000\">%1</font></b></p><p><a href=\"update:\">Download update</a>").arg(newVersion);
			m_updateInfoNews += "</p><hr>";
		}
	}
	updateWelcomePage();
}



const char * const HTML_TEMPLATE =
		"<html>\n"
		"<head>\n"
		"<!-- <link rel=stylesheet type=\"text/css\" href=\"welcome.css\"> -->\n"
		"<style type=\"text/css\">\n"
		"body     { font-size: medium; color: black; background-color: white }\n"
		"a        { color: #0053A6; text-decoration:none }\n"
		"a:hover  { color: #1C7DEF; background-color: white }\n"
		"p        { font-size: medium; text-align: justify; margin-top:0px; margin-bottom:8px;   }\n"
		"h1       { font-size: large; color: #003264; font-weight:bold; \n"
		"           text-decoration: none; margin-top:15px; margin-bottom:15px }\n"
		"h2       { font-size: medium; color: #0069A8; font-weight:bold; margin-top:15px; margin-bottom:6px }\n"
		"h3       { font-size: medium; color: #00660F; font-weight:bold; margin-top:10px; margin-bottom:2px}\n"
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
		"[ <a href=\"page:recentFiles\">${RECENT_PROJECTS_LINK}</a> ] [ <a href=\"page:demoFiles\">${DEMO_CASES_LINK}</a> ]"
		" [ <a href=\"page:templateFiles\">${TEMPLATES_LINK}</a> ]\n"
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


const char * const RECENT_PROJECT_TABLE_TEMPLATE =
		"<table border=\"0\" cellspacing=\"2\" cellpadding=\"0\">\n"
		"<tr valign=center><th width=\"${THUMBNAILSIZE}\" rowspan=\"3\">${IMG_FILENAME}</th><th align=left>${PROJECT_FILENAME}</th></tr>\n"
		"<tr valign=center><td align=left><a href=\"projectFile:${PROJECT_FULL_PATH}\">${PROJECT_FULL_PATH}</a></td></tr>\n"
		"<tr valign=top><td align=justify>${PROJECT_DESCRIPTION}</td></tr>\n"
		"</table>\n"
		"<br>\n"
		"\n";




