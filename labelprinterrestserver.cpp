#include "labelprinterrestserver.h"

#include "proofnetwork/labelprinter/errorcodes.h"

#include <QJsonObject>
#include <QJsonDocument>

LabelPrinterRestServer::LabelPrinterRestServer(int port, const QString &printerName, const QString &printerHost, QObject *parent)
    : Proof::AbstractRestServer("", port, Proof::RestAuthType::NoAuth, parent),
      m_handler(printerName, printerHost)
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
            sendIncorrectBodyCode(socket);
        } else {
            QJsonObject object = doc.object();
            if (!object.contains("data")) {
                sendIncorrectBodyCode(socket);
            } else {
                QJsonValue data = object.value("data");
                if (!data.isString()) {
                    sendIncorrectBodyCode(socket);
                } else {
                    QString errorMessage;
                    bool result = m_handler.print(QByteArray::fromBase64(data.toString().toLocal8Bit()), &errorMessage);
                    sendStatus(socket, result, errorMessage);
                }
            }
        }
    } else {
        sendIncorrectBodyCode(socket);
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

void LabelPrinterRestServer::sendIncorrectBodyCode(QTcpSocket *socket)
{
    sendErrorCode(socket, 400, "Bad Request", Proof::NetworkServices::LabelPrinterErrors::IncorrectRequestBody);
}
