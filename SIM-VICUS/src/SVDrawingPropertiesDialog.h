#ifndef SVDrawingPropertiesDialogH
#define SVDrawingPropertiesDialogH

#include <VICUS_Drawing.h>
#include <QDialog>

namespace Ui {
class SVDrawingPropertiesDialog;
}

class SVDrawingPropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVDrawingPropertiesDialog(QWidget *parent = nullptr);
	~SVDrawingPropertiesDialog();

	static bool showDrawingProperties(QWidget *parent, VICUS::Drawing *drawing);

private:
	Ui::SVDrawingPropertiesDialog *m_ui;
};

#endif // SVDrawingPropertiesDialogH
