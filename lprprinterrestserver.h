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
#ifndef LPRPRINTERRESTSERVER_H
#define LPRPRINTERRESTSERVER_H

#include "lprprinterhandler.h"

#include "proofnetwork/abstractrestserver.h"

#include <QList>
#include <QMap>
#include <QThread>
#include <QUrlQuery>

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
    explicit LprPrinterRestServer(quint16 port, const QList<PrinterInfo> &printers, const QString &defaultPrinter);
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
