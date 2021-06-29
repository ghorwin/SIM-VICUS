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

#include <IBK_messages.h>
#include <IBK_Version.h>

#include "QtExt_AutoUpdateDialog.h"

namespace QtExt {

AutoUpdater::AutoUpdater(QObject *parent) :
	QObject(parent),
	m_networkManager(new QNetworkAccessManager)
{
	connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(downloadFinished(QNetworkReply*)));
}


AutoUpdater::~AutoUpdater() {
	delete m_networkManager;
}


bool AutoUpdater::installUpdateWhenAvailable(const QString & localPath) {
	if (!QFileInfo::exists(localPath) )
		return false;

	// automatic call of installer only on Windows OS
#if defined(Q_OS_WIN)
	// check if we have started this installation already
	if (QFileInfo::exists(localPath + ".run")) {
		IBK::IBK_Message(IBK::FormatString("Update appears to be running already. Attempting to remove file '%1'.\n")
						 .arg((localPath + ".run").toStdString()), IBK::MSG_WARNING);
		// delete update file and info file
		QFile(localPath + ".run").remove();
		QFile(localPath).remove();
		return false;
	}

	// remember that we have already started this update
	QFile updateInfo(localPath + ".run");
	updateInfo.open(QFile::WriteOnly);
	QTextStream strm(&updateInfo);
	strm << QDateTime().currentDateTime().toString();

	bool res = QProcess::startDetached(localPath);
	if (!res) {
		IBK::IBK_Message("Error installing update.", IBK::MSG_ERROR);
		QFile(localPath).remove();
		QFile(localPath +".run").remove();
		return false;
	}
#endif
	return true;
}


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
	QWidget * w = dynamic_cast<QWidget*>(parent());
	if (reply->error()) {
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
						showUpdateDialog(w, m_currentVersion, m_newVersion, m_changeLogText, m_downloadUrl, m_downloadFilePath, rejected);
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
								   QString changeLogText, QString downloadUrl, QString downloadFilePath, bool & rejected)
{
	// we expect m_newVersion, m_currentVersion and m_changeLogText to be populated correctly
	AutoUpdateDialog dlg(parent);
	dlg.run(currentVersion, newVersion, changeLogText, downloadUrl, downloadFilePath);
	rejected = dlg.m_updateRejected;
}


} // namespace QtExt
