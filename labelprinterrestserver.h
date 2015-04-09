#ifndef LABELPRINTERRESTSERVER_H
#define LABELPRINTERRESTSERVER_H

#include "proofnetwork/abstractrestserver.h"

#include "labelprinterhandler.h"

#include <QUrlQuery>

class LabelPrinterRestServer : public Proof::AbstractRestServer
{
    Q_OBJECT
public:
    explicit LabelPrinterRestServer(const QString &userName, const QString &password, int port,
                                    const QString &printerName, const QString &printerHost, QObject *parent = 0);

protected slots:
    void rest_get_LabelPrinter_Status(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                                      const QUrlQuery &queryParams, const QByteArray &body);
    void rest_post_LabelPrinter_Print(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                                       const QUrlQuery &queryParams, const QByteArray &body);

private:
    void sendStatus(QTcpSocket *socket, bool isReady, const QString &reason);
    void sendBadRequest(QTcpSocket *socket, const QString &reason);

private:
    LabelPrinterHandler m_handler;
};

#endif // LABELPRINTERRESTSERVER_H
