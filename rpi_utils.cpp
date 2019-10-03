#include <QString>
#include <QNetworkInterface>
#include <QFile>

#define TEMPERATURE_SENSOR "/sys/class/thermal/thermal_zone0/temp"

QString getHostInterfaces()
{
    QString sIPAddresses;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (int i = 0; i < interfaces.count(); i++)
    {
        if ((interfaces.at(i).flags() & QNetworkInterface::IsLoopBack) == 0) {
            QList<QNetworkAddressEntry> entries = interfaces.at(i).addressEntries();
            for (int j = 0; j < entries.count(); j++)
            {
                if (entries.at(j).ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    if (!sIPAddresses.isEmpty()) {
                        sIPAddresses += ", ";
                    }
                    sIPAddresses += entries.at(j).ip().toString();
                }
            }
        }
    }
    return sIPAddresses;
}

/**
 * Get the current operating temperature from the sensor.
 */

QString getRaspberryPi3Temp()
{
    QString result = "N/A";

    double tempC{0.0}, tempF{0.0};
    QFile tmpSensor(TEMPERATURE_SENSOR);

    if (tmpSensor.open(QIODevice::ReadOnly)) {
        QTextStream in(&tmpSensor);

        in >> tempC;

        tempF = (tempC / 1000) * (9.0/5.0) + 32;

        result = QString::number(tempF) + "℉";
    }

    return result;
}

QString getTestDisplayVersion()
{
    return QString("v%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD);
}

QString getBuildDateTime()
{
    QString fullTimeStamp = QString(__DATE__) + " " + QString(__TIME__);
    return fullTimeStamp;
}
