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
#ifndef LPRPRINTERHANDLER_H
#define LPRPRINTERHANDLER_H

#include "proofutils/lprprinter.h"

#include <QFile>
#include <QObject>
#include <QString>

#include <functional>

//TODO: adopt future-based interface of Hardware::LprPrinter
class LprPrinterHandler : public QObject
{
    Q_OBJECT
public:
    explicit LprPrinterHandler(const QString &printerName, const QString &printerHost, bool acceptsRaw,
                               bool acceptsFiles, QObject *parent = nullptr);

    using ResultCallback = std::function<void(bool, const QString &)>;

    void printerStatus(const ResultCallback &callback);
    void printRaw(const QByteArray &label, const ResultCallback &callback);
    void printFile(const QSharedPointer<QFile> &file, unsigned int quantity, const ResultCallback &callback);

    bool acceptsRaw() const;
    bool acceptsFiles() const;

private:
    Proof::Hardware::LprPrinter *m_printer;
    QString m_lastError;
    bool m_acceptsRaw;
    bool m_acceptsFiles;
};

#endif // LPRPRINTERHANDLER_H
