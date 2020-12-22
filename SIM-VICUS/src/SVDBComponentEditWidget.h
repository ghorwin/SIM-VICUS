#ifndef SVDBCOMPONENT_H
#define SVDBCOMPONENT_H

#include <QWidget>

namespace Ui {
class SVDBComponent;
}

class SVDBComponent : public QWidget
{
	Q_OBJECT

public:
	explicit SVDBComponent(QWidget *parent = nullptr);
	~SVDBComponent();

private:
	Ui::SVDBComponent *m_ui;
};

#endif // SVDBCOMPONENT_H
