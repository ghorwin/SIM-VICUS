#ifndef SVSmartIntersectionDialogH
#define SVSmartIntersectionDialogH

#include <QDialog>
#include <QPlainTextEdit>

#include <VICUS_Building.h>
#include <VICUS_ComponentInstance.h>

#include <IBK_MessageHandler.h>

#include "SVMessageHandler.h"

namespace Ui {
class SVSmartIntersectionDialog;
}

class SVSmartIntersectionDialog : public QDialog
{
    Q_OBJECT

public:

    enum ClippingResults {
        AcceptClipping,
        CancelledClipping
    };

    explicit SVSmartIntersectionDialog(QWidget *parent = nullptr);
    ~SVSmartIntersectionDialog();

    /*! Starts Clipping Dialog and returns clipping result. */
    ClippingResults clipProject();

	const std::vector<VICUS::Building> &buildings() const;

	const std::vector<VICUS::ComponentInstance> &componentInstances() const;

private slots:
    void on_pushButtonStartClipping_clicked();

    void on_pushButtonApply_clicked();

    void on_pushButtonCancel_clicked();

private:
    Ui::SVSmartIntersectionDialog *m_ui;

    /*! Maximum distance between surfaces used for clipping. */
    double                                   m_maxDistance;
    /*! Maximum angle between normals of surfaces used for clipping. */
    double                                   m_maxAngle;

    /*! Copy of all buildings. */
    std::vector<VICUS::Building>             m_buildings;

    /*! Copy of all component instances. */
	std::vector<VICUS::ComponentInstance>    m_componentInstances;

    /*! Return Code of Clipping. */
    ClippingResults                          m_returnCode;
};


class SVClippingMessageHandler : public QObject, public IBK::MessageHandler {
    Q_OBJECT
public:
    explicit SVClippingMessageHandler(QObject *parent, QPlainTextEdit *plainTextEdit);
    virtual ~SVClippingMessageHandler();


    /*! Overloaded to received msg info. */
    virtual void msg(const std::string& msg,
        IBK::msg_type_t t = IBK::MSG_PROGRESS,
        const char * func_id = nullptr,
        int verbose_level = -1);

    SVMessageHandler	*m_defaultMsgHandler = nullptr;
    QPlainTextEdit		*m_plainTextEdit = nullptr;
};

#endif // SVSmartIntersectionDialogH
