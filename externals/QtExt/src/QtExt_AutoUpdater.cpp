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

#include "QtExt_AutoUpdater.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QFileInfo>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QProcess>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QLabel>
#include <QDesktopWidget>
#include <QApplication>
#include <QDir>

#include <IBK_messages.h>
#include <IBK_Version.h>
#include <IBK_Exception.h>
#include <IBK_crypt.h>

#include "QtExt_AutoUpdateDialog.h"
#include "QtExt_Directories.h"

namespace QtExt {

AutoUpdater::AutoUpdater(QObject *parent) :
	QObject(parent),
	m_networkManager(new QNetworkAccessManager)
{
	connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(downloadFinished(QNetworkReply*)));
}


#if defined(Q_OS_WIN)
bool AutoUpdater::installUpdateWhenAvailable(const QString & localPath, const std::string & md5hash) {
	// this is only done for Windows
	if (!QFileInfo::exists(localPath) )
		return false;

	// check if there is an exe file in here
	QDir updateDir(localPath); // example: "C:\Users\xxx\AppData\Local\PostProcApp\updates"
	QStringList updaterExecutables = updateDir.entryList(QStringList() << "*.exe",QDir::Files);
	if (updaterExecutables.isEmpty())
		return false;

	// generate path to installer file and remove the file if existing already
	QString tmpDir = QtExt::Directories::tmpDir(); // example: "C:/Users/xxx/AppData/Local/Temp"
	if (!QDir().exists(tmpDir))
		QDir().mkpath(tmpDir);


	// process all files in updates directory
	QString installerFile;
	for (QString updateExecutable : updaterExecutables) {

		// example: updaterExecutables[0] = "PostProc_2.2.2_win64_2020-05-27.exe"

		QString tmpInstallerFile = QDir(tmpDir).absoluteFilePath(updateExecutable);
		// example: tmpInstallerFile = "C:/Users/xxxx/AppData/Local/Temp/PostProc_2.2.2_win64_2020-05-27.exe"
		QFile::remove(tmpInstallerFile); // remove if already existing (from previous update)

		// absolute path to downloaded file
		updateExecutable = updateDir.absoluteFilePath(updateExecutable);
		// example: updateExecutable = "C:/Users/xxx/AppData/Local/PostProcApp/updates/PostProc_2.2.2_win64_2020-05-27.exe"

		// We move the first executable to the system wide temporary directory and execute it there.
		QFile::rename(updateExecutable, tmpInstallerFile);

		// if we have a md5hash, check if contents match, otherwise ignore update
		if (!md5hash.empty()) {
			QFile f(tmpInstallerFile);
			f.open(QIODevice::ReadOnly);
			// this might be several Megabytes (..100Mb), but that isn't an issue with todays memory availability, right?
			QByteArray downloadedData;
			downloadedData = f.readAll();
			// compute MD5 hash
			std::vector<unsigned char> data(downloadedData.data(), downloadedData.data() + downloadedData.size());
			std::string updateFileMD5 = IBK::md5_str(data);
			if (updateFileMD5 == md5hash)
				installerFile = tmpInstallerFile; // found the correct one
		}
		else {
			// just take the last executable that was moved
			installerFile = tmpInstallerFile;
		}
	}
	if (!installerFile.isEmpty()) {
		// automatic call of installer only on Windows OS
		bool res = QProcess::startDetached(installerFile);
		if (!res) {
			IBK::IBK_Message("Error installing update.", IBK::MSG_ERROR);
			return false;
		}
	}
	else {
		IBK::IBK_Message("Mismatching md5 hash of installer file.", IBK::MSG_ERROR);
		return false;
	}
	return true;
}
#endif


void AutoUpdater::checkForUpdateInfo(const QString & url, const char * const LONG_VERSION, bool interactive, QString downloadFilePath,
									 QString newestRejectedVersion)
{
	m_currentVersion = LONG_VERSION;
	m_interactive = interactive;
	m_downloadFilePath = downloadFilePath;
	m_newestRejectedVersion = newestRejectedVersion;

	QUrl addy(url);
	QString fullPath = addy.toString();
	int pos = fullPath.lastIndexOf('/');
	m_updateInfoDownloadPath = fullPath.left(pos);

	QNetworkRequest request(addy);
	m_networkManager->get(request);
}


void AutoUpdater::downloadFinished(QNetworkReply *reply) {
	FUNCID(AutoUpdater::downloadFinished);
	QWidget * w = dynamic_cast<QWidget*>(parent());
	if (reply->error()) {
		// Note: when QNetworkReply::UnknownNetworkError occurs on Windows while downloading from https,
		//       it is likely that the libraries libeay32.dll and ssleay32.dll are missing in exe directory
		std::string errormsg;
		switch (reply->error()) {
			case QNetworkReply::NoError: break;
			case QNetworkReply::ConnectionRefusedError: errormsg = "ConnectionRefusedError"; break;
			case QNetworkReply::RemoteHostClosedError: errormsg = "RemoteHostClosedError"; break;
			case QNetworkReply::HostNotFoundError: errormsg = "HostNotFoundError"; break;
			case QNetworkReply::TimeoutError: errormsg = "TimeoutError"; break;
			case QNetworkReply::OperationCanceledError: errormsg = "OperationCanceledError"; break;
			case QNetworkReply::SslHandshakeFailedError: errormsg = "SslHandshakeFailedError"; break;
			case QNetworkReply::TemporaryNetworkFailureError: errormsg = "TemporaryNetworkFailureError"; break;
			case QNetworkReply::NetworkSessionFailedError: errormsg = "NetworkSessionFailedError"; break;
			case QNetworkReply::BackgroundRequestNotAllowedError: errormsg = "BackgroundRequestNotAllowedError"; break;
			case QNetworkReply::TooManyRedirectsError: errormsg = "TooManyRedirectsError"; break;
			case QNetworkReply::InsecureRedirectError: errormsg = "InsecureRedirectError"; break;
			case QNetworkReply::UnknownNetworkError: errormsg = "UnknownNetworkError"; break;
			case QNetworkReply::ProxyConnectionRefusedError: errormsg = "ProxyConnectionRefusedError"; break;
			case QNetworkReply::ProxyConnectionClosedError: errormsg = "ProxyConnectionClosedError"; break;
			case QNetworkReply::ProxyNotFoundError: errormsg = "ProxyNotFoundError"; break;
			case QNetworkReply::ProxyTimeoutError: errormsg = "ProxyTimeoutError"; break;
			case QNetworkReply::ProxyAuthenticationRequiredError: errormsg = "ProxyAuthenticationRequiredError"; break;
			case QNetworkReply::UnknownProxyError: errormsg = "UnknownProxyError"; break;
			case QNetworkReply::ContentAccessDenied: errormsg = "ContentAccessDenied"; break;
			case QNetworkReply::ContentOperationNotPermittedError: errormsg = "ContentOperationNotPermittedError"; break;
			case QNetworkReply::ContentNotFoundError: errormsg = "ContentNotFoundError"; break;
			case QNetworkReply::AuthenticationRequiredError: errormsg = "AuthenticationRequiredError"; break;
			case QNetworkReply::ContentReSendError: errormsg = "ContentReSendError"; break;
			case QNetworkReply::ContentConflictError: errormsg = "ContentConflictError"; break;
			case QNetworkReply::ContentGoneError: errormsg = "ContentGoneError"; break;
			case QNetworkReply::UnknownContentError: errormsg = "UnknownContentError"; break;
			case QNetworkReply::ProtocolUnknownError: errormsg = "ProtocolUnknownError"; break;
			case QNetworkReply::ProtocolInvalidOperationError: errormsg = "ProtocolInvalidOperationError"; break;
			case QNetworkReply::ProtocolFailure: errormsg = "ProtocolFailure"; break;
			case QNetworkReply::InternalServerError: errormsg = "InternalServerError"; break;
			case QNetworkReply::OperationNotImplementedError: errormsg = "OperationNotImplementedError"; break;
			case QNetworkReply::ServiceUnavailableError: errormsg = "ServiceUnavailableError"; break;
			case QNetworkReply::UnknownServerError: errormsg = "UnknownServerError"; break;
		}
		IBK::IBK_Message(IBK::FormatString("Network error: %1").arg(errormsg), IBK::MSG_ERROR, FUNC_ID);
		if (m_interactive)
			QMessageBox::critical(w, tr("Connection error"), tr("Could not retrieve update information."));
		else {
			emit updateInfoRetrieved(2, QString());
		}
	}
	else {
		QByteArray newsRaw = reply->readAll();
		// extract text
		QMap<QString, QString>	header;
		QString news = QString::fromUtf8(newsRaw);
		int headerEndPos = news.indexOf("---HeaderEnd---");
		if (headerEndPos != -1) {
			m_changeLogText = news.mid(headerEndPos + 15);
			news = news.left(headerEndPos);
		}
		QStringList lines = news.split('\n');
		for (int i=0; i<lines.size(); ++i) {
			QString  line = lines[i].trimmed();
			QStringList tokens = line.split(":");
			if (tokens.size() == 2)
				header[tokens[0]] = tokens[1];
		}

		// find matching file version
#ifdef _WIN32
#ifdef _WIN64
		QString updateFile = header["win64"];
#else // _WIN64
		QString updateFile = header["win"];
#endif
#else
#ifdef Q_OS_MAC
		QString updateFile = header["mac"];
#else // Q_OS_MAC
		QString updateFile = header["linux"];
#endif
#endif
		// extract version number
		QStringList tokens = updateFile.split("_");
		if (tokens.count() < 2)
			updateFile.clear(); // malformed filename - same as no file
		else {
			m_newVersion = tokens[1];
		}
		if (updateFile.isEmpty()) {
			if (m_interactive)
				showChangelogInfo(w);
			else {
				emit updateInfoRetrieved(1, QString());
			}
		}
		else {
			m_downloadUrl = m_updateInfoDownloadPath + "/" + updateFile;
			if (m_interactive) {
				// only show dialog if a newer version is available
				if (IBK::Version::lesserVersionNumber(m_currentVersion.toStdString(), m_newVersion.toStdString())) {
					// when the new version was already skipped by the user, do not show anything
					QString relevantVersion;
					if (!m_newestRejectedVersion.isEmpty() &&
						IBK::Version::lesserVersionNumber(m_currentVersion.toStdString(), m_newestRejectedVersion.toStdString()))
					{
						relevantVersion = m_newestRejectedVersion;
					}
					else
						relevantVersion = m_currentVersion;

					if (IBK::Version::lesserVersionNumber(relevantVersion.toStdString(), m_newVersion.toStdString())) {
						bool rejected;
						showUpdateDialog(w, m_currentVersion, m_newVersion, m_changeLogText, m_downloadUrl, m_downloadFilePath, m_downloadedFileMD5, rejected);
						if (rejected)
							emit updateRejected(m_newVersion);
					}
				}
				else
					showChangelogInfo(w);
			}
			else {
				emit updateInfoRetrieved(0, m_newVersion);
			}
		}
	}

	reply->deleteLater();
}


void AutoUpdater::showChangelogInfo(QWidget * w) {
	QDialog dlg(w, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	dlg.setWindowTitle(tr("No update available"));
	QVBoxLayout * lay = new QVBoxLayout(w);
	QLabel * label = new QLabel(tr("There is currently no update available for this software and platform/OS."));
	lay->addWidget(label);
	QPlainTextEdit * edit = new QPlainTextEdit(w);
	edit->setReadOnly(true);
	QFont f;
#if defined(Q_OS_MAC) // Q_OS_UNIX
	f.setFamily("Monaco");
#elif defined(Q_OS_UNIX)
	f.setFamily("Monospace");
	f.setPointSize(8); // smaller point size for changelog
#else
	f.setFamily("Consolas");
#endif
	edit->setFont(f);
	edit->setWordWrapMode( QTextOption::NoWrap );
	lay->addWidget(edit);
	QString changes = m_changeLogText;
	int pos = changes.indexOf("CHANGES");
	if (pos != -1)
		changes = changes.right(changes.length() - pos);
	edit->setPlainText(changes);
	dlg.setLayout(lay);
	QRect rec = QApplication::desktop()->screenGeometry();
	int height = rec.height();
	int width = rec.width();
	dlg.resize(QSize(width*0.6, height*0.8));
	dlg.exec();
}


void AutoUpdater::showUpdateDialog(QWidget * parent, QString currentVersion, QString newVersion,
								   QString changeLogText, QString downloadUrl, QString downloadFilePath,
								   std::string & downloadedFileMD5, bool & rejected)
{
	// we expect m_newVersion, m_currentVersion and m_changeLogText to be populated correctly
	AutoUpdateDialog dlg(parent);
	dlg.run(currentVersion, newVersion, changeLogText, downloadUrl, downloadFilePath);
	rejected = dlg.m_updateRejected;
	downloadedFileMD5 = dlg.m_updateFileMD5;
		// remember file size of downloaded file... can be used to check that nobody
		// has tempered with the files. Normally, a hash code would
}


} // namespace QtExt
