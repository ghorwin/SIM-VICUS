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

#ifndef QtExt_FeatureWidgetH
#define QtExt_FeatureWidgetH

#include <QWidget>

class QSettings;
class QVBoxLayout;


namespace QtExt {

   class IconButton;
   class ActiveLabel;
   class FeatureWidgetGroup;


/*! This class is a widget container that contains a subwidget and a title bar with
	shade buttons and optionally plus and minus buttons.

	You can create a FeatureWidget as in the code shown below.
	\code
	// create a feature widget with a shade and a plus button
	FeatureWidget * w = new FeatureWidget(this, FeatureWidget::ShadeButton | FeatureWidget::PlusButton);
	\endcode
	The widget always creates all buttons internally, but only shows those that are indicated in the
	constructor, or set in the member function \a setVisibleButtons().

	The shade button toggles visibility of the content widget. The plus and minus
	buttons simply emit signals you can connect to.

	The content of the widget will be hidden or shown whenever the user presses
	the shade button to the top left. You can programmatically collapse/expand
	the widget by using the corresponding slots collapse() and expand().

	If several FeatureWidget instances are placed within the same layout, you can additionally
	use the exclusive property. For all feature widgets that have this property set, the expand()
	action of a feature widget will collapse all other feature widgets.
*/
class FeatureWidget : public QWidget {
	Q_OBJECT
public:
	/*! Buttons in the featurewidget, use in constructor or setVisibleButtons(). */
	enum ButtonTypes {
		ShadeButton		= 0x1, ///< The button to the top left to collapse/expand the widget.
		PlusButton		= 0x2, ///< The plus button, used to add things.
		MinusButton		= 0x4, ///< The minus-button, used to remove things.
		CopyButton		= 0x8, ///< The copy-button, used to create a copy of the currently selected item.
		RecycleButton	= 0x10, ///< The recycle-button, used to ....use as less as possible!!!
		MoveUpButton	= 0x20, ///< A button for moving things up
		MoveDownButton	= 0x40 ///< A button for moving things down
	};

	/*! Constructor, takes a parent widget and an or'd compination of the buttons. */
	explicit FeatureWidget(QWidget * parent, QWidget * content = NULL,
		int visibleButtons = ShadeButton | PlusButton | MinusButton );

	/*! destructor */
	~FeatureWidget();

	/*! Overwrite the enabled behavior for a feature widget and its contents. */
	void setEnabled(bool);

	/*! Replaces the current content widget with another. */
	void setContentWidget(QWidget * w);

	/*! Returns the current content widget. */
	QWidget * contentWidget() const;

	/*! Return access to the shade button. */
	IconButton * shadeButton() const;
	/*! Return access to the plus button. */
	IconButton * plusButton() const;
	/*! Return access to the minus button. */
	IconButton * minusButton() const;
	/*! Return access to the copy button. */
	IconButton * copyButton() const;
	/*! Return access to the copy button. */
	IconButton * recycleButton() const;
	/*! Return access to the move-up button. */
	IconButton * moveUpButton() const;
	/*! Return access to the move-down button. */
	IconButton * moveDownButton() const;

	/*! Returns the title of the widget. */
	QString title() const;
	/*! Sets the title of the widget. */
	void setTitle(const QString &);

	/*! Returns whether the widget is collapsed. */
	bool isCollapsed();

	/*! Sets the exclusive property. */
	void setExclusive(bool is_exclusive);
	/*! Returns whether the widget is exclusive. */
	bool isExclusive() const;

	/*! Stores the display and enabled states to the given settings.
		Default implementation just stores states of panels and corresponding buttons.
		Re-implement in each view to store view-specific settings.
		\warning Don't forget to call ViewBase::writeSettings() in reimplementations.
	*/
	virtual void writeSettings( QSettings& settings );

	/*! Restores the display and enabled states from the given settings.
		Default implementation just restores states of panels and corresponding buttons.
		Re-implement in each view to restore view-specific settings.
		\warning Don't forget to call ViewBase::readSettings() in reimplementations.
	*/
	virtual void readSettings( QSettings& settings );

signals:
	/*! Emitted when the user clicks on the shade button and hides the
		widget's content. */
	void collapsed( QtExt::FeatureWidget* );
	/*! Emitted when the user clicks on the shade button and shows the
		widget's content again. */
	void expanded( QtExt::FeatureWidget* );
	/*! Emitted when user clicks on the plus button. */
	void plusClicked();
	/*! Emitted when user clicks on the minus button. */
	void minusClicked();
	/*! Emitted when user clicks on the copy button. */
	void copyClicked();
	/*! Emitted when user clicks on the recycle button. */
	void recycleClicked();
	/*! Emitted when user clicks on the move-up button. */
	void moveUpClicked();
	/*! Emitted when user clicks on the move-down button. */
	void moveDownClicked();

public slots:
	/*! This slot will collapse the feature widget/hide the content of the widget. */
	void collapse();
	/*! This slot will expand the feature widget/show the content of thewidget.
		If the exclusive property is set, this slot will collapse all other
		feature widgets in the same layout which have the exclusive property set, too.
		\see isExclusive()
		\see setExclusive()
	*/
	void expand();

	/*! Triggered when the plus button was clicked, simply emits the plusClicked() signal. */
	void on_plusButton_clicked();
	/*! Triggered when the minus button was clicked, simply emits the minusClicked() signal. */
	void on_minusButton_clicked();
	/*! Triggered when the copy button was clicked, simply emits the copyClicked() signal. */
	void on_copyButton_clicked();
	/*! Triggered when the recycle button was clicked, simply emits the recycleClicked() signal. */
	void on_recycleButton_clicked();
	/*! Triggered when the move-up button was clicked, simply emits the moveUpClicked() signal. */
	void on_moveUpButton_clicked();
	/*! Triggered when the move-down button was clicked, simply emits the moveDownClicked() signal. */
	void on_moveDownButton_clicked();

private slots:
	/*! Triggered when user clicks on the shade button.
		Use collapse() and expand() to programmatically show/hide the widget's content!
		*/
	void on_shadeButton_clicked(bool);

private:
	/*! The main vertical layout that contains the title bar and the content widget. */
	QVBoxLayout			*m_vlay;

	/*! Set the visibility of the buttons. */
	void setVisibleButtons(int visibleButtons);


	/*! Pointer to the content widget.
		If no widget has been specified, a default widget is used.
	*/
	QWidget				*m_contentWidget;
	/*! Whether or not the feature widget is to be shown exclusively. */
	bool				m_exclusive;
	/*! Previous state of featureWidget */
	bool				m_previousShadeState;
	/*! The shade button. */
	IconButton		*m_shadeButton;
	/*! The plus button. */
	IconButton		*m_plusButton;
	/*! The minus button. */
	IconButton		*m_minusButton;
	/*! The copy button. */
	IconButton		*m_copyButton;
	/*! The recycle button. */
	IconButton		*m_recycleButton;
	/*! The move-up button. */
	IconButton		*m_moveUpButton;
	/*! The move-down button. */
	IconButton		*m_moveDownButton;

	/*! The title label.
		Clicking on the title label will trigger the shade button event.
	*/
	ActiveLabel	*m_titleLabel;
};

} // namespace QtExt {


#endif // QtExt_FeatureWidgetH
