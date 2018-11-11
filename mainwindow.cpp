//
// Created by chang on 2017-07-28.
//
#include <QAction>
#include <QCheckBox>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QtSerialPort/QSerialPort>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QDebug>
#include <QTextBrowser>
#include <QtWidgets/QFileDialog>
#include <QTimer>
#include <QtCore/QSettings>
#include <QtCore/QProcess>
#include <QStatusBar>
#include <QSplitter>
#include <data/SerialReadWriter.h>
//#include <data/TcpReadWriter.h>
#include <QRadioButton>
#include <QButtonGroup>
//#include <data/BridgeReadWriter.h>
#include <QtSerialPort/QSerialPortInfo>

#if WIN32
#include <windows.h>
#include <windowsx.h>
#include <dbt.h>
#endif
#include "mainwindow.h"
//#include "CalculateCheckSumDialog.h"
#include "global.h"

#include "serial/SerialController.h"
#include "serial/NormalSerialController.h"
//#include "serial/FixedBytesSerialController.h"
//#include "serial/LineSerialController.h"
//#include "serial/FrameSerialController.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), sendCount(0), receiveCount(0) {

    init();
    initUi();
    createConnect();

    readSettings();
    createActions();
    createMenu();
    createStatusBar();

    updateStatusMessage(tr("就绪！"));
}

MainWindow::~MainWindow() {
    writeSettings();
    closeReadWriter();
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#if WIN32
    MSG *msg = reinterpret_cast<MSG*>(message);
    PDEV_BROADCAST_HDR lpdb = reinterpret_cast<PDEV_BROADCAST_HDR>(msg->lParam);

    // Device changed message
    if (msg->message == WM_DEVICECHANGE)
    {
        // Only care about plug-in or plug-out.
        if (msg->wParam == DBT_DEVICEARRIVAL || msg->wParam == DBT_DEVICEREMOVECOMPLETE)
        {
            // Only care about PORT type device.
            if (lpdb->dbch_devicetype == DBT_DEVTYP_PORT)
            {
                PDEV_BROADCAST_PORT_W port = reinterpret_cast<PDEV_BROADCAST_PORT_W>(lpdb);
                //Get dbcp_name length
                size_t len = port->dbcp_size - sizeof(port->dbcp_devicetype) - sizeof(port->dbcp_reserved) - sizeof(port->dbcp_size);
                wchar_t *name = new wchar_t[len];
                QString serialPortName;
                memcpy(name, port->dbcp_name, len);
                serialPortName = QString::fromWCharArray(name);
                emit this->serialPortUpdate(msg->wParam == DBT_DEVICEARRIVAL ? true : false, serialPortName);
                delete [] name;
                return true;
            }
        }
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::init() {
    autoSendTimer = new QTimer();
}

void MainWindow::initUi() {

    setWindowTitle(tr("串口调试工具"));

    setMinimumSize(1280, 800);

    auto serialPortNameLabel = new QLabel(tr("串口"), this);
    QStringList serialPortNameList = getSerialNameList();

    serialPortNameComboBox = new QComboBox(this);
    serialPortNameComboBox->addItems(serialPortNameList);
    serialPortNameLabel->setBuddy(serialPortNameComboBox);

    auto *serialPortBaudRateLabel = new QLabel(tr("波特率"), this);
    serialPortBaudRateComboBox = new QComboBox(this);
    serialPortBaudRateComboBox->addItems(QStringList()
                                                 << "1200"
                                                 << "2400"
                                                 << "4800"
                                                 << "9600"
                                                 << "19200"
                                                 << "38400"
                                                 << "25600"
                                                 << "57600"
                                                 << "115200"
                                                 << "256000"

    );
    serialPortBaudRateComboBox->setEditable(true);
    serialPortBaudRateLabel->setBuddy(serialPortBaudRateComboBox);


    auto serialPortDataBitsLabel = new QLabel(tr("数据位"), this);
    serialPortDataBitsComboBox = new QComboBox(this);
    serialPortDataBitsComboBox->addItems(QStringList() << "5" << "6" << "7" << "8");
    serialPortDataBitsLabel->setBuddy(serialPortDataBitsComboBox);

    auto serialPortStopBitsLabel = new QLabel(tr("停止位"), this);
    serialPortStopBitsComboBox = new QComboBox(this);
    serialPortStopBitsLabel->setBuddy(serialPortStopBitsComboBox);
    serialPortStopBitsComboBox->addItem(tr("1"), QSerialPort::OneStop);
    serialPortStopBitsComboBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
    serialPortStopBitsComboBox->addItem(tr("2"), QSerialPort::TwoStop);

    auto serialPortParityLabel = new QLabel(tr("校验位"), this);
    serialPortParityComboBox = new QComboBox(this);
    serialPortParityComboBox->addItem(tr("无校验"), QSerialPort::NoParity);
    serialPortParityComboBox->addItem(tr("奇校验"), QSerialPort::OddParity);
    serialPortParityComboBox->addItem(tr("偶校验"), QSerialPort::EvenParity);
    serialPortParityComboBox->addItem(tr("空校验"), QSerialPort::SpaceParity);
    serialPortParityComboBox->addItem(tr("标志校验"), QSerialPort::MarkParity);
    serialPortParityLabel->setBuddy(serialPortParityComboBox);

    auto serialPortSettingsGridLayout = new QGridLayout;
    serialPortSettingsGridLayout->addWidget(serialPortNameLabel, 0, 0);
    serialPortSettingsGridLayout->addWidget(serialPortNameComboBox, 0, 1);
    serialPortSettingsGridLayout->addWidget(serialPortBaudRateLabel, 1, 0);
    serialPortSettingsGridLayout->addWidget(serialPortBaudRateComboBox, 1, 1);
    serialPortSettingsGridLayout->addWidget(serialPortDataBitsLabel, 2, 0);
    serialPortSettingsGridLayout->addWidget(serialPortDataBitsComboBox, 2, 1);
    serialPortSettingsGridLayout->addWidget(serialPortStopBitsLabel, 3, 0);
    serialPortSettingsGridLayout->addWidget(serialPortStopBitsComboBox, 3, 1);
    serialPortSettingsGridLayout->addWidget(serialPortParityLabel, 4, 0);
    serialPortSettingsGridLayout->addWidget(serialPortParityComboBox, 4, 1);

    auto serialPortSettingsGroupBox = new QGroupBox(tr("串口设置"));
    serialPortSettingsGroupBox->setLayout(serialPortSettingsGridLayout);

    openSerialButton = new QPushButton(tr("打开"), this);

    addLineReturnCheckBox = new QCheckBox(tr("自动换行"), this);
    displayReceiveDataAsHexCheckBox = new QCheckBox(tr("十六进制显示"), this);
    addReceiveTimestampCheckBox = new QCheckBox(tr("添加时间戳"), this);
    pauseReceiveCheckBox = new QCheckBox(tr("暂停接收"), this);
    saveReceiveDataButton = new QPushButton(tr("保存数据"), this);
    clearReceiveDataButton = new QPushButton(tr("清除显示"), this);

    auto receiveSettingLayout = new QGridLayout;
    receiveSettingLayout->addWidget(addLineReturnCheckBox,0,0);
    receiveSettingLayout->addWidget(displayReceiveDataAsHexCheckBox,0,1);
    receiveSettingLayout->addWidget(addReceiveTimestampCheckBox,1,0);
    receiveSettingLayout->addWidget(pauseReceiveCheckBox,1,1);

    receiveSettingLayout->addWidget(saveReceiveDataButton, 2, 0);
    receiveSettingLayout->addWidget(clearReceiveDataButton, 2, 1);

    auto receiveSettingGroupBox = new QGroupBox(tr("接收设置"));
    receiveSettingGroupBox->setLayout(receiveSettingLayout);

    receiveDataBrowser = new QTextBrowser(this);
    receiveDataBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto receiveDataLayout = new QVBoxLayout;
    receiveDataLayout->addWidget(receiveDataBrowser);
    auto receiveDataGroupBox = new QGroupBox(tr("数据接收显示"));
    receiveDataGroupBox->setLayout(receiveDataLayout);
    receiveDataGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    sendHexCheckBox = new QCheckBox(tr("十六进制发送"), this);
    displaySendDataCheckBox = new QCheckBox(tr("显示发送数据"), this);
    displaySendDataAsHexCheckBox = new QCheckBox(tr("十六进制显示"), this);


    autoSendCheckBox = new QCheckBox(tr("自动发送"), this);
    auto sendIntervalLabel = new QLabel(tr("间隔(毫秒)"), this);
    sendIntervalLineEdit = new QLineEdit(this);
    sendIntervalLineEdit->setMaximumWidth(50);
    sendIntervalLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    sendIntervalLabel->setBuddy(sendIntervalLineEdit);

    auto sendIntervalLayout = new QHBoxLayout;
    sendIntervalLayout->addWidget(sendIntervalLabel);
    sendIntervalLayout->addWidget(sendIntervalLineEdit);

    auto autoSendLayout = new QVBoxLayout;
    autoSendLayout->addWidget(autoSendCheckBox);
    autoSendLayout->addLayout(sendIntervalLayout);
    auto autoSendGroupBox = new QGroupBox("自动发送设置");
    autoSendGroupBox->setLayout(autoSendLayout);

    loopSendCheckBox = new QCheckBox(tr("循环发送"), this);
    resetLoopSendButton = new QPushButton(tr("重置计数"), this);
    currentSendCountLineEdit = new QLineEdit(this);
    currentSendCountLineEdit->setMaximumWidth(50);
    currentSendCountLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    auto currentSendCountLabel = new QLabel(tr("计数"), this);
    currentSendCountLabel->setBuddy(currentSendCountLineEdit);
    auto divideLabel = new QLabel(tr("/"),this);
    totalSendCountLabel = new QLabel(tr("0"),this);

    auto loopLayout1 = new QHBoxLayout;
    loopLayout1->addWidget(loopSendCheckBox);
    loopLayout1->addWidget(resetLoopSendButton);

    auto sendCountLayout = new QHBoxLayout;
    sendCountLayout->setAlignment(Qt::AlignLeft);
    sendCountLayout->addWidget(currentSendCountLabel);
    sendCountLayout->addWidget(currentSendCountLineEdit);
    sendCountLayout->addWidget(divideLabel);
    sendCountLayout->addWidget(totalSendCountLabel);

    auto loopSendLayout = new QVBoxLayout;
    loopSendLayout->addLayout(loopLayout1);
    loopSendLayout->addLayout(sendCountLayout);
    auto loopSendGroupBox = new QGroupBox(tr("循环发送设置"), this);
    loopSendGroupBox->setLayout(loopSendLayout);

    saveSentDataButton = new QPushButton(tr("保存数据"), this);
    clearSentDataButton = new QPushButton(tr("清除显示"), this);

    auto sendSettingLayout = new QGridLayout;
    sendSettingLayout->addWidget(sendHexCheckBox,0,0,1,2);
    sendSettingLayout->addWidget(displaySendDataCheckBox,1,0);
    sendSettingLayout->addWidget(displaySendDataAsHexCheckBox,1,1);
    sendSettingLayout->addWidget(saveSentDataButton,2,0);
    sendSettingLayout->addWidget(clearSentDataButton,2,1);

    auto sendSettingGroupBox = new QGroupBox(tr("发送设置"));
    sendSettingGroupBox->setLayout(sendSettingLayout);

    sendDataBrowser = new QTextBrowser(this);
    sendDataBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto sendDataLayout = new QVBoxLayout;
    sendDataLayout->addWidget(sendDataBrowser);
    auto sendDataGroupBox = new QGroupBox(tr("数据发送显示"));
    sendDataGroupBox->setLayout(sendDataLayout);
    sendDataGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto dataBrowserSplitter = new QSplitter(this);
    dataBrowserSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    dataBrowserSplitter->addWidget(receiveDataGroupBox);
    dataBrowserSplitter->addWidget(sendDataGroupBox);

    sendTextEdit = new QTextEdit(this);
    sendTextEdit->setMinimumHeight(100);
    sendTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);


    sendAllButton = new QPushButton(tr("全部发送"));

    auto sendButtonLayout = new QVBoxLayout;
    sendButtonLayout->setAlignment(Qt::AlignBottom);
    sendButtonLayout->addWidget(sendAllButton);

    sendButtonLayout->setSizeConstraint(QLayout::SetFixedSize);

    auto sendLayout = new QHBoxLayout;
    sendLayout->addWidget(sendTextEdit);
    sendLayout->addLayout(sendButtonLayout);

    sendLayout->setSizeConstraint(QLayout::SetFixedSize);

    auto mainVBoxLayout1 = new QVBoxLayout;
    mainVBoxLayout1->addWidget(serialPortSettingsGroupBox);
    mainVBoxLayout1->addWidget(openSerialButton);
    mainVBoxLayout1->addWidget(receiveSettingGroupBox);
    mainVBoxLayout1->addWidget(sendSettingGroupBox);
    mainVBoxLayout1->addWidget(autoSendGroupBox);
    mainVBoxLayout1->addWidget(loopSendGroupBox);
    mainVBoxLayout1->addStretch();

    auto mainVBoxLayout2 = new QVBoxLayout;
    mainVBoxLayout2->addWidget(dataBrowserSplitter);
    mainVBoxLayout2->addLayout(sendLayout);

    auto widget = new QWidget(this);

    auto mainLayout = new QHBoxLayout;

    mainLayout->addLayout(mainVBoxLayout1);
    mainLayout->addLayout(mainVBoxLayout2);

    widget->setLayout(mainLayout);
    setCentralWidget(widget);
}

void MainWindow::createStatusBar() {

    auto receiveByteCountLabel = new QLabel(tr("接收:"), this);
    statusBarReadBytesLabel = new QLabel(this);
    statusBarReadBytesLabel->setMinimumWidth(100);
    statusBarReadBytesLabel->setText("0");

    auto sendByteCountLabel = new QLabel(tr("发送:"), this);
    statusBarWriteBytesLabel = new QLabel(this);
    statusBarWriteBytesLabel->setMinimumWidth(100);
    statusBarWriteBytesLabel->setText("0");

    statusBarResetCountButton = new QPushButton(tr("重置"), this);

    statusBar()->addPermanentWidget(receiveByteCountLabel);
    statusBar()->addPermanentWidget(statusBarReadBytesLabel);
    statusBar()->addPermanentWidget(sendByteCountLabel);
    statusBar()->addPermanentWidget(statusBarWriteBytesLabel);
    statusBar()->addPermanentWidget(statusBarResetCountButton);


    connect(statusBarResetCountButton, &QPushButton::clicked, [this] {
        receiveCount = 0;
        sendCount = 0;
        emit writeBytesChanged(sendCount);
        emit readBytesChanged(receiveCount);
    });
}


void MainWindow::openReadWriter() {
    if (_readWriter != nullptr) {
        _readWriter->close();
        delete _readWriter;
        _readWriter = nullptr;
    }
    bool result;

    auto settings = new SerialSettings();
    settings->name = serialPortNameComboBox->currentText();
    settings->baudRate = serialPortBaudRateComboBox->currentText().toInt();

    settings->dataBits = static_cast<QSerialPort::DataBits>(serialPortDataBitsComboBox->currentText().toInt());
    settings->stopBits = static_cast<QSerialPort::StopBits>(serialPortStopBitsComboBox->currentData().toInt());
    settings->parity = static_cast<QSerialPort::Parity>(serialPortParityComboBox->currentData().toInt());
    auto readWriter = new SerialReadWriter(this);
    readWriter->setSerialSettings(*settings);
    qDebug() << settings->name << settings->baudRate << settings->dataBits << settings->stopBits
             << settings->parity;
    result = readWriter->open();
    if (!result) {
        showWarning(tr("消息"), tr("串口被占用或者不存在"));
        return;
    }
    _readWriter = readWriter;

    connect(_readWriter, &SerialReadWriter::readyRead,
            this, &MainWindow::readData);
    emit serialStateChanged(result);
}

void MainWindow::closeReadWriter() {
    stopAutoSend();
    if (_readWriter != nullptr) {
        _readWriter->close();
        delete _readWriter;
        _readWriter = nullptr;
    }
    emit serialStateChanged(false);
}

void MainWindow::createConnect() {

    connect(this, &MainWindow::serialStateChanged, [this](bool isOpen) {
        setOpenButtonText(isOpen);
        QString stateText;
        if (isOpen) {
            stateText = QString("串口打开成功，%1").arg(_readWriter->settingsText());
        } else {
            stateText = QString("串口关闭");
        }

        updateStatusMessage(stateText);
    });

    connect(this, &MainWindow::readBytesChanged, this, &MainWindow::updateReadBytes);
    connect(this, &MainWindow::writeBytesChanged, this, &MainWindow::updateWriteBytes);
    connect(this, &MainWindow::currentWriteCountChanged, this, &MainWindow::updateCurrentWriteCount);

    connect(openSerialButton, &QPushButton::clicked, [=](bool value) {
        Q_UNUSED(value);
        if (!isReadWriterOpen()) {
            openReadWriter();
        } else {
            closeReadWriter();
        }
    });

    connect(saveReceiveDataButton, &QPushButton::clicked, this, &MainWindow::saveReceivedData);
    connect(clearReceiveDataButton, &QPushButton::clicked, this, &MainWindow::clearReceivedData);

    connect(saveSentDataButton, &QPushButton::clicked, this, &MainWindow::saveSentData);
    connect(clearSentDataButton, &QPushButton::clicked, this, &MainWindow::clearSentData);


    connect(autoSendCheckBox, &QCheckBox::clicked, [this] {
        autoSendTimer->stop();
    });

    connect(loopSendCheckBox, &QCheckBox::stateChanged, [this] {
        _loopSend = loopSendCheckBox->isChecked();
    });

    connect(resetLoopSendButton, &QPushButton::clicked, [this] {
        currentSendCount = 0;
        emit currentWriteCountChanged(currentSendCount);
    });

    connect(currentSendCountLineEdit, &QLineEdit::editingFinished, [this] {
        bool ok;
        auto newCount = currentSendCountLineEdit->text().toInt(&ok);
        if (ok) {
            currentSendCount = newCount;
        } else {
            currentSendCountLineEdit->setText(QString::number(currentSendCount));
        }
    });

    connect(sendAllButton, &QPushButton::clicked, [this](bool value) {
        Q_UNUSED(value);
                if (!isReadWriterConnected()) {
                    handlerSerialNotOpen();
                    return;
                }

                if (autoSendState == AutoSendState::Sending) {
                    stopAutoSend();
                } else {
                    updateSendData(sendHexCheckBox->isChecked(), sendTextEdit->toPlainText());
                    updateSendType();
                    sendNextData();
                    startAutoSendTimerIfNeed();
                }
                if (autoSendState == AutoSendState::Sending) {
                    sendAllButton->setText("停止");
                } else {
                    resetSendButtonText();
                }
            }

    );

    connect(autoSendTimer, &QTimer::timeout,
            [this] {
        sendNextData();
            });

    connect(this, SIGNAL(serialPortChanged(bool, QString)), this, SLOT(serialPortUpdate(bool, QString)));
}

void MainWindow::setOpenButtonText(bool isOpen) {
    if (isOpen) {
        openSerialButton->setText(tr("关闭"));
    } else {
        openSerialButton->setText("打开");
    }
}

void MainWindow::createActions() {
}

void MainWindow::createMenu() {

}

void MainWindow::open() {

}

void MainWindow::save() {

}

void MainWindow::tool() {

}

void MainWindow::openDataValidator() {

}

void MainWindow::displayReceiveData(const QByteArray &data) {

    if (pauseReceiveCheckBox->isChecked()) {
        return;
    }

    static QString s;

    s.clear();

    if (addReceiveTimestampCheckBox->isChecked()) {
        s.append("[").append(getTimestamp()).append("] ");
    }

    if (displayReceiveDataAsHexCheckBox->isChecked()) {
        s.append(dataToHex(data));
    } else {
        s.append(QString::fromLocal8Bit(data));
    }

    if (addLineReturnCheckBox->isChecked() || addReceiveTimestampCheckBox->isChecked()) {
        receiveDataBrowser->append(s);
    } else {
        auto text = receiveDataBrowser->toPlainText();
        text.append(s);
        receiveDataBrowser->setText(text);
        auto cursor = receiveDataBrowser->textCursor();
        receiveDataBrowser->moveCursor(QTextCursor::End);
    }

}

void MainWindow::displaySentData(const QByteArray &data) {
    if (displaySendDataAsHexCheckBox->isChecked()) {
        sendDataBrowser->append(dataToHex(data));
    } else {
        sendDataBrowser->append(QString::fromLocal8Bit(data));
    }
}

void MainWindow::openFrameInfoSettingDialog() {

}

void MainWindow::sendNextData() {
    auto data = serialController->getNextFrame();
    if (data.isEmpty()) {
        return;
    }

    if (isReadWriterConnected()) {
        writeData(data);
    } else {
        handlerSerialNotOpen();
    }
}

void MainWindow::updateSendData(bool isHex, const QString &text) {
    if (serialController != nullptr) {
        serialController->setIsHex(isHex);
        serialController->setData(text);
        totalSendCount = serialController->getTotalCount();
        updateTotalSendCount(totalSendCount);
    }
}


void MainWindow::readSettings() {

    qDebug() << "readSettings";

    QSettings settings("Zhou Jinlong", "Serial Wizard");

    settings.beginGroup("SerialSettings");
    auto nameIndex = settings.value("name", 0).toInt();
    auto baudRateIndex = settings.value("baud_rate", 0).toInt();
    auto dataBitsIndex = static_cast<QSerialPort::DataBits>(settings.value("data_bits", 0).toInt());
    auto stopBitsIndex = static_cast<QSerialPort::StopBits>(settings.value("stop_bits", 0).toInt());
    auto parityIndex = static_cast<QSerialPort::Parity>(settings.value("parity", 0).toInt());
    auto sendText = settings.value("send_text", "").toString();

    serialPortNameComboBox->setCurrentIndex(nameIndex);
    serialPortBaudRateComboBox->setCurrentIndex(baudRateIndex);
    serialPortDataBitsComboBox->setCurrentIndex(dataBitsIndex);
    serialPortStopBitsComboBox->setCurrentIndex(stopBitsIndex);
    serialPortParityComboBox->setCurrentIndex(parityIndex);

    settings.beginGroup("SerialReceiveSettings");
    auto addLineReturn = settings.value("add_line_return", true).toBool();
    auto displayReceiveDataAsHex = settings.value("display_receive_data_as_hex", false).toBool();
    auto addTimestamp = settings.value("add_timestamp", false).toBool();

    addLineReturnCheckBox->setChecked(addLineReturn);
    displayReceiveDataAsHexCheckBox->setChecked(displayReceiveDataAsHex);
    addReceiveTimestampCheckBox->setChecked(addTimestamp);

    settings.beginGroup("SerialSendSettings");
    auto sendAsHex = settings.value("send_as_hex", false).toBool();
    auto displaySendData = settings.value("display_send_data", false).toBool();
    auto displaySendDataAsHex = settings.value("display_send_data_as_hex", false).toBool();
    auto autoSend = settings.value("auto_send", false).toBool();
    auto autoSendInterval = settings.value("auto_send_interval", 100).toInt();

    auto loopSend = settings.value("loop_send", false).toBool();

    sendHexCheckBox->setChecked(sendAsHex);
    displaySendDataCheckBox->setChecked(displaySendData);
    displaySendDataAsHexCheckBox->setChecked(displaySendDataAsHex);
    autoSendCheckBox->setChecked(autoSend);
    loopSendCheckBox->setChecked(loopSend);
    sendIntervalLineEdit->setText(QString::number(autoSendInterval));

    sendTextEdit->setText(sendText);

    _loopSend = loopSend;

    serialController =new NormalSerialController();
    serialController->setIsHex(sendAsHex);
    serialController->setAutoSend(autoSend);
    serialController->setLoopSend(loopSend);
    serialController->setAutoSendInterval(autoSendInterval);
    serialController->setData(sendText);

    updateSendType();
}

void MainWindow::writeSettings() {

    qDebug() << "writeSettings()";

    QSettings settings("Zhou Jinlong", "Serial Wizard");

    settings.beginGroup("SerialSettings");
    settings.setValue("name", serialPortNameComboBox->currentIndex());
    settings.setValue("baud_rate", serialPortBaudRateComboBox->currentIndex());
    settings.setValue("data_bits", serialPortDataBitsComboBox->currentIndex());
    settings.setValue("stop_bits", serialPortStopBitsComboBox->currentIndex());
    settings.setValue("parity", serialPortParityComboBox->currentIndex());

    settings.setValue("send_text", sendTextEdit->toPlainText());

    settings.beginGroup("SerialReceiveSettings");
    settings.setValue("add_line_return", addLineReturnCheckBox->isChecked());
    settings.setValue("display_receive_data_as_hex", displayReceiveDataAsHexCheckBox->isChecked());
    settings.setValue("add_timestamp", addReceiveTimestampCheckBox->isChecked());

    settings.beginGroup("SerialSendSettings");
    settings.setValue("send_as_hex", sendHexCheckBox->isChecked());
    settings.setValue("display_send_data", displaySendDataCheckBox->isChecked());
    settings.setValue("display_send_data_as_hex", displaySendDataAsHexCheckBox->isChecked());
    settings.setValue("auto_send", autoSendCheckBox->isChecked());
    settings.setValue("auto_send_interval", sendIntervalLineEdit->text().toInt());
    settings.setValue("loop_send", loopSendCheckBox->isChecked());

    settings.sync();

}




void MainWindow::clearReceivedData() {
    receiveDataBrowser->clear();
}

void MainWindow::saveReceivedData() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存接收数据"),
                                                    "/", tr("Text (*.txt)"));
    if (fileName.isEmpty()) {
        return;
    }
    qDebug() << fileName;

    QFile::remove(fileName);

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream in(&file);

        in << receiveDataBrowser->toPlainText().toLocal8Bit();

        file.close();


        if (okToContinue(tr("消息"), tr("接收数据保存成功,是否打开所在文件夹？"))) {
            QProcess::startDetached("explorer.exe /select," + QDir::toNativeSeparators(fileName));
        }

    }
}

void MainWindow::clearSentData() {
    sendDataBrowser->clear();
}

void MainWindow::saveSentData() {
}


void MainWindow::updateSendCount(qint64 count) {
    currentSendCountLineEdit->setText(QString::number(currentSendCount));
    statusBarWriteBytesLabel->setText(QString::number(count));
}

void MainWindow::updateReceiveCount(qint64 count) {
    statusBarReadBytesLabel->setText(QString::number(count));
}

void MainWindow::readData() {
    auto data = _readWriter->readAll();
    if (!data.isEmpty()) {
        displayReceiveData(data);
        receiveCount += data.count();
        updateReceiveCount(receiveCount);
        emit readBytesChanged(receiveCount);
    }
}

qint64 MainWindow::writeData(const QByteArray &data) {
    qint64 count = 0;
    if (!data.isEmpty() && isReadWriterConnected()) {
        count = _readWriter->write(data);
        displaySentData(data);
        sendCount += count;
        emit writeBytesChanged(sendCount);
    }

    return count;
}

void MainWindow::startAutoSendTimerIfNeed() {
    if (autoSendCheckBox->isChecked()) {
        autoSendTimer->start(sendIntervalLineEdit->text().toInt());
        autoSendState = AutoSendState::Sending;
    } else {
        autoSendState = AutoSendState::Finish;
    }
}

void MainWindow::handlerSerialNotOpen() {
    autoSendTimer->stop();
    showMessage(tr("消息"), tr("串口未打开，请打开串口"));
}

void MainWindow::updateStatusMessage(const QString &message) {
    statusBar()->showMessage(message);
}


void MainWindow::updateReadBytes(qint64 bytes) {
    statusBarReadBytesLabel->setText(QString::number(bytes));
}

void MainWindow::updateWriteBytes(qint64 bytes) {
    statusBarWriteBytesLabel->setText(QString::number(bytes));
}

void MainWindow::stopAutoSend() {
    autoSendTimer->stop();
    autoSendState = AutoSendState::Finish;

    resetSendButtonText();
}

void MainWindow::resetSendButtonText() {
    sendAllButton->setText("全部发送");
}

void MainWindow::updateTotalSendCount(qint64 count) {
    totalSendCountLabel->setText(QString::number(count));
}

bool MainWindow::isReadWriterOpen() {
    return _readWriter != nullptr && _readWriter->isOpen();
}

void MainWindow::updateCurrentWriteCount(qint64 count) {
    currentSendCountLineEdit->setText(QString::number(count));
}

bool MainWindow::isReadWriterConnected() {
    return _readWriter != nullptr && _readWriter->isConnected();
}


void MainWindow::showReadData(const QByteArray &data) {
    if (!data.isEmpty()) {
        displayReceiveData(data);
        receiveCount += data.count();
        updateReceiveCount(receiveCount);
        emit readBytesChanged(receiveCount);
    }
}

void MainWindow::showSendData(const QByteArray &data) {
    if (!data.isEmpty() && isReadWriterConnected()) {
        displaySentData(data);
        sendCount += data.count();
        emit writeBytesChanged(sendCount);
    }
}

QStringList MainWindow::getSerialNameList() {

    auto serialPortInfoList = QSerialPortInfo::availablePorts();
    QStringList l;
    for (auto &s:serialPortInfoList){
        l.append(s.portName());
    }
    return l;
}

void MainWindow::updateSendType() {

    SerialController * newController= nullptr;

    newController = new NormalSerialController(serialController);

    if (newController != nullptr) {
        serialController = newController;
    }
}

void MainWindow::serialPortUpdate(bool isInsert, QString serialPortName)
{
    QString text = QString("检测到") + serialPortName + (isInsert ? QString("已插入") : QString("已拔出"));

    serialPortNameComboBox->clear();
    serialPortNameComboBox->addItems(getSerialNameList());

    this->updateStatusMessage(text);
}
