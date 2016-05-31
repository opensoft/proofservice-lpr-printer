#include "proofservice_lpr_printer_global.h"

#include "proofcore/coreapplication.h"
#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"

#include "lprprinterrestserver.h"

Q_LOGGING_CATEGORY(proofServiceLprPrinterLog, "proofservices.lpr-printer")

int main(int argc, char *argv[])
{
    Proof::CoreApplication a(argc, argv, "Opensoft", "proofservice-lpr-printer");

    Proof::SettingsGroup *serverGroup = a.settings()->group("server", Proof::Settings::NotFoundPolicy::Add);

    int serverPort = serverGroup->value("port", 8090, Proof::Settings::NotFoundPolicy::Add).toInt();
    QStringList printerSections = serverGroup->value("printers", "", Proof::Settings::NotFoundPolicy::Add)
            .toString().split('|', QString::SkipEmptyParts);
    QString defaultPrinterSection = serverGroup->value("default_printer", QString(), Proof::Settings::NotFoundPolicy::Add).toString();
    QList<PrinterInfo> printerInfos;
    QString defaultPrinter;
    for (const QString &printerSection : printerSections) {
        Proof::SettingsGroup *printerGroup = a.settings()->group(printerSection.trimmed(), Proof::Settings::NotFoundPolicy::Add);
        QString alias = printerGroup->value("alias", "", Proof::Settings::NotFoundPolicy::Add).toString();
        QString name = printerGroup->value("name", "", Proof::Settings::NotFoundPolicy::Add).toString();
        QString host = printerGroup->value("host", "", Proof::Settings::NotFoundPolicy::Add).toString();
        bool acceptsRaw = printerGroup->value("accepts_raw", false, Proof::Settings::NotFoundPolicy::Add).toBool();
        bool acceptsFiles = printerGroup->value("accepts_files", false, Proof::Settings::NotFoundPolicy::Add).toBool();
        if (!alias.isEmpty()) {
            printerInfos << PrinterInfo{alias, name, host, acceptsRaw, acceptsFiles};
            if (defaultPrinterSection == printerGroup->name())
                defaultPrinter = alias;
        }
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

    return a.exec();
}
