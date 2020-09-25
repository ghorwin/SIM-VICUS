#ifndef SVWelcomeScreenH
#define SVWelcomeScreenH

#include <QWidget>
#include <QUrl>

#include <QtExt_AutoUpdater.h>

namespace Ui {
	class SVWelcomeScreen;
}

class QNetworkAccessManager;
class QNetworkReply;

/*! The welcome screen, showing news and recently projects. */
class SVWelcomeScreen : public QWidget {
	Q_OBJECT

public:

	enum PageType {
		PT_RecentFiles,
		PT_DemoFiles,
		PT_TemplateFiles
	};

	explicit SVWelcomeScreen(QWidget *parent = nullptr);
	~SVWelcomeScreen();

	/*! Updates the welcome page content. */
	void updateWelcomePage();

public slots:
	/*! Triggered when user clicks on a project file or external link. */
	void onAnchorClicked( const QUrl & link );

signals:
	/*! Emitted when user clicked on the file name of a recently used file. */
	void openProject(const QString & projectFile);
	/*! Emitted when user clicked on the file name of an example (read-only location). */
	void openExample(const QString & projectFile);
	/*! Emitted when user clicked on the file name of a template (read-only location). */
	void openTemplate(const QString & projectFile);

	void newProjectClicked();
	void openProjectClicked();

	/*! Emitted, when user has removed a project from the recent files list. */
	void updateRecentList();
	/*! Emitted, when user clicks on "Download update" link. */
	void softwareUpdateRequested();

private slots:
	void on_toolButtonNewProject_clicked();
	void on_toolButtonOpenProject_clicked();
	void downloadFinished(QNetworkReply *reply);

	/*! Triggered by update manager. */
	void onUpdateInfoRetrieved(int, QString);

private:
	Ui::SVWelcomeScreen	*m_ui;

	QNetworkAccessManager	*m_networkManager;

	/*! Contains information about current and available newer version (content is set in onUpdateInfoRetrieved()). */
	QString					m_updateInfoNews;
	/*! Holds the news content for the welcome page. */
	QString					m_welcomePageNews;

	QtExt::AutoUpdater		*m_autoUpdater;

	/*! Indicates which page should be shown. */
	PageType				m_pageType;
};

#endif // SVWelcomeScreenH
