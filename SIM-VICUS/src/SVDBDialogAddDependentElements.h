#ifndef SVDBDIALOGADDDEPENDENTELEMENTSH
#define SVDBDIALOGADDDEPENDENTELEMENTSH

#include <QDialog>
#include <set>

namespace VICUS {
	class AbstractDBElement;
}


namespace Ui {
class SVDBDialogAddDependentElements;
}

class SVDBDialogAddDependentElements : public QDialog
{
	Q_OBJECT

public:
	explicit SVDBDialogAddDependentElements(QWidget *parent = nullptr);
	~SVDBDialogAddDependentElements();

	void setup(const QString & infoText, const std::set<VICUS::AbstractDBElement *> &elements);

private:
	Ui::SVDBDialogAddDependentElements *m_ui;
};

#endif // SVDBDIALOGADDDEPENDENTELEMENTS_H
