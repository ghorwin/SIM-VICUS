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

#ifndef QtExt_AutoUpdateDialogH
#define QtExt_AutoUpdateDialogH

#include <QDialog>

namespace Ui {
	class AutoUpdateDialog;
}

class QNetworkAccessManager;
class QNetworkReply;

namespace QtExt {

/*! This is the dialog that shows the update information and manages the download
	of the new software version.

	This dialog is created and shown from AutoUpdater and should normally not be
	used directly.
*/
class AutoUpdateDialog : public QDialog {
	Q_OBJECT
public:
	explicit AutoUpdateDialog(QWidget *parent = 0);
	~AutoUpdateDialog();

	/*! Call this function instead of exec(). */
	int run(const QString & currentVersion, const QString & newVersion, const QString & changeLogText,
			const QString & downloadUrl, const QString & localFilePath);


	/*! Set to true, if user rejects update. */
	bool m_updateRejected;

	/*! Here the MD5 hash of the downloaded update file is stored. */
	std::string m_updateFileMD5;

private slots:

	void on_pushButtonDownload_clicked();

	/*! Connected to network reply to update progress bar. */
	void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

	/*! Triggered when download has completed. */
	void onDownloadFinished(QNetworkReply*);

	void on_pushButtonSkipUpdate_clicked();

private:

	Ui::AutoUpdateDialog	*m_ui;

	/*! Download url for update installer, set in run(). */
	QString					m_downloadUrl;
	/*! Path where downloaded file shall be stored in, set in run(). */
	QString					m_localFilePath;


	/*! Network manager, used to execute network requests. */
	QNetworkAccessManager	*m_networkManager;
};

} // namespace QtExt

#endif // QtExt_AutoUpdateDialogH
