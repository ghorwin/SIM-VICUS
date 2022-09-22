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

#ifndef QtExt_ConstructionViewH
#define QtExt_ConstructionViewH

#include <QObject>
#include <QGraphicsView>
#include <QVector>
#include <QPair>
#include <QString>
#include <QPrinter>
#include <QResizeEvent>

#include "QtExt_ConstructionGraphicsScene.h"
#include "QtExt_ConstructionLayer.h"

namespace QtExt {

/*! The view that shows the 1D construction model. */
class ConstructionView : public QGraphicsView {
	Q_OBJECT
public:
	/*! Pre-defined colors for the individual layers. */
	static const QColor ColorList[12];

	/*! Default constructor.
		\param parent Parent widget.
	*/
	explicit ConstructionView(QWidget *parent = 0);

	~ConstructionView();

	/*! Updates the locally cached data.*/
	void setData(QPaintDevice* paintDevice, const QVector<ConstructionLayer>& layers, double resolution, int visibleItems);

	/*! Clear all line markers.*/
	void clearLineMarkers();

	/*! Add a line marker to the list.
		\param pos Position in construction starting at left or bottom.
		\param pen Pen used for drawing line
	*/
	void addLinemarker(double pos, const QPen& pen, const QString& name);

	/*! Set a marked area.*/
	void setAreaMarker(const ConstructionGraphicsScene::AreaMarker& am);

	/*! Remove a existing area marker.*/
	void removeAreaMarker();

	/*! Update the view with the current settings.
		Must be called after changing background color or layer mark.
	*/
	void updateView();

	/*! Set the background color for calculating font and line colors.
		The background color itself will not be changed.
	*/
	void setBackground(const QColor& bkgColor);

	/*! Mark a layer with a hatching.
		\param LayerIndex Index of layer to be marked starting with 0. Set -1 to unmark the construction.*/
	void markLayer(int layerIndex);

	/*! Clears content and scene.*/
	void clear();

	/*! Creates a svg image and paint it to the outputDevice.*/
	void createSvg(QIODevice * outputDevice);

	/*! Creates a pixmap.*/
	QPixmap createPixmap();

	/*! Write to the given printer.*/
	void print(QPrinter* printer);

	/*! Deselect all selected items.*/
	void deselectItems();

	/*! Return selected layer.*/
	int selectedLayer() const { return m_selectedLayer; }

	/*! Return the current layer set.*/
	const QVector<ConstructionLayer>& layers() const {
		return m_inputData;
	}

	QString	m_leftSideLabel;
	QString	m_rightSideLabel;

signals:
	/*! Signal that contains selected layer or -1 if no layer is selected.*/
	void layerSelected(int index);
	/*! Signal that contains selected layer or -1 if no layer is selected in case of double click.*/
	void layerDoubleClicked(int index);

public slots:

	void selectLayer(int index);

protected:
	/*! Is called if a resize is necessary.
		Set new size for the diagramScene object.
	*/
	virtual void resizeEvent ( QResizeEvent * event);

	virtual void mousePressEvent(QMouseEvent *event);

	virtual void mouseDoubleClickEvent(QMouseEvent *event);

	/*! New paint event can draw items independently from scene.*/
	void paintEvent ( QPaintEvent * event );

private slots:
	/*! Connected to graphicsscene selectionChanged signal.*/
	void sceneSelectionChanged();
	/*! Connected to graphicsscene doubleClick signal.*/
	void sceneDoubleClicked();

private:
	QPaintDevice*		m_device;					///< Paintdevice.

	/*! Local copy of all input data to be used by the drawing code.
		We store a local copy of the data so that graphics updates can be
		done individually from outer code.
	*/
	QVector<ConstructionLayer> m_inputData;

	/*! Paints the diagram.*/
	ConstructionGraphicsScene*	m_diagramScene;

	/*! Margins between view and scene.*/
	int				m_margins;

	/*! Resolution of page (pixel per millimeter).*/
	double			m_resolution;

	/*! Currently selected layer. Is -1 if nothing is selected or no layer exist.*/
	int				m_selectedLayer;

	/*! Store the item visibilty for the graphics scene.
		See QtExt::ConstructionGraphicsScene for more information.
	*/
	int				m_visibleItems;
};

} // namespace QtExt

#endif // QtExt_ConstructionViewH
