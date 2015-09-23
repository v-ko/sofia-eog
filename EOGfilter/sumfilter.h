#ifndef SUMFILTER_H
#define SUMFILTER_H

#include <iostream>

#include <QCoreApplication>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QTimer>

//Tips:
//>Use cerr for error output
//>connect(&stdinFile,SIGNAL(readyRead()),this,SLOT(processData())); DOESNT work with stdin
//>When processData starts execution pretty much doesn't get back into the main loop
//(app->processEvents doesn't seem to help, but that remains unclear)

class SumFilter : public QObject
{
    Q_OBJECT
private:
    float inputSampleRate, outputSampleRate, cutoffFrequency,flushFrequency;
    int samplesInFilterSum, maxSamplesInFilterSum;
    float intervalForFilteredSamples, intervalForFlushingTheBuffer;
    int filteredSamplesWritten, flushesMade;
    int rawSamplesProcessed;

    qint32 filterSum;
    qint16 sample;
    QList<qint16> sample_buf;

    QFile stdinFile, stdoutFile, stderrFile;
    QDataStream rawStream, filteredStream;

    QTimer processDataTimer;

    QCoreApplication * app;
    bool processingData;

public:
    //Functions
    SumFilter(QCoreApplication * app_, float inputSampleRate_, float outputSampleRate_, float cutoffFrequency_, float flushFrequency_);
    ~SumFilter();

public slots:
    void processData();
};

#endif // SUMFILTER_H
