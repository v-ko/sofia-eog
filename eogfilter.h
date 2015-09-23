#ifndef EOGFILTER_H
#define EOGFILTER_H

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QDateTime>

class EOGFilter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString configurationFilePath READ configurationFilePath WRITE setConfigurationFilePath NOTIFY configurationFilePathChanged)
    Q_PROPERTY(QString pathToExecutable READ pathToExecutable WRITE setPathToExecutable NOTIFY pathToExecutableChanged)
    Q_PROPERTY(QString pathToDataFile READ pathToDataFile WRITE setPathToDataFile NOTIFY pathToDataFileChanged)

    Q_PROPERTY(QString argumentsListString READ argumentsListString WRITE setArgumentsListString NOTIFY argumentsListStringChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QString inputIdentifier READ inputIdentifier WRITE setInputIdentifier NOTIFY inputIdentifierChanged)
    Q_PROPERTY(float outputSampleRate READ outputSampleRate WRITE setOutputSampleRate NOTIFY outputSampleRateChanged)
    Q_PROPERTY(float displaySampleRate READ displaySampleRate WRITE setDisplaySampleRate NOTIFY displaySampleRateChanged)
    Q_PROPERTY(float flushFrequency READ flushFrequency WRITE setFlushFrequency NOTIFY flushFrequencyChanged)
    Q_PROPERTY(unsigned int sampleSize READ sampleSize WRITE setSampleSize NOTIFY sampleSizeChanged)
    Q_PROPERTY(QDateTime firstSampleTime READ firstSampleTime WRITE setFirstSampleTime NOTIFY firstSampleTimeChanged)

    Q_PROPERTY(bool recordsToFile READ recordsToFile WRITE setRecordsToFile NOTIFY recordsToFileChanged)
    Q_PROPERTY(bool isEnabled READ isEnabled WRITE setIsEnabled NOTIFY isEnabledChanged)
    Q_PROPERTY(bool isBroken READ isBroken WRITE setIsBroken NOTIFY isBrokenChanged)
    Q_PROPERTY(bool isStarted READ isStarted WRITE setIsStarted NOTIFY isStartedChanged)

    Q_PROPERTY(QByteArray * outputBuffer READ outputBuffer WRITE setOutputBuffer NOTIFY outputBufferChanged)
    Q_PROPERTY(QString errorBuffer READ errorBuffer WRITE setErrorBuffer NOTIFY errorBufferChanged)

public:
    //Property reads
    QString configurationFilePath();
    QString pathToExecutable();
    QString pathToDataFile();

    QString argumentsListString();
    QString name();
    QString identifier();
    QString inputIdentifier();
    float outputSampleRate();
    float displaySampleRate();
    float flushFrequency();
    unsigned int sampleSize();
    QDateTime firstSampleTime();

    bool recordsToFile();
    bool isEnabled();
    bool isBroken();
    bool isStarted();

    QByteArray * outputBuffer();
    QString errorBuffer();
    QProcess * filterProcess();
    QFile * file();

    //Functions
private:
    void commonConstructor(QString configurationFilePath_);
public:
    EOGFilter(QString configurationFilePath_);
    EOGFilter(EOGFilter * filter, QString recordDir);

    //Variables
    QString dateTimeFormat;

signals:
    //Property changes
    void configurationFilePathChanged(QString);
    void pathToExecutableChanged(QString);
    void pathToDataFileChanged(QString);

    void argumentsListStringChanged(QString);
    void nameChanged(QString);
    void identifierChanged(QString);
    void inputIdentifierChanged(QString);
    void outputSampleRateChanged(float);
    void displaySampleRateChanged(float);
    void flushFrequencyChanged(float);
    void sampleSizeChanged(unsigned int);
    void firstSampleTimeChanged(QDateTime);

    void recordsToFileChanged(bool);
    void isEnabledChanged(bool);
    void isBrokenChanged(bool);
    void isStartedChanged(bool);

    void outputBufferChanged(QByteArray*);
    void errorBufferChanged(QString);
    void filterProcessChanged();

public slots:
    //Property set-s
    int setConfigurationFilePath(QString);
    int setPathToExecutable(QString);
    void setPathToDataFile(QString);

    void setArgumentsListString(QString);
    int setName(QString);
    int setIdentifier(QString);
    void setInputIdentifier(QString);
    int setOutputSampleRate(float);
    int setDisplaySampleRate(float);
    int setFlushFrequency(float);
    int setSampleSize(unsigned int);
    int setFirstSampleTime(QDateTime firstSampleTime_);

    void setRecordsToFile(bool);
    void setIsEnabled(bool);
    void setIsBroken(bool);
    void setIsStarted(bool);

    void setOutputBuffer(QByteArray *outputBuffer_);
    void setErrorBuffer(QString errorBuffer_);

    //Functions
    int save();
    int saveAs(QString newPath);
    bool parseConfigurationFile();
    void writeBufferToFile(QByteArray *buffer);
    void flushDataFileBuffer();
    void start(QString argumentsList);
    void stop();
    void updateIsStarted(QProcess::ProcessState state); //updates the property
    void handleQProcessError();

private:
    //Properties
    QString configurationFilePath_m;
    QString pathToExecutable_m;
    QString pathToDataFile_m;

    QString argumentsListString_m;
    QString name_m;
    QString identifier_m;
    QString inputIdentifier_m;
    float outputSampleRate_m;
    float displaySampleRate_m;
    float flushFrequency_m;
    unsigned int sampleSize_m;
    QDateTime firstSampleTime_m;

    bool recordsToFile_m;
    bool isEnabled_m;
    bool isBroken_m;
    bool isStarted_m;

    QByteArray *outputBuffer_m;
    QString errorBuffer_m;
    QProcess filterProcess_m;
    QFile dataFile;

    //Functions
private slots:
    void setDataFile(QString path);

};

#endif // EOGFILTER_H
