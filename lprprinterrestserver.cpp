/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
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
