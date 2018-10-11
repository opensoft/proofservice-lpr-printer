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

#include "proofcore/basic_package.h"

#include "proofnetwork/lprprinter/proofnetworklprprinter_global.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTemporaryFile>

LprPrinterRestServer::LprPrinterRestServer() : Proof::AbstractRestServer()
{
    SettingsGroup *serverGroup = proofApp->settings()->group("server", Settings::NotFoundPolicy::Add);

    setPort(serverGroup->value("port", 8090, Settings::NotFoundPolicy::Add).toInt());
    QStringList printerSections =
        serverGroup->value("printers", "", Settings::NotFoundPolicy::Add).toString().split('|', QString::SkipEmptyParts);
    m_infos = algorithms::map(
        printerSections,
        [](const QString &printerSection) {
            const auto printerGroup = proofApp->settings()->group(printerSection.trimmed(),
                                                                  Settings::NotFoundPolicy::Add);
            QString name = printerGroup->value("name", "", Settings::NotFoundPolicy::Add).toString();
            QString host = printerGroup->value("host", "", Settings::NotFoundPolicy::Add).toString();
            bool acceptsRaw = printerGroup->value("accepts_raw", false, Settings::NotFoundPolicy::Add).toBool();
            bool acceptsFiles = printerGroup->value("accepts_files", false, Settings::NotFoundPolicy::Add).toBool();
            return qMakePair(printerGroup->name(),
                             PrinterInfo{new Proof::Hardware::LprPrinter(host, name, true, proofApp), acceptsRaw,
                                         acceptsFiles});
        },
        QMap<QString, PrinterInfo>());
    m_defaultPrinter = serverGroup->value("default_printer", "", Settings::NotFoundPolicy::Add).toString();
    if (m_defaultPrinter.isEmpty()) {
        m_defaultPrinter = m_infos.isEmpty() ? QString() : m_infos.firstKey();
        serverGroup->setValue("default_printer", m_defaultPrinter);
    }
}

int LprPrinterRestServer::printersCount() const
{
    return m_infos.count();
}

void LprPrinterRestServer::rest_post_Lpr_PrintRaw(QTcpSocket *socket, const QStringList &, const QStringList &,
                                                  const QUrlQuery &queryParams, const QByteArray &body)
{
    auto printerAlias = queryParams.hasQueryItem("printer") ? queryParams.queryItemValue("printer") : m_defaultPrinter;
    if (!m_infos.contains(printerAlias)) {
        sendErrorCode(socket, 400, "Bad Request", Proof::NetworkErrorCode::InvalidUrl);
        return;
    }
    const auto info = m_infos.value(printerAlias);
    if (!info.acceptsRaw) {
        sendErrorCode(socket, 400, "Bad Request", Proof::NetworkErrorCode::InvalidRequest);
        return;
    }

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &jsonError);
    if (jsonError.error == QJsonParseError::NoError) {
        QJsonValue data = doc.object().value("data");
        if (!data.isString())
            sendErrorCode(socket, 400, "Bad Request", Proof::NetworkErrorCode::InvalidRequest);
        else
            decorateFuture(socket, info.handler->printRawData(QByteArray::fromBase64(data.toString().toLatin1())));
    } else {
        sendErrorCode(socket, 400, "Bad Request", Proof::NetworkErrorCode::InvalidRequest);
    }
}

void LprPrinterRestServer::rest_post_Lpr_Print(QTcpSocket *socket, const QStringList &, const QStringList &,
                                               const QUrlQuery &queryParams, const QByteArray &body)
{
    auto printerAlias = queryParams.hasQueryItem("printer") ? queryParams.queryItemValue("printer") : m_defaultPrinter;
    if (!m_infos.contains(printerAlias)) {
        sendErrorCode(socket, 400, "Bad Request", Proof::NetworkErrorCode::InvalidUrl);
        return;
    }
    const auto info = m_infos.value(printerAlias);
    if (!info.acceptsFiles) {
        sendErrorCode(socket, 400, "Bad Request", Proof::NetworkErrorCode::InvalidRequest);
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

    decorateFuture(socket, info.handler->printFile(file->fileName(), numCopies))->onSuccess([file](bool) {});
}

void LprPrinterRestServer::rest_get_Lpr_Status(QTcpSocket *socket, const QStringList &, const QStringList &,
                                               const QUrlQuery &queryParams, const QByteArray &)
{
    auto printerAlias = queryParams.hasQueryItem("printer") ? queryParams.queryItemValue("printer") : m_defaultPrinter;
    if (!m_infos.contains(printerAlias)) {
        sendErrorCode(socket, 400, "Bad Request", Proof::NetworkErrorCode::InvalidUrl);
        return;
    }

    const auto info = m_infos.value(printerAlias);
    decorateFuture(socket, info.handler->printerIsReady());
}

void LprPrinterRestServer::rest_get_Lpr_List(QTcpSocket *socket, const QStringList &, const QStringList &,
                                             const QUrlQuery &, const QByteArray &)
{
    const auto answer = algorithms::map(m_infos,
                                        [](const QString &name, PrinterInfo info) {
                                            QJsonObject jsonObject;
                                            jsonObject["printer"] = name;
                                            jsonObject["accepts_raw"] = info.acceptsRaw;
                                            jsonObject["accepts_files"] = info.acceptsFiles;
                                            return jsonObject;
                                        },
                                        QJsonArray());
    sendAnswer(socket, QJsonDocument(answer).toJson(QJsonDocument::Compact), "application/json", 200, "OK");
}

FutureSP<bool> LprPrinterRestServer::decorateFuture(QTcpSocket *socket, const FutureSP<bool> &f)
{
    return f
        ->onSuccess([this, socket](bool isReady) {
            QJsonObject answer;
            answer.insert("is_ready", isReady);
            sendAnswer(socket, QJsonDocument(answer).toJson(QJsonDocument::Compact), "application/json", 200, "OK");
        })
        ->onFailure([this, socket](const Failure &f) {
            QJsonObject answer;
            answer.insert("is_ready", false);
            answer.insert("reason", f.message);
            sendAnswer(socket, QJsonDocument(answer).toJson(QJsonDocument::Compact), "application/json", 200, "OK");
        });
}
