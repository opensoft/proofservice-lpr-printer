#include "labelprinterhandler.h"

#include "proofhardware/labelprinter/labelprinter.h"

#include "proofservice_label_printer_global.h"

LabelPrinterHandler::LabelPrinterHandler(const QString &printerName, const QString &printerHost)
    : m_printerName(printerName), m_printerHost(printerHost)
{
}

bool LabelPrinterHandler::printerStatus(QString *errorMessage)
{
    Proof::Hardware::LabelPrinter printer(m_printerHost, m_printerName, true);
    QObject::connect(&printer, &Proof::Hardware::LabelPrinter::errorOccurred,
                     &printer, [errorMessage](const QString &message) {
        qCDebug(proofServiceLabelPrinter) << message;
        if (errorMessage != nullptr)
            *errorMessage = message;
    });
    return printer.printerIsReady();
}

bool LabelPrinterHandler::print(const QByteArray &label, QString *errorMessage)
{
    Proof::Hardware::LabelPrinter printer(m_printerHost, m_printerName, true);
    QObject::connect(&printer, &Proof::Hardware::LabelPrinter::errorOccurred,
                     &printer, [errorMessage](const QString &message) {
        qCDebug(proofServiceLabelPrinter) << message;
        if (errorMessage != nullptr)
            *errorMessage = message;
    });
    return printer.printLabel(label);
}
