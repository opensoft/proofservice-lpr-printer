#ifndef LPRPRINTERHANDLER_H
#define LPRPRINTERHANDLER_H

#include "proofhardware/lprprinter/lprprinter.h"

#include <QObject>
#include <QString>
#include <QFile>

#include <functional>

class LprPrinterHandler : public QObject
{
    Q_OBJECT
public:
    explicit LprPrinterHandler(const QString &printerName, const QString &printerHost, bool acceptsRaw,
                               bool acceptsFiles, QObject *parent = nullptr);

    using ResultCallback = std::function<void (bool, const QString &)>;

    void printerStatus(const ResultCallback &callback);
    void printRaw(const QByteArray &label, const ResultCallback &callback);
    void printFile(QSharedPointer<QFile> file, unsigned int quantity, const ResultCallback &callback);

    bool acceptsRaw() const;
    bool acceptsFiles() const;

private:
    Proof::Hardware::LprPrinter *m_printer;
    QString m_lastError;
    bool m_acceptsRaw;
    bool m_acceptsFiles;
};

#endif // LPRPRINTERHANDLER_H
