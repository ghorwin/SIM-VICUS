#ifndef SVIMPORTDXFDIALOGH
#define SVIMPORTDXFDIALOGH

#include <QDialog>

#include <VICUS_Drawing.h>

namespace Ui {
class SVImportDxfDialog;
}

class SVImportDxfDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVImportDxfDialog(QWidget *parent = nullptr);
	~SVImportDxfDialog();

private:

	void run();

	void readDxfFile();

	Ui::SVImportDxfDialog		*m_ui;

	QString						m_lastFilePath;

	VICUS::Drawing				m_drawing;

};

#endif // SVIMPORTDXFDIALOGH
