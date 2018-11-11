//
// Created by chang on 2018-03-07.
//

#ifndef SERIALWIZARD_SERIALREADWRITER_H
#define SERIALWIZARD_SERIALREADWRITER_H

#include <QtSerialPort/QSerialPort>

struct SerialSettings {
    QString name;
    qint32 baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl;
    bool localEchoEnabled;
};


class SerialReadWriter : public QObject{
Q_OBJECT
public:
    explicit SerialReadWriter(QObject *parent = nullptr);

    void setSerialSettings(SerialSettings serialSettings);

    QString settingsText() const;

    bool open();

    bool isOpen();

    bool isConnected();

    void close();

    QByteArray readAll();

    qint64 write(const QByteArray &byteArray) const;
signals:
    void readyRead();

private:
    SerialSettings settings;
    QSerialPort *serial{nullptr};
};


#endif //SERIALWIZARD_SERIALREADWRITER_H
