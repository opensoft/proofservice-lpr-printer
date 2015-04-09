#ifndef LABELPRINTERHANDLER_H
#define LABELPRINTERHANDLER_H

#include <QString>

class LabelPrinterHandler
{
public:
    explicit LabelPrinterHandler(const QString &printerName, const QString &printerHost, bool strictPrinterCheck);

    bool printerStatus(QString *errorMessage);
    bool print(const QByteArray &label, QString *errorMessage);

private:
    QString m_printerName;
    QString m_printerHost;
    bool m_strictPrinterCheck;
};

#endif // LABELPRINTERHANDLER_H
