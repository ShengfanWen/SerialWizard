//
// Created by chang on 2017-07-28.
//

#ifndef SERIALWIZARD_MAINWINDOW_H
#define SERIALWIZARD_MAINWINDOW_H

class QPushButton;

class QComboBox;

class QTextBrowser;

class QCheckBox;

class QLineEdit;

class QLabel;

class QTextEdit;

class QTimer;

class SerialSettings;

class SerialReadWriter;

class QRadioButton;

class QButtonGroup;

class SerialController;

#include <QtWidgets/QMainWindow>

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void init();

    void initUi();

    void createConnect();

    void createStatusBar();

    ~MainWindow() override;

public:

signals:

    void serialStateChanged(bool);

    void writeBytesChanged(qint64 bytes);

    void readBytesChanged(qint64 bytes);

    void currentWriteCountChanged(qint64 count);

    void serialPortChanged(bool isInsert, QString serialPortName);

public slots:

    void openReadWriter();

    void closeReadWriter();

    void sendNextData();

    void readData();

    qint64 writeData(const QByteArray &data);

    void setOpenButtonText(bool isOpen);

    void displayReceiveData(const QByteArray &data);

    void displaySentData(const QByteArray &data);

    void open();

    void save();

    void tool();

    void openDataValidator();

    void openFrameInfoSettingDialog();

    void clearReceivedData();

    void saveReceivedData();

    void clearSentData();

    void saveSentData();

    void handlerSerialNotOpen();

    void updateStatusMessage(const QString &message);

    void updateReadBytes(qint64 bytes);

    void updateWriteBytes(qint64 bytes);

    void updateCurrentWriteCount(qint64 count);

    void updateSendType();

    void serialPortUpdate(bool isInsert, QString serialPortName);

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;


private:


    enum class AutoSendState {
        NotStart, Sending, Finish
    };

    bool isReadWriterOpen();

    bool isReadWriterConnected();

    void readSettings();

    void writeSettings();

    void createActions();

    void createMenu();

    void updateSendData(bool isHex, const QString &text);

    void updateSendCount(qint64 count);

    void updateReceiveCount(qint64 count);

    void startAutoSendTimerIfNeed();

    void stopAutoSend();

    void resetSendButtonText();

    void updateTotalSendCount(qint64 count);

    void showReadData(const QByteArray &data);

    void showSendData(const QByteArray &data);

    QStringList getSerialNameList();

    //状态栏
    QLabel *statusBarReadBytesLabel;
    QLabel *statusBarWriteBytesLabel;
    QPushButton *statusBarResetCountButton;

    SerialReadWriter *_readWriter{nullptr};

    qint64 sendCount{0};
    qint64 receiveCount{0};

    QButtonGroup *readWriterButtonGroup;

    // 串口设置
    QComboBox *serialPortNameComboBox;
    QComboBox *serialPortBaudRateComboBox;
    QComboBox *serialPortParityComboBox;
    QComboBox *serialPortDataBitsComboBox;
    QComboBox *serialPortStopBitsComboBox;
    QPushButton *openSerialButton;

    // 接收设置
    QCheckBox *addLineReturnCheckBox;
    QCheckBox *displayReceiveDataAsHexCheckBox;
    QCheckBox *pauseReceiveCheckBox;
    QPushButton *saveReceiveDataButton;
    QPushButton *clearReceiveDataButton;
    QCheckBox *addReceiveTimestampCheckBox;

    // 发送设置
    QCheckBox *sendHexCheckBox;
    QCheckBox *displaySendDataCheckBox;
    QCheckBox *displaySendDataAsHexCheckBox;
    QCheckBox *autoSendCheckBox;
    QLineEdit *sendIntervalLineEdit;
    QPushButton *saveSentDataButton;
    QPushButton *clearSentDataButton;

    QCheckBox *loopSendCheckBox;
    QPushButton *resetLoopSendButton;
    QLineEdit *currentSendCountLineEdit;
    QLabel *totalSendCountLabel;

    QTextBrowser *receiveDataBrowser;
    QTextBrowser *sendDataBrowser;

    QTextEdit *sendTextEdit;

    QPushButton *sendAllButton;

    AutoSendState autoSendState{AutoSendState::NotStart};

    QTimer *autoSendTimer{nullptr};


    SerialController *serialController{nullptr};

    int currentSendCount{0};
    int totalSendCount{0};

    bool _loopSend{false};

};


#endif //SERIALWIZARD_MAINWINDOW_H
