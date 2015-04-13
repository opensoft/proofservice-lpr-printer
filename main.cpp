#include "proofservice_label_printer_global.h"

#include "proofcore/coreapplication.h"
#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"

#include "labelprinterrestserver.h"

Q_LOGGING_CATEGORY(proofServiceLabelPrinterLog, "proofservices.label-printer")

int main(int argc, char *argv[])
{
    Proof::CoreApplication a(argc, argv, "Opensoft", "proofservice-label-printer");

    Proof::SettingsGroup *serverGroup = a.settings()->group("server", Proof::Settings::NotFoundPolicy::Add);
    Proof::SettingsGroup *printerGroup = a.settings()->group("label_printer", Proof::Settings::NotFoundPolicy::Add);

    int serverPort = serverGroup->value("port", 8090, Proof::Settings::NotFoundPolicy::Add).toInt();

    QString printerName = printerGroup->value("name", "Zebra_UPS", Proof::Settings::NotFoundPolicy::Add).toString();
    QString printerHost = printerGroup->value("host", "", Proof::Settings::NotFoundPolicy::Add).toString();

    LabelPrinterRestServer server(serverPort, printerName, printerHost);
    server.startListen();

    return a.exec();
}
