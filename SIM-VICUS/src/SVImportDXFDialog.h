#ifndef SVIMPORTDXFDIALOGH
#define SVIMPORTDXFDIALOGH

#include <VICUS_Drawing.h>
#include <VICUS_Project.h>
#include <QDialog>

#include <IBKMK_Vector2D.h>
#include <IBKMK_Vector3D.h>
#include <IBK_Line.h>

#include <libdxfrw.h>
#include <drw_interface.h>
#include <drw_objects.h>
#include <drw_base.h>

namespace Ui {
class SVImportDXFDialog;
}

class SVImportDXFDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVImportDXFDialog(QWidget *parent = nullptr);

	enum ImportResults {
		AddDrawings,
		ImportCancelled
	};

	enum ScaleUnit {
		SU_Auto,
		SU_Meter,
		SU_Decimeter,
		SU_Centimeter,
		SU_Millimeter,
		NUM_SU
	};

	ImportResults import(const QString & fname);

	~SVImportDXFDialog();

	const VICUS::Drawing &drawing() const;

private slots:
	void on_comboBoxUnit_activated(int index);

	void on_pushButtonConvert_clicked();

	void on_pushButtonImport_clicked();

private:
	/*! Read a specified dxf file.
		\param drawing VICUS Drawing, where all primitives are added
		\param fname Filename that will be read
		\returns true if reading has been successful
	*/
	bool readDxfFile(VICUS::Drawing & drawing, const QString &fname);

	/*! Moves drawings to global coordinate center. */
	void moveDrawings();

	/*! Fix too big fonts. */
	void fixFonts();

	/*! Pointer to UI. */
	Ui::SVImportDXFDialog		*m_ui;

	/*! Last file path. */
	QString						m_filePath;

	/*! VICUS Drawing with all drawing primitives. */
	VICUS::Drawing				m_drawing;

	/*! Next VICUS project ID. */
	unsigned int				m_nextId;

	/*! Center point of drawing. Used in moveDrawings()
		to center the painting. */
	IBKMK::Vector3D				m_center;

	/*! Return code. */
	ImportResults				m_returnCode;

};

/* Implementation of DRW_Interface. A dxf file will be read from top to bottom,
 * every time an object is found, one of the following methods are called.
The objects are then inserted into drawing */

class DRW_InterfaceImpl : public DRW_Interface {

	VICUS::Drawing				*m_drawing		= nullptr;

	VICUS::DrawingLayer::Block	*m_activeBlock	= nullptr;

	unsigned int				*m_nextId		= nullptr;

public :

	/*! C'tor */
	DRW_InterfaceImpl(VICUS::Drawing *drawing, unsigned int &nextId);

	/** Called when header is parsed.  */
	void addHeader(const DRW_Header* data) override;

	/** Called for every line Type.  */
	void addLType(const DRW_LType& data) override;

	/** Called for every layer. */
	void addLayer(const DRW_Layer& data) override;

	/** Called for every dim style. */
	void addDimStyle(const DRW_Dimstyle& data) override;

	/** Called for every VPORT table. */
	void addVport(const DRW_Vport& data) override;

	/** Called for every text style. */
	void addTextStyle(const DRW_Textstyle& data) override;

	/** Called for every AppId entry. */
	void addAppId(const DRW_AppId& data) override;

	/**
	 * Called for every block. Note: all entities added after this
	 * command go into this block until endBlock() is called.
	 *
	 * @see endBlock()
	 */
	void addBlock(const DRW_Block& data) override;

	/**
	 * In DWG called when the following entities corresponding to a
	 * block different from the current. Note: all entities added after this
	 * command go into this block until setBlock() is called already.
	 *
	 * int handle are the value of DRW_Block::handleBlock added with addBlock()
	 */
	void setBlock(const int handle) override;

	/** Called to end the current block */
	void endBlock() override;

	/** Called for every point */
	void addPoint(const DRW_Point& data) override;

	/** Called for every line */
	void addLine(const DRW_Line& data) override;

	/** Called for every ray */
	void addRay(const DRW_Ray& data) override;

	/** Called for every xline */
	void addXline(const DRW_Xline& data) override;

	/** Called for every arc */
	void addArc(const DRW_Arc& data) override;

	/** Called for every circle */
	void addCircle(const DRW_Circle& data) override;

	/** Called for every ellipse */
	void addEllipse(const DRW_Ellipse& data) override;

	/** Called for every lwpolyline */
	void addLWPolyline(const DRW_LWPolyline& data) override;

	/** Called for every polyline start */
	void addPolyline(const DRW_Polyline& data) override;

	/** Called for every spline */
	void addSpline(const DRW_Spline* data) override;

	/** Called for every spline knot value */
	void addKnot(const DRW_Entity& data) override;

	/** Called for every insert. */
	void addInsert(const DRW_Insert& data) override;

	/** Called for every trace start */
	void addTrace(const DRW_Trace& data) override;

	/** Called for every 3dface start */
	void add3dFace(const DRW_3Dface&) override;

	/** Called for every solid start */
	void addSolid(const DRW_Solid& data) override;

	/** Called for every Multi Text entity. */
	void addMText(const DRW_MText& data) override;

	/** Called for every Text entity. */
	void addText(const DRW_Text& data) override;

	/** Called for every aligned dimension entity. */
	void addDimAlign(const DRW_DimAligned *data) override;

	/** Called for every linear or rotated dimension entity. */
	void addDimLinear(const DRW_DimLinear *data) override;

	/** Called for every radial dimension entity. */
	void addDimRadial(const DRW_DimRadial *data) override;

	/** Called for every diametric dimension entity. */
	void addDimDiametric(const DRW_DimDiametric *data) override;

	/** Called for every angular dimension (2 lines version) entity. */
	void addDimAngular(const DRW_DimAngular *data) override;

	/** Called for every angular dimension (3 points version) entity. */
	void addDimAngular3P(const DRW_DimAngular3p *data) override;

	/** Called for every ordinate dimension entity. */
	void addDimOrdinate(const DRW_DimOrdinate *data) override;

	/** Called for every leader start. */
	void addLeader(const DRW_Leader *data) override;

	/** Called for every hatch entity. */
	void addHatch(const DRW_Hatch *data) override;

	/** Called for every viewport entity. */
	void addViewport(const DRW_Viewport& data) override;

	/** Called for every image entity. */
	void addImage(const DRW_Image *data) override;

	/** Called for every image definition. */
	void linkImage(const DRW_ImageDef *data) override;

	/** Called for every comment in the DXF file (code 999). */
	void addComment(const char* comment) override;

	void writeHeader(DRW_Header& data) override;
	void writeBlocks() override;
	void writeBlockRecords() override;
	void writeEntities() override;
	void writeLTypes() override;
	void writeLayers() override;
	void writeTextstyles() override;
	void writeVports() override;
	void writeDimstyles() override;
	void writeAppId() override;

};

#endif // SVIMPORTDXFDIALOGH
