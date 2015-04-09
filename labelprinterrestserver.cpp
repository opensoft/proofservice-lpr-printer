#include "labelprinterrestserver.h"

#include <QJsonObject>
#include <QJsonDocument>

LabelPrinterRestServer::LabelPrinterRestServer(const QString &userName, const QString &password, int port,
                                               const QString &printerName, const QString &printerHost, bool strictPrinterCheck,
                                               QObject *parent)
    : Proof::AbstractRestServer(userName, password, "", port, parent), m_handler(printerName, printerHost, strictPrinterCheck)
{

}

void LabelPrinterRestServer::rest_get_LabelPrinter_Status(QTcpSocket *socket, const QStringList &, const QStringList &,
                                                          const QUrlQuery &, const QByteArray &)
{
    QString errorMessage;
    bool result = m_handler.printerStatus(&errorMessage);
    sendStatus(socket, result, errorMessage);
}

void LabelPrinterRestServer::sendStatus(QTcpSocket *socket, bool isReady, const QString &reason)
{
    QJsonObject answer;
    answer.insert("is_ready", isReady);
    if (!reason.isNull())
        answer.insert("reason", reason);
    sendAnswer(socket, QJsonDocument(answer).toJson(QJsonDocument::Compact), "application/json");
}
