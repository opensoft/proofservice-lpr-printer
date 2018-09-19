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
#include "lprprinterhandler.h"

#include "proofservice_lpr_printer_global.h"

#include "proofcore/proofobject.h"

#include <QScopedPointer>
#include <QTemporaryFile>

LprPrinterHandler::LprPrinterHandler(const QString &printerName, const QString &printerHost, bool acceptsRaw,
                                     bool acceptsFiles, QObject *parent)
    : QObject(parent), m_printer(new Proof::Hardware::LprPrinter(printerHost, printerName, true, this)),
      m_acceptsRaw(acceptsRaw), m_acceptsFiles(acceptsFiles)
{
    connect(m_printer, &Proof::Hardware::LprPrinter::errorOccurred, this,
            [this](long moduleCode, long errorCode, const QString &message, bool) {
                qCWarning(proofServiceLprPrinterLog) << moduleCode << errorCode << message;
                m_lastError = message;
            });
}

void LprPrinterHandler::printerStatus(const ResultCallback &callback)
{
    if (Proof::ProofObject::call(this, &LprPrinterHandler::printerStatus, callback))
        return;
    QString errorMessage;
    bool result = m_printer->printerIsReady()->result();
    if (!result)
        errorMessage = m_lastError;
    callback(result, errorMessage);
}

void LprPrinterHandler::printRaw(const QByteArray &label, const ResultCallback &callback)
{
    if (Proof::ProofObject::call(this, &LprPrinterHandler::printRaw, label, callback))
        return;
    QString errorMessage;
    bool result = m_printer->printRawData(label)->result();
    if (!result)
        errorMessage = m_lastError;
    callback(result, errorMessage);
}

void LprPrinterHandler::printFile(const QSharedPointer<QFile> &file, unsigned int quantity, const ResultCallback &callback)
{
    if (Proof::ProofObject::call(this, &LprPrinterHandler::printFile, file, quantity, callback))
        return;
    QString errorMessage;
    bool result = m_printer->printFile(file->fileName(), quantity)->result();
    if (!result)
        errorMessage = m_lastError;
    callback(result, errorMessage);
}

bool LprPrinterHandler::acceptsRaw() const
{
    return m_acceptsRaw;
}

bool LprPrinterHandler::acceptsFiles() const
{
    return m_acceptsFiles;
}
