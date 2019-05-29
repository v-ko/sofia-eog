#include "sumfilter.h"

#include <QDebug>

using namespace std;

SumFilter::SumFilter(QCoreApplication *app_, float inputSampleRate_, float outputSampleRate_, float cutoffFrequency_, float flushFrequency_)
{
    app = app_;
    processingData = false;
    inputSampleRate = inputSampleRate_;
    outputSampleRate = outputSampleRate_;
    cutoffFrequency = cutoffFrequency_;
    flushFrequency = flushFrequency_;

    maxSamplesInFilterSum = inputSampleRate/cutoffFrequency;
    intervalForFilteredSamples = inputSampleRate/outputSampleRate;
    intervalForFlushingTheBuffer = inputSampleRate/flushFrequency;

    if(flushFrequency>outputSampleRate){
        cerr<<"Flush frequency is greater than the output sample rate. Terminating.";
        app->exit(-1);
    }

    samplesInFilterSum = 0;
    filteredSamplesWritten=0;
    rawSamplesProcessed=0;

    filterSum = 0;
    sample=0;

    stdinFile.open(stdin,QIODevice::ReadOnly);
    stdoutFile.open(stdout,QIODevice::WriteOnly);

    //---Testing---
    //stdinFile.setFileName("/home/p10/Dropbox/Pepi/C++/EOGfilter/build-EOGfilter-Desktop_Qt_5_2_1_GCC_64bit-Debug/raw_input.dat");
    //stdinFile.open(QIODevice::ReadOnly);

    //stdoutFile.setFileName("/home/p10/Desktop/sum_filter.dat");
    //stdoutFile.open(QIODevice::WriteOnly);
    //-------------

    rawStream.setDevice(&stdinFile);
    filteredStream.setDevice(&stdoutFile);

    rawStream.setByteOrder(QDataStream::LittleEndian);
    filteredStream.setByteOrder(QDataStream::LittleEndian);

    processDataTimer.setInterval(50);
    connect(&processDataTimer,SIGNAL(timeout()),this,SLOT(processData()));

    processDataTimer.start();
}
SumFilter::~SumFilter()
{
    stdinFile.close();
    stdoutFile.close();
}

void SumFilter::processData()
{
    if(processingData==true){
        return; //so there's only one instance of the func
    }else{
        processingData = true;
    }

    while( !rawStream.atEnd() ){
        rawStream>>sample;

        rawSamplesProcessed++;

        filterSum += sample;
        samplesInFilterSum++;

        sample_buf.push_back(sample);
        if(sample_buf.size()>maxSamplesInFilterSum){
            sample_buf.pop_front();
        }

        if( (float(rawSamplesProcessed)/intervalForFilteredSamples) >= filteredSamplesWritten)
        {
            filteredStream<<filterSum;
            filteredSamplesWritten++;
        }

        if( float(rawSamplesProcessed)/intervalForFlushingTheBuffer >= flushesMade){
            stdoutFile.flush();
            //qDebug()<<"Flushing";
            flushesMade++;
        }

        app->processEvents();

        if(samplesInFilterSum>=maxSamplesInFilterSum)
        {
            filterSum -= sample_buf.front();
            samplesInFilterSum--;
        }
    }
    processingData = false;
    app->processEvents();
}
