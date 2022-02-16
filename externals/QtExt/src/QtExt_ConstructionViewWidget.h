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

	/*! Set visibility of toolbar widget.*/
	void setToolbarVisible(bool visible);

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
