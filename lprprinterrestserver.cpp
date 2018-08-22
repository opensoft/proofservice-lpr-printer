#include "lprprinterrestserver.h"

#include "proofservice_lpr_printer_global.h"

#include "proofnetwork/lprprinter/proofnetworklprprinter_global.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTemporaryFile>

LprPrinterRestServer::LprPrinterRestServer(quint16 port, const QList<PrinterInfo> &printers, const QString &defaultPrinter)
    : Proof::AbstractRestServer(port), m_defaultPrinter(defaultPrinter)
{
    m_workingThreads.reserve(printers.count());
    for (const PrinterInfo &printer : printers) {
        auto handler = QSharedPointer<LprPrinterHandler>::create(printer.name, printer.host, printer.acceptsRaw,
                                                                 printer.acceptsFiles);
        auto thread = QSharedPointer<QThread>::create();
        handler->moveToThread(thread.data());
        thread->start();
        m_workingThreads << thread;
        m_handlers[printer.alias] = handler;
    }
}

LprPrinterRestServer::~LprPrinterRestServer()
{
    for (const auto &thread : qAsConst(m_workingThreads))
        thread->wait();
}

void LprPrinterRestServer::rest_post_Lpr_PrintRaw(QTcpSocket *socket, const QStringList &, const QStringList &,
                                                  const QUrlQuery &queryParams, const QByteArray &body)
{
    auto handler = getPrinterHandler(socket, queryParams);
    if (!handler)
        return;
    if (!handler->acceptsRaw()) {
        sendIncorrectBodyCode(socket);
        return;
    }

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
                if (!data.isString())
                    sendIncorrectBodyCode(socket);
                else
                    handler->printRaw(QByteArray::fromBase64(data.toString().toLocal8Bit()), makeCallback(socket));
            }
        }
    } else {
        sendIncorrectBodyCode(socket);
    }
}

void LprPrinterRestServer::rest_post_Lpr_Print(QTcpSocket *socket, const QStringList &, const QStringList &,
                                               const QUrlQuery &queryParams, const QByteArray &body)
{
    auto handler = getPrinterHandler(socket, queryParams);
    if (!handler)
        return;
    if (!handler->acceptsFiles()) {
        sendIncorrectBodyCode(socket);
        return;
    }

    unsigned int numCopies = queryParams.hasQueryItem("copies") ? queryParams.queryItemValue("copies").toUInt() : 1;

    auto file = QSharedPointer<QTemporaryFile>::create();
    if (!file->open()) {
        qCDebug(proofServiceLprPrinterLog) << file->errorString();
        sendErrorCode(socket, 500, "Internal Server Error", Proof::NetworkErrorCode::InternalError);
        return;
    }
    file->write(body);
    file->flush();

    handler->printFile(file, numCopies, makeCallback(socket));
}

void LprPrinterRestServer::rest_get_Lpr_Status(QTcpSocket *socket, const QStringList &, const QStringList &,
                                               const QUrlQuery &queryParams, const QByteArray &)
{
    auto handler = getPrinterHandler(socket, queryParams);
    if (!handler)
        return;
    handler->printerStatus(makeCallback(socket));
}

void LprPrinterRestServer::rest_get_Lpr_List(QTcpSocket *socket, const QStringList &, const QStringList &,
                                             const QUrlQuery &, const QByteArray &)
{
    QJsonArray answer;
    for (auto it = m_handlers.constBegin(); it != m_handlers.constEnd(); ++it) {
        const auto handler = it.value();
        QJsonObject printerInfo;
        printerInfo["printer"] = it.key();
        printerInfo["accepts_raw"] = handler->acceptsRaw();
        printerInfo["accepts_files"] = handler->acceptsFiles();
        answer << printerInfo;
    }
    sendAnswer(socket, QJsonDocument(answer).toJson(QJsonDocument::Compact), "application/json", 200, "OK");
}

void LprPrinterRestServer::sendStatus(QTcpSocket *socket, bool isReady, const QString &reason)
{
    QJsonObject answer;
    answer.insert("is_ready", isReady);
    if (!reason.isNull())
        answer.insert("reason", reason);
    sendAnswer(socket, QJsonDocument(answer).toJson(QJsonDocument::Compact), "application/json", 200, "OK");
}

void LprPrinterRestServer::sendIncorrectBodyCode(QTcpSocket *socket)
{
    sendErrorCode(socket, 400, "Bad Request", Proof::NetworkErrorCode::InvalidRequest);
}

QSharedPointer<LprPrinterHandler> LprPrinterRestServer::getPrinterHandler(QTcpSocket *socket, const QUrlQuery &queryParams)
{
    auto printerAlias = queryParams.hasQueryItem("printer") ? queryParams.queryItemValue("printer") : m_defaultPrinter;
    auto handler = m_handlers.value(printerAlias);
    if (!handler)
        sendErrorCode(socket, 400, "Bad Request", Proof::NetworkErrorCode::InvalidUrl);
    return handler;
}

LprPrinterHandler::ResultCallback LprPrinterRestServer::makeCallback(QTcpSocket *socket)
{
    return [socket, this](bool result, const QString &errorMessage) { sendStatus(socket, result, errorMessage); };
}
