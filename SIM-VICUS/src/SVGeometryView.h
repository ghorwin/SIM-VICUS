#ifndef SVGEOMETRYVIEW_H
#define SVGEOMETRYVIEW_H

#include <QWidget>

namespace Vic3D {
	class SceneView;
}

/*! The main geometry view.
	The 3D scene view is embedded into this widget.
*/
class SVGeometryView : public QWidget {
	Q_OBJECT
public:
	explicit SVGeometryView(QWidget *parent = nullptr);

	void saveScreenShot(const QString & imgFilePath);

	/*! Provides read-only access to sceneView() so that signals can be connected. */
	const Vic3D::SceneView * sceneView() const { return m_sceneView; }

private:
	// The scene view, that shows our world and allows navigation
	Vic3D::SceneView * m_sceneView;

};

#endif // SVGEOMETRYVIEW_H
