#ifndef SVIMPORTDXFDIALOGH
#define SVIMPORTDXFDIALOGH

#include <VICUS_Drawing.h>
#include <VICUS_Project.h>
#include <QDialog>

#include <IBKMK_Vector2D.h>
#include <IBKMK_Vector3D.h>
#include <IBK_Line.h>

#include <libdxfrw0/libdxfrw.h>
#include <libdxfrw0/drw_interface.h>
#include <libdxfrw0/drw_objects.h>
#include <libdxfrw0/drw_base.h>

namespace Ui {
class SVImportDxfDialog;
}

class SVImportDxfDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVImportDxfDialog(QWidget *parent = nullptr);
	~SVImportDxfDialog();
    void run();

private:

    void readDxfFile(VICUS::Drawing *drawing);

	Ui::SVImportDxfDialog		*m_ui;

	QString						m_lastFilePath;

	VICUS::Drawing				m_drawing;


};

/* Implementation of DRW_Interface. A dxf file will be read from top to bottom,
 * every time an object is found, one of the following methods are called.
The objects are then inserted into drawing */

class DRW_InterfaceImpl : public DRW_Interface {

    VICUS::Drawing *drawing;

public : DRW_InterfaceImpl(VICUS::Drawing *drawing);

    /** Called when header is parsed.  */
    void addHeader(const DRW_Header* data);

    /** Called for every line Type.  */
    void addLType(const DRW_LType& data);

    /** Called for every layer. */
    void addLayer(const DRW_Layer& data);

    /** Called for every dim style. */
    void addDimStyle(const DRW_Dimstyle& data);

    /** Called for every VPORT table. */
    void addVport(const DRW_Vport& data);

    /** Called for every text style. */
    void addTextStyle(const DRW_Textstyle& data);

    /** Called for every AppId entry. */
    void addAppId(const DRW_AppId& data);

    /**
     * Called for every block. Note: all entities added after this
     * command go into this block until endBlock() is called.
     *
     * @see endBlock()
     */
    void addBlock(const DRW_Block& data);

    /**
     * In DWG called when the following entities corresponding to a
     * block different from the current. Note: all entities added after this
     * command go into this block until setBlock() is called already.
     *
     * int handle are the value of DRW_Block::handleBlock added with addBlock()
     */
    void setBlock(const int handle);

    /** Called to end the current block */
    void endBlock();

    /** Called for every point */
    void addPoint(const DRW_Point& data);

    /** Called for every line */
    void addLine(const DRW_Line& data);

    /** Called for every ray */
    void addRay(const DRW_Ray& data);

    /** Called for every xline */
    void addXline(const DRW_Xline& data);

    /** Called for every arc */
    void addArc(const DRW_Arc& data);

    /** Called for every circle */
    void addCircle(const DRW_Circle& data);

    /** Called for every ellipse */
    void addEllipse(const DRW_Ellipse& data);

    /** Called for every lwpolyline */
    void addLWPolyline(const DRW_LWPolyline& data);

    /** Called for every polyline start */
    void addPolyline(const DRW_Polyline& data);

    /** Called for every spline */
    void addSpline(const DRW_Spline* data);

    /** Called for every spline knot value */
    void addKnot(const DRW_Entity& data);

    /** Called for every insert. */
    void addInsert(const DRW_Insert& data);

    /** Called for every trace start */
    void addTrace(const DRW_Trace& data);

    /** Called for every 3dface start */
    void add3dFace(const DRW_3Dface&);

    /** Called for every solid start */
    void addSolid(const DRW_Solid& data);

    /** Called for every Multi Text entity. */
    void addMText(const DRW_MText& data);

    /** Called for every Text entity. */
    void addText(const DRW_Text& data);

    /** Called for every aligned dimension entity. */
    void addDimAlign(const DRW_DimAligned *data);

    /** Called for every linear or rotated dimension entity. */
    void addDimLinear(const DRW_DimLinear *data);

    /** Called for every radial dimension entity. */
    void addDimRadial(const DRW_DimRadial *data);

    /** Called for every diametric dimension entity. */
    void addDimDiametric(const DRW_DimDiametric *data);

    /** Called for every angular dimension (2 lines version) entity. */
    void addDimAngular(const DRW_DimAngular *data);

    /** Called for every angular dimension (3 points version) entity. */
    void addDimAngular3P(const DRW_DimAngular3p *data);

    /** Called for every ordinate dimension entity. */
    void addDimOrdinate(const DRW_DimOrdinate *data);

    /** Called for every leader start. */
    void addLeader(const DRW_Leader *data);

    /** Called for every hatch entity. */
    void addHatch(const DRW_Hatch *data);

    /** Called for every viewport entity. */
    void addViewport(const DRW_Viewport& data);

    /** Called for every image entity. */
    void addImage(const DRW_Image *data);

    /** Called for every image definition. */
    void linkImage(const DRW_ImageDef *data);

    /** Called for every comment in the DXF file (code 999). */
    void addComment(const char* comment);

    void writeHeader(DRW_Header& data);
    void writeBlocks();
    void writeBlockRecords();
    void writeEntities();
    void writeLTypes();
    void writeLayers();
    void writeTextstyles();
    void writeVports();
    void writeDimstyles();
    void writeAppId();

};

#endif // SVIMPORTDXFDIALOGH
