#include "labelprinterhandler.h"

#include "proofhardware/labelprinter/labelprinter.h"

#include "proofservice_label_printer_global.h"

LabelPrinterHandler::LabelPrinterHandler(const QString &printerName, const QString &printerHost, bool strictPrinterCheck)
    : m_printerName(printerName), m_printerHost(printerHost), m_strictPrinterCheck(strictPrinterCheck)
{
}

bool LabelPrinterHandler::printerStatus(QString *errorMessage)
{
    Proof::Hardware::LabelPrinter printer(m_printerName, m_printerHost, m_strictPrinterCheck);
    QObject::connect(&printer, &Proof::Hardware::LabelPrinter::errorOccurred,
                     &printer, [errorMessage](const QString &message) {
        qCDebug(proofServiceLabelPrinter) << message;
        if (errorMessage != nullptr)
            *errorMessage = message;
    });
    return printer.printerIsReady();
}
