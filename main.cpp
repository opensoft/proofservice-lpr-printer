#include "proofservice_label_printer_global.h"

#include "proofcore/coreapplication.h"
#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"

#include "labelprinterrestserver.h"

Q_LOGGING_CATEGORY(proofServiceLabelPrinter, "proofservices.label_printer")

int main(int argc, char *argv[])
{
    Proof::CoreApplication a(argc, argv, "Opensoft", "proofservice-label-printer");

    Proof::SettingsGroup *serverGroup = a.settings()->group("server", Proof::Settings::NotFoundPolicy::Add);
    Proof::SettingsGroup *printerGroup = a.settings()->group("printer", Proof::Settings::NotFoundPolicy::Add);

    int serverPort = serverGroup->value("port", 80, Proof::Settings::NotFoundPolicy::Add).toInt();
    QString userName = serverGroup->value("username", "proofuser", Proof::Settings::NotFoundPolicy::Add).toString();
    QString password = serverGroup->value("password", "123456", Proof::Settings::NotFoundPolicy::Add).toString();

    QString printerName = printerGroup->value("printer_name").toString();
    QString printerHost = printerGroup->value("printer_host", "", Proof::Settings::NotFoundPolicy::Add).toString();

    LabelPrinterRestServer server(userName, password, serverPort, printerName, printerHost);
    server.startListen();

    return a.exec();
}
