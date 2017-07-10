#ifndef LPRPRINTERRESTSERVER_H
#define LPRPRINTERRESTSERVER_H

#include "proofnetwork/abstractrestserver.h"

#include "lprprinterhandler.h"

#include <QUrlQuery>
#include <QList>
#include <QMap>
#include <QThread>

#include <functional>

struct PrinterInfo
{
    QString alias;
    QString name;
    QString host;
    bool acceptsRaw;
    bool acceptsFiles;
};

class LprPrinterRestServer : public Proof::AbstractRestServer
{
    Q_OBJECT
public:
    explicit LprPrinterRestServer(int port, const QList<PrinterInfo> &printers, const QString &defaultPrinter);
    ~LprPrinterRestServer() override;

protected slots:
    void rest_post_Lpr_PrintRaw(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                                const QUrlQuery &queryParams, const QByteArray &body);
    void rest_post_Lpr_Print(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                             const QUrlQuery &queryParams, const QByteArray &body);
    void rest_get_Lpr_Status(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                             const QUrlQuery &queryParams, const QByteArray &body);
    void rest_get_Lpr_List(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                           const QUrlQuery &queryParams, const QByteArray &body);

private:
    void sendStatus(QTcpSocket *socket, bool isReady, const QString &reason);
    void sendIncorrectBodyCode(QTcpSocket *socket);
    QSharedPointer<LprPrinterHandler> getPrinterHandler(QTcpSocket *socket, const QUrlQuery &queryParams);
    LprPrinterHandler::ResultCallback makeCallback(QTcpSocket *socket);

private:
    QMap<QString, QSharedPointer<LprPrinterHandler>> m_handlers;
    QList<QSharedPointer<QThread>> m_workingThreads;
    QString m_defaultPrinter;
};

#endif // LPRPRINTERRESTSERVER_H
