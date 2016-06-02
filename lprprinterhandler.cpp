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
        qCDebug(proofServiceLprPrinterLog) << moduleCode << errorCode << message;
        m_lastError = message;
    });
}

void LprPrinterHandler::printerStatus(const ResultCallback &callback)
{
    if (Proof::ProofObject::call(this, &LprPrinterHandler::printerStatus, callback))
        return;
    QString errorMessage;
    bool result = m_printer->printerIsReady();
    if (!result)
        errorMessage = m_lastError;
    callback(result, errorMessage);
}

void LprPrinterHandler::printRaw(const QByteArray &label, const ResultCallback &callback)
{
    if (Proof::ProofObject::call(this, &LprPrinterHandler::printRaw, label, callback))
        return;
    QString errorMessage;
    bool result = m_printer->printRawData(label);
    if (!result)
        errorMessage = m_lastError;
    callback(result, errorMessage);
}

void LprPrinterHandler::printFile(QSharedPointer<QFile> file, unsigned int quantity, const ResultCallback &callback)
{
    if (Proof::ProofObject::call(this, &LprPrinterHandler::printFile, file, quantity, callback))
        return;
    QString errorMessage;
    bool result = m_printer->printFile(file->fileName(), quantity);
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
