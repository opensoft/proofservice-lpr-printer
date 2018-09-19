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

#include "proofcore/coreapplication.h"
#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"
#include "proofcore/updatemanager.h"

#include <QTimer>

Q_LOGGING_CATEGORY(proofServiceLprPrinterLog, "proofservices.lpr-printer")

int main(int argc, char *argv[])
{
    Proof::CoreApplication a(argc, argv, "Opensoft", "proofservice-lpr-printer", APP_VERSION);

    Proof::SettingsGroup *serverGroup = a.settings()->group("server", Proof::Settings::NotFoundPolicy::Add);

    int serverPort = serverGroup->value("port", 8090, Proof::Settings::NotFoundPolicy::Add).toInt();
    QStringList printerSections = serverGroup->value("printers", "", Proof::Settings::NotFoundPolicy::Add)
                                      .toString()
                                      .split('|', QString::SkipEmptyParts);
    QString defaultPrinter = serverGroup->value("default_printer", "", Proof::Settings::NotFoundPolicy::Add).toString();
    QList<PrinterInfo> printerInfos;
    printerInfos.reserve(printerSections.count());
    for (const QString &printerSection : printerSections) {
        Proof::SettingsGroup *printerGroup = a.settings()->group(printerSection.trimmed(),
                                                                 Proof::Settings::NotFoundPolicy::Add);
        QString name = printerGroup->value("name", "", Proof::Settings::NotFoundPolicy::Add).toString();
        QString host = printerGroup->value("host", "", Proof::Settings::NotFoundPolicy::Add).toString();
        bool acceptsRaw = printerGroup->value("accepts_raw", false, Proof::Settings::NotFoundPolicy::Add).toBool();
        bool acceptsFiles = printerGroup->value("accepts_files", false, Proof::Settings::NotFoundPolicy::Add).toBool();
        printerInfos << PrinterInfo{printerGroup->name(), name, host, acceptsRaw, acceptsFiles};
    }

    if (printerInfos.isEmpty()) {
        qCCritical(proofServiceLprPrinterLog) << "Printers list are empty!";
        return 0;
    }

    if (defaultPrinter.isEmpty()) {
        defaultPrinter = printerInfos.first().alias;
        serverGroup->setValue("default_printer", defaultPrinter);
    }

    LprPrinterRestServer server(serverPort, printerInfos, defaultPrinter);
    server.startListen();

    QObject::connect(a.updateManager(), &Proof::UpdateManager::updateSucceeded, &a, &QCoreApplication::quit);
    QTimer::singleShot(1, &a, &Proof::CoreApplication::postInit);

    return a.exec();
}
