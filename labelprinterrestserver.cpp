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

void LabelPrinterRestServer::rest_post_LabelPrinter_Print(QTcpSocket *socket, const QStringList &, const QStringList &,
                                                           const QUrlQuery &, const QByteArray &body)
{
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &jsonError);
    if (jsonError.error == QJsonParseError::NoError) {
        if (!doc.isObject()) {
            sendBadRequest(socket, "Can't find object in JSON");
        } else {
            QJsonObject object = doc.object();
            if (!object.contains("data")) {
                sendBadRequest(socket, "Can't find property \"data\" in JSON");
            } else {
                QJsonValue data = object.value("data");
                if (!data.isString()) {
                    sendBadRequest(socket, "Property \"data\" must be a string");
                } else {
                    QString errorMessage;
                    bool result = m_handler.print(QByteArray::fromBase64(data.toString().toLocal8Bit()), &errorMessage);
                    sendStatus(socket, result, errorMessage);
                }
            }
        }
    } else {
        sendBadRequest(socket, jsonError.errorString());
    }
}

void LabelPrinterRestServer::sendStatus(QTcpSocket *socket, bool isReady, const QString &reason)
{
    QJsonObject answer;
    answer.insert("is_ready", isReady);
    if (!reason.isNull())
        answer.insert("reason", reason);
    sendAnswer(socket, QJsonDocument(answer).toJson(QJsonDocument::Compact), "application/json", 200, "OK");
}

void LabelPrinterRestServer::sendBadRequest(QTcpSocket *socket, const QString &reason)
{
    sendAnswer(socket, QByteArray(), QString(), 400, reason);
}
