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

#ifndef QtExt_ConstructionViewWidgetH
#define QtExt_ConstructionViewWidgetH

#include <QWidget>
#include <QToolBar>

#include <QtExt_ConstructionLayer.h>
#include <QtExt_ConstructionGraphicsScene.h>

namespace QtExt {

namespace Ui {
class ConstructionViewWidget;
}

/*! Editor for 1D constructions. */
class ConstructionViewWidget : public QWidget {
	Q_OBJECT

public:
	explicit ConstructionViewWidget(QWidget *parent = nullptr);

	~ConstructionViewWidget();

	/*! Set the construction.
		\param layers Vector with construction layers
		\param fixed If true construction cannot be changed
	*/
	void setData(const QVector<ConstructionLayer>& layers, bool fixed,
				 QString	leftSideLabel = tr("Outside"),
				 QString	rightSideLabel = tr("Inside"),
				 int visibleItems = ConstructionGraphicsScene::VI_All );

	/*! Update the view with the current settings.
		Must be called after changing background color or layer mark.
	*/
	void updateView();

	/*! Set visibility of toolbar widget.*/
	void setToolbarVisible(bool visible);

	/*! Set the background color for calculating font and line colors.
		The background color itself will not be changed.
		Call it before setData().
	*/
	void setBackground(const QColor& bkgColor);

	/*! Mark a layer with a hatching.
		\param LayerIndex Index of layer to be marked starting with 0. Set -1 to unmark the construction.*/
	void markLayer(int layerIndex);

	/*! Clear all content and empty the widget.*/
	void clear();

	/*! Enable toolbar and actions.*/
	void enableToolBar(bool enable);

public slots:
	/*! Set selection for the layer given by index.*/
	void selectLayer(int index);

signals:
	/*! Layer index is selected.*/
	void layerSelected(int index);
	/*! Toolbutton 'assign material' is triggered while layer index is selected.*/
	void assignMaterial(int index);
	/*! Toolbutton 'insert layer' is triggered while layer index is selcted. If left ist true inserting on left side.*/
	void insertLayer(int index, bool left);
	/*! Move layer with index to left or right side.*/
	void moveLayer(int index, bool left);
	/*! Remove layer index.*/
	void removelayer(int index);

private slots:
	void on_actionAssign_material_triggered();

	void on_actionInsert_layer_left_triggered();

	void on_actionInsert_layer_right_triggered();

	void on_actionRemove_layer_triggered();

	void on_actionMove_layer_left_triggered();

	void on_actionMove_layer_right_triggered();

	void onLayerSelected(int);

	void onLayerDoubleClicked(int);

private:
	Ui::ConstructionViewWidget *ui;
	QToolBar*					m_toolBar;
	bool						m_fixed;

	void setupUI();
};


} // namespace QtExt
#endif // QtExt_ConstructionViewWidgetH
