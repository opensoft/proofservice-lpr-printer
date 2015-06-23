#include "labelprinterhandler.h"

#include "proofhardware/lprprinter/lprprinter.h"

#include "proofservice_label_printer_global.h"

#include <QScopedPointer>

LabelPrinterHandler::LabelPrinterHandler(const QString &printerName, const QString &printerHost)
    : m_printerName(printerName), m_printerHost(printerHost)
{
}

bool LabelPrinterHandler::printerStatus(QString *errorMessage)
{
    QScopedPointer<Proof::Hardware::LprPrinter> printer(new Proof::Hardware::LprPrinter(m_printerHost, m_printerName, true));
    QObject::connect(printer.data(), &Proof::Hardware::LprPrinter::errorOccurred,
                     printer.data(), [errorMessage](const QString &message) {
        qCDebug(proofServiceLabelPrinterLog) << message;
        if (errorMessage != nullptr)
            *errorMessage = message;
    });
    return printer->printerIsReady();
}

bool LabelPrinterHandler::print(const QByteArray &label, QString *errorMessage)
{
    QScopedPointer<Proof::Hardware::LprPrinter> printer(new Proof::Hardware::LprPrinter(m_printerHost, m_printerName, true));
    QObject::connect(printer.data(), &Proof::Hardware::LprPrinter::errorOccurred,
                     printer.data(), [errorMessage](const QString &message) {
        qCDebug(proofServiceLabelPrinterLog) << message;
        if (errorMessage != nullptr)
            *errorMessage = message;
    });
    return printer->printRawData(label);
}
