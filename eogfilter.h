#ifndef EOGFILTER_H
#define EOGFILTER_H

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QDateTime>

class EOGFilter : public QObject
{
    Q_OBJECT

public:
    EOGFilter(QString configurationFilePath_, QString recordFolderPath);
    bool isStarted();

    //Variables
    QString dateTimeFormat = "yyyy.MM.dd HH:mm:ss.zzz";

signals:
    //Property changes
    void isBrokenChanged(bool);
    void isStartedChanged(bool);

    void outputBufferUpdated(QByteArray*);
    void errorBufferUpdated(QString);

public slots:
    //Property set-s
    int setConfigurationFilePath(QString);
    int setPathToExecutable(QString);

    int setIdentifier(QString);
    int setFlushFrequency(float);
    void setIsBroken(bool);
    void setRecordsToFile(bool records){
        recordsToFile = records;
        saveFilterConfig();
    }
    void setIsEnabled(bool enabled){
        isEnabled = enabled;
    }

    void updateOutputBuffer(QByteArray *outputBuffer_);
    void updateErrorBuffer(QString errorBuffer_);

    //Functions
    int saveFilterConfig();
    bool parseConfigurationFile();
    void writeBufferToFile(QByteArray *buffer);
    void flushDataFileBuffer();
    void start(QString argumentsList);
    void stop();
    void handleQProcessError();
    int q_get_value_for_key(QString string, QString key, QString& result);
    int q_get_value_for_key(QString string, QString key, float& result);
    int q_get_value_for_key(QString string, QString key, int& result);
    int q_get_value_for_key(QString string, QString key, unsigned int& result);
    int q_get_value_for_key(QString string, QString key, bool& result);
    int q_get_value_for_key(QString string, QString key, QStringList& results);

public:
    //Properties
    QString configurationFilePath;
    QString pathToExecutable;
    QString pathToDataFile;

    QString argumentsListString;
    QString name;
    QString identifier;
    QString inputIdentifier;
    float outputSampleRate;
    float displaySampleRate;
    float flushFrequency = 0;
    unsigned int sampleSize;
    QDateTime firstSampleTime;

    bool recordsToFile;
    bool isEnabled;
    bool isBroken = false;

    QByteArray *outputBuffer = nullptr;
    QString errorBuffer;
    QProcess filterProcess;
    QFile dataFile;

};

#endif // EOGFILTER_H
