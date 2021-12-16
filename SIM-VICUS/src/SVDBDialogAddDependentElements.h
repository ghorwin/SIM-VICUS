#ifndef SVDBDialogAddDependentElementsH
#define SVDBDialogAddDependentElementsH

#include <QDialog>
#include <set>

namespace VICUS {
	class AbstractDBElement;
}


namespace Ui {
	class SVDBDialogAddDependentElements;
}

/*! Dialog shows list of referenced local elements and asks user to confirm importing these
	elements into the user DB.
*/
class SVDBDialogAddDependentElements : public QDialog {
	Q_OBJECT

public:
	explicit SVDBDialogAddDependentElements(QWidget *parent = nullptr);
	~SVDBDialogAddDependentElements();

	/*! Populates list widget with names/types of reference elements. */
	void setup(const std::set<VICUS::AbstractDBElement *> &elements);

private:
	Ui::SVDBDialogAddDependentElements *m_ui;
};

#endif // SVDBDialogAddDependentElementsH
