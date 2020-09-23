#ifndef QWT_MML_DOCUMENT_H
#define QWT_MML_DOCUMENT_H

#include <qwt_global.h>

class QString;
class QSizeF;
class QPainter;
class QPointF;
class QColor;

class QwtMmlDocument;

class QWT_EXPORT QwtMathMLDocument
{
public:
    enum MmlFont
    {
        NormalFont,
        FrakturFont,
        SansSerifFont,
        ScriptFont,
        MonospaceFont,
        DoublestruckFont
    };

    QwtMathMLDocument();
    ~QwtMathMLDocument();

    void clear();

    bool setContent( const QString &text, QString *errorMsg = 0,
        int *errorLine = 0, int *errorColumn = 0 );

    void paint( QPainter *, const QPointF &pos ) const;
    QSizeF size() const;

    QString fontName( MmlFont type ) const;
    void setFontName( MmlFont type, const QString &name );

    qreal baseFontPointSize() const;
    void setBaseFontPointSize( qreal size );

    QColor foregroundColor() const;
    void setForegroundColor( const QColor &color );

    QColor backgroundColor() const;
    void setBackgroundColor( const QColor &color );

#ifdef MML_TEST
    bool drawFrames() const;
    void setDrawFrames( bool );
#endif

private:
    QwtMmlDocument *m_doc;
};

#endif
