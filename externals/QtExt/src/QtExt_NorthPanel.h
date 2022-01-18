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

#ifndef QtExt_NorthPanelH
#define QtExt_NorthPanelH

#include <QWidget>

#include <QMutex>
#include <QPixmap>


class QHBoxLayout;
class QButtonGroup;
class QLabel;
class QProgressBar;

namespace QtExt {
	class IconButton;
}

namespace QtExt {

	class PushButton;


class NorthPanel : public QWidget {
	Q_OBJECT
public:
	explicit NorthPanel( QWidget * parent = 0 );
	~NorthPanel();

	/*! Adds a view button to the horizontal layout. */
	void addViewButton( PushButton * btn );

	/*! Turns visibility or the progress widget on/off. */
	void setProgressVisible(bool visible);

	/*! Return access to the abort button. */
	QtExt::IconButton * abortButton() const;

	/*! Sets the logo of the user.
		\param userLogo pixmap that keeps the logo. */
	void setLogo( const QPixmap &userLogo  );

signals:

	/*! Emitted when user clicks on the abort button. */
	void abortClicked();



public slots:

	/*! Sets the progress widget caption. */
	void setProgressCaption(const QString & msg);

	/*! Sets the progress bar range and message.
		\param total		The total number of items to process. This value will
							be used as max value of the progress bar with the special
							case of 0, which means the progress bar is to be left untouched.
		\param current		The current item being processed, must not be larger than totalItems.
							Special case: current = -1 means we are done and can hide the
							progress bar again. This also allows "killing" the progress bar
							when an error occurred before current = total.
		\param msg			An optional message string to be displayed.
		*/
	void setProgressInfo(int total, int current, QString msg);



private slots:

	/*! Triggered when user clicks on "Abort task" button. */
	void on_progressAbortButton_clicked();


private:

	/*! Layout for buttons. */
	QHBoxLayout		*m_buttonLayout;

	/*! The widget containing all the progress stuff.
		The progress-related controls are all wrapped in
		the widget to allow for easy hiding/showing and
		better layout control. */
	QWidget			*m_progressWidget;

	/*! Label for the progress widget. */
//	QLabel			*m_progressCaption;

	/*! Label for the progress message. */
//	QLabel			*m_progressMessage;

	/*! The progress bar. */
	QProgressBar	*m_progressBar;

	/*! The abort button for longterm progress things */
	QtExt::IconButton		*m_progressAbortButton;

	/*! logo state guard */
	bool			m_bLogo;

	/*! The label that contains the logo. */
	QLabel			*m_labelLogo;
};


} // namespace QtExt

#endif // QtExt_NorthPanelH
