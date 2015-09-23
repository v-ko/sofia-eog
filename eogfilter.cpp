#include "eogfilter.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "../../petko10q.h"

void EOGFilter::commonConstructor(QString configurationFilePath_)
{
    outputBuffer_m = NULL;
    flushFrequency_m = 0; //because it can be omitted in the config file
    dateTimeFormat = "yyyy.MM.dd HH:mm:ss.zzz";

    setIsBroken(false);
    connect(this,SIGNAL(configurationFilePathChanged(QString)),this,SLOT(parseConfigurationFile()));
    connect(this,SIGNAL(outputBufferChanged(QByteArray*)),this,SLOT(writeBufferToFile(QByteArray*)));
    connect(this,SIGNAL(pathToDataFileChanged(QString)),this,SLOT(setDataFile(QString)));
    connect(&filterProcess_m,SIGNAL(stateChanged(QProcess::ProcessState)),this,SLOT(updateIsStarted(QProcess::ProcessState)));
    connect(&filterProcess_m,SIGNAL(error(QProcess::ProcessError)),this,SLOT(handleQProcessError()));

    setConfigurationFilePath(configurationFilePath_);
}
EOGFilter::EOGFilter(QString configurationFilePath_)
{
    commonConstructor(configurationFilePath_);
}
EOGFilter::EOGFilter(EOGFilter *filter, QString recordDir)
{
    QString confFilePath=QDir(recordDir).absoluteFilePath(filter->name()+".filter");
    QString dataFilePath=QDir(recordDir).absoluteFilePath(filter->name()+".dat");

    //Check if the configuration file exists
    if(QFile(confFilePath).exists()){
        //Set conf file
        commonConstructor(confFilePath);

        //if filter is not broken
        if(!isBroken()){
            //check for compliance to the current one
            if( (pathToExecutable()!=filter->pathToExecutable()) |
                    (argumentsListString()!=filter->argumentsListString()) |
                    (inputIdentifier()!=filter->inputIdentifier()) |
                    (outputSampleRate()!=filter->outputSampleRate()) |
                    (sampleSize()!=filter->sampleSize()) )
            {
                setIsBroken(true);
                qDebug()<<"[EOGFilter::EOGFilter(EOGFilter *filter, QString recordDir)]The constructed filter differs from the given one.";
            }
        }
    }else{ //else error
        commonConstructor(filter->configurationFilePath());
        if(recordsToFile()){
            saveAs(confFilePath);
        }
    }

    //Set data file
    setPathToDataFile(dataFilePath);
}

QString EOGFilter::configurationFilePath()
{
    return configurationFilePath_m;
}
QString EOGFilter::pathToExecutable()
{
    return pathToExecutable_m;
}
QString EOGFilter::pathToDataFile()
{
    return pathToDataFile_m;
}

QString EOGFilter::argumentsListString()
{
    return argumentsListString_m;
}
QString EOGFilter::name()
{
    return name_m;
}
QString EOGFilter::identifier()
{
    return identifier_m;
}
QString EOGFilter::inputIdentifier()
{
    return inputIdentifier_m;
}
float EOGFilter::outputSampleRate()
{
    return outputSampleRate_m;
}
float EOGFilter::flushFrequency()
{
    if(flushFrequency_m<=outputSampleRate_m){
        return flushFrequency_m;
    }else{
        return 0;
    }
}
unsigned int EOGFilter::sampleSize()
{
    return sampleSize_m;
}
QDateTime EOGFilter::firstSampleTime()
{
    return firstSampleTime_m;
}

float EOGFilter::displaySampleRate()
{
    return displaySampleRate_m;
}

bool EOGFilter::recordsToFile()
{
    return recordsToFile_m;
}
bool EOGFilter::isEnabled()
{
    return isEnabled_m;
}
bool EOGFilter::isBroken()
{
    return isBroken_m;
}
bool EOGFilter::isStarted()
{
    return isStarted_m;
}

QByteArray * EOGFilter::outputBuffer()
{
    return outputBuffer_m;
}
QString EOGFilter::errorBuffer()
{
    return errorBuffer_m;
}

QProcess * EOGFilter::filterProcess()
{
    return &filterProcess_m;
}

int EOGFilter::setConfigurationFilePath(QString configurationFilePath_)
{
    if( QFile(configurationFilePath_).exists() ){
        configurationFilePath_m=configurationFilePath_;
        emit configurationFilePathChanged(configurationFilePath_);
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setConfigurationFilePath]The configuration file"<<configurationFilePath_<<" does not exist.";
        return -1;
    }
}
int EOGFilter::setPathToExecutable(QString pathToExecutable_)
{
    if( QFile(pathToExecutable_).exists() ){
        pathToExecutable_m=pathToExecutable_;
        emit pathToExecutableChanged(pathToExecutable_);
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setPathToExecutable]The executable"<<pathToExecutable_<<" does not exist.";
        return -1;
    }
}
void EOGFilter::setPathToDataFile(QString pathToDataFile_)
{
    pathToDataFile_m = pathToDataFile_;
    emit pathToDataFileChanged(pathToDataFile_);
}

void EOGFilter::setArgumentsListString(QString argumentsListString_)
{
    argumentsListString_m=argumentsListString_;
    emit argumentsListStringChanged(argumentsListString_);
}
int EOGFilter::setName(QString name_)
{
    if( !name_.isEmpty() ){
        name_m=name_;
        emit nameChanged(name_);
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setName]An empty name is given, returning.";
        return -1;
    }
}
int EOGFilter::setIdentifier(QString identifier_)
{
    if( !identifier_.isEmpty() ){
        identifier_m=identifier_;
        emit identifierChanged(identifier_);
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setIdentifier]An empty identifier is given, returning.";
        return -1;
    }
}
void EOGFilter::setInputIdentifier(QString inputIdentifier_)
{
    inputIdentifier_m=inputIdentifier_;
    emit inputIdentifierChanged(inputIdentifier_);
}
int EOGFilter::setOutputSampleRate(float outputSampleRate_)
{
    if( outputSampleRate_>0 ){
        outputSampleRate_m=outputSampleRate_;
        emit outputSampleRateChanged(outputSampleRate_);
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setOutputSampleRate]An invalid samplerate is given.";
        return -1;
    }

}
int EOGFilter::setDisplaySampleRate(float displaySampleRate_)
{
    if( displaySampleRate_>0 ){
        displaySampleRate_m=displaySampleRate_;
        save();
        emit displaySampleRateChanged(displaySampleRate_);
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setDisplaySampleRate]An invalid samplerate is given:"<<displaySampleRate_;
        return -1;
    }
}
int EOGFilter::setFlushFrequency(float flushFrequency_)
{
    if( (flushFrequency_>0) && (flushFrequency_<=outputSampleRate_m) ){
        flushFrequency_m=flushFrequency_;
        //save();
        emit flushFrequencyChanged(flushFrequency_);
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setFlushFrequency]An invalid flush frequency is given:"<<flushFrequency_;
        return -1;
    }
}

int EOGFilter::setSampleSize(unsigned int sampleSize_)
{
    if( (sampleSize_>0) && (8>=sampleSize_)){
        sampleSize_m=sampleSize_;
        emit sampleSizeChanged(sampleSize_);
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setSampleSize]An invalid sample size is given:"<<sampleSize_;
        return -1;
    }
}
int EOGFilter::setFirstSampleTime(QDateTime firstSampleTime_)
{
    if(firstSampleTime_.isValid()){
        firstSampleTime_m = firstSampleTime_;
        emit firstSampleTimeChanged(firstSampleTime_);
        return 0;
    }else{
        //qDebug()<<"[EOGFilter::setFirstSampleTime]Invalid DateTime given."; teq vs trqbva da se izmestqt izvyn funkciite
        return -1;
    }
}

void EOGFilter::setRecordsToFile(bool recordsToFile_)
{
    recordsToFile_m=recordsToFile_;
    save();
    emit recordsToFileChanged(recordsToFile_);
}
void EOGFilter::setIsEnabled(bool isEnabled_)
{
    isEnabled_m = isEnabled_;
    save();
    emit isEnabledChanged(isEnabled_);
}
void EOGFilter::setIsBroken(bool isBroken_)
{
    isBroken_m = isBroken_;
    emit isBrokenChanged(isBroken_);
}
void EOGFilter::setIsStarted(bool isStarted_)
{
    isStarted_m = isStarted_;
    emit isStartedChanged(isStarted_);
}

void EOGFilter::setOutputBuffer(QByteArray *outputBuffer_)
{
    delete outputBuffer_m;
    outputBuffer_m = outputBuffer_;
    emit outputBufferChanged(outputBuffer_);
}
void EOGFilter::setErrorBuffer(QString errorBuffer_)
{
    errorBuffer_m = errorBuffer_;
    emit errorBufferChanged(errorBuffer_);
}

void EOGFilter::setDataFile(QString path)
{
    if( !dataFile.isOpen() ){
        dataFile.setFileName(path);
    }else{
        qDebug()<<"[EOGFilter::setFile]Trying to set file path while file is open.";
    }
}

int EOGFilter::save()
{
    return saveAs(configurationFilePath());
}
int EOGFilter::saveAs(QString newPath )
{
    QFile file(newPath);
    QTextStream out(&file);

    if( file.open(QIODevice::WriteOnly) ){

        out<<"#This file defines a filter for the sofia-eog library"<<"\n";
        out<<"#See the help on defining filters at FIXME"<<"\n\n";

        out<<"#Static"<<"\n";
        out<<"Exec="<<pathToExecutable()<<"\n";
        out<<"Arguments="<<argumentsListString()<<"\n";
        out<<"Identifier="<<identifier()<<"\n";
        out<<"InputIdentifier="<<inputIdentifier()<<"\n";
        out<<"OutputSampleRate="<<outputSampleRate()<<"\n";
        out<<"\n";

        out<<"#Dynamic (can be changed by the program)"<<"\n";
        out<<"RecordsToFile="<<recordsToFile()<<"\n";
        out<<"DisplaySampleRate="<<displaySampleRate()<<"\n";
        if( (flushFrequency()!=0) ){ //it means that the freq is set and is valid
            out<<"FlushFrequency="<<flushFrequency()<<"\n";
        }
        out<<"SampleSize="<<sampleSize()<<"\n";
        out<<"IsEnabled="<<isEnabled()<<"\n";

        out<<"FirstSampleDateTime=";
        if(firstSampleTime().isValid()){
            out<<firstSampleTime().toString(dateTimeFormat);
        }

        out<<"\n";


        file.close();

        if( newPath!=configurationFilePath() ){
            setConfigurationFilePath(newPath);
        }
        return 0;
    }else{
        qDebug()<<"[EOGFilter::saveAs]Error when opening the file '"<<newPath<<"'for writing";
        return -1;
    }
}

bool EOGFilter::parseConfigurationFile()
{
    QFile file(configurationFilePath());
    QByteArray byteArr;
    QString confFileString,tmpQString;
    float tmpFloat;
    bool tmpBool;
    unsigned int tmpUInt;

    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"[EOGFilter::parseConfigurationFile]Error opening configuration file: "<<configurationFilePath();
        return false;
    }

    //Get the filter name from the file name
    if( setName( QFileInfo(file).baseName() )<0 ){
        qDebug()<<"[EOGFilter::parseConfigurationFile]Error setting name for filter"<<QFileInfo(file).baseName();
        setIsBroken(true);
    }

    //Get the .filter file into a string
    byteArr=file.readAll();
    file.close();
    confFileString = QString(byteArr);

    //Extract the rest of the properties
    if( q_get_value_for_key(confFileString,"Exec",tmpQString)==0 ){
        if ( setPathToExecutable(tmpQString)<0 ){
            qDebug()<<"[EOGFilter::parseConfigurationFile]Couldn't setPathToExecutable for filter"<<name();
            setIsBroken(true);
        }
    }else{
        qDebug()<<"[EOGFilter::parseConfigurationFile]No value for key 'Exec' in filter"<<name();
        setIsBroken(true);
    }

    if( q_get_value_for_key(confFileString,"Arguments",tmpQString)==0 ){
        setArgumentsListString(tmpQString);
    }else{
        qDebug()<<"[EOGFilter::parseConfigurationFile]No value for key 'Arguments' in filter"<<name();
        setIsBroken(true);
    }

    if( q_get_value_for_key(confFileString,"Identifier",tmpQString)==0 ){
        if( setIdentifier(tmpQString)<0 ){
            qDebug()<<"[EOGFilter::parseConfigurationFile]Couldn't setIdentifier for filter"<<name();
            setIsBroken(true);
        }
    }else{
        qDebug()<<"[EOGFilter::parseConfigurationFile]No value for key 'Identifier' in filter"<<name() ;
        setIsBroken(true);
    }

    if( q_get_value_for_key(confFileString,"InputIdentifier",tmpQString)==0 ){
        setInputIdentifier(tmpQString);
    }//no problem if its missing

    if( q_get_value_for_key(confFileString, "OutputSampleRate", tmpFloat)==0 ){
        if( setOutputSampleRate(tmpFloat)<0 ){
            qDebug()<<"[EOGFilter::parseConfigurationFile]Couldn't setInputIdentifier for filter"<<name();
            setIsBroken(true);
        }
    }else{
        qDebug()<<"[EOGFilter::parseConfigurationFile]No value for key 'OutputSampleRate' in filter"<<name();
        setIsBroken(true);
    }

    if( q_get_value_for_key(confFileString, "DisplaySampleRate", tmpFloat)==0 ){
        if( setDisplaySampleRate(tmpFloat)<0 ){
            qDebug()<<"[EOGFilter::parseConfigurationFile]Couldn't setDisplaySampleRate for filter"<<name();
            setIsBroken(true);
        }
    }else{
        qDebug()<<"[EOGFilter::parseConfigurationFile]No value for key 'DisplaySampleRate' in filter"<<name();
        setIsBroken(true);
    }

    if( q_get_value_for_key(confFileString, "FlushFrequency", tmpFloat)==0 ){
        if( setFlushFrequency(tmpFloat)<0 ){
            qDebug()<<"[EOGFilter::parseConfigurationFile]Couldn't setFlushFrequency for filter"<<name();
            setIsBroken(true);
        }
    }//no problem if its missing

    if( q_get_value_for_key(confFileString, "SampleSize", tmpUInt)==0 ){
        if(setSampleSize(tmpUInt)<0){
            qDebug()<<"[EOGFilter::parseConfigurationFile]Couldn't setSampleSize for filter"<<name();
            setIsBroken(true);
        }
    }else{
        qDebug()<<"[EOGFilter::parseConfigurationFile]No value for key 'SampleSize' in filter"<<name();
        setIsBroken(true);
    }

    if( q_get_value_for_key(confFileString, "FirstSampleDateTime", tmpQString)==0 ){
        if(tmpQString.isEmpty()){ //the filters in the conf dir have no start datetime
            setFirstSampleTime(QDateTime());
        }else{
            if(setFirstSampleTime(QDateTime::fromString(tmpQString,dateTimeFormat))<0){
                qDebug()<<"[EOGFilter::parseConfigurationFile]setFirstSampleTimeInvalid returns negative on DateTime:"<<tmpQString<<"in filter"<<name();
                setIsBroken(true);
            }
        }
    }//no problem if its missing

    if( q_get_value_for_key(confFileString, "RecordsToFile", tmpBool)==0 ){
        setRecordsToFile(tmpBool);
    }else{
        qDebug()<<"[EOGFilter::parseConfigurationFile]No value for key 'RecordsToFile' in filter"<<name();
        setIsBroken(true);
    }

    if( q_get_value_for_key(confFileString, "IsEnabled", tmpBool)==0 ){
        setIsEnabled(tmpBool);
    }else{
        qDebug()<<"[EOGFilter::parseConfigurationFile]No value for key 'IsEnabled' in filter"<<name();
        setIsBroken(true);
    }

    if(isBroken()){
        qDebug()<<"[EOGFilter::parseConfigurationFile]The filter"<<name()<<"is broken.";
        return false;
    }
    else return true;
}
void EOGFilter::writeBufferToFile(QByteArray* buffer)
{
    if(recordsToFile()){
        qint64 bytesWritten = dataFile.write(*buffer);
        if( bytesWritten<0 ){
            qDebug()<<"[EOGFilter::writeBufferToFile]Could not write buffer to file:"<<pathToDataFile();
        }else{
            qDebug()<<"[EOGFilter::writeBufferToFile]Bytes written to"<<pathToDataFile()<<bytesWritten;
        }
    }
}
void EOGFilter::flushDataFileBuffer()
{
    if( !dataFile.flush() ){
        qDebug()<<"[EOGFilter::flushDataFileBuffer]Unsuccessful flush";
    }
}

void EOGFilter::start(QString argumentsList)
{
    if( isEnabled() && (!isBroken()) ){
        setFirstSampleTime(QDateTime::currentDateTime());
        if(recordsToFile()){
            saveAs(configurationFilePath());
        }

        qDebug()<<"[EOGFilter::start]Executing"<<pathToExecutable()<<"with arguments:"<<argumentsList;
        filterProcess_m.start(pathToExecutable(),argumentsList.split(QString(" "),QString::SkipEmptyParts));

        if(recordsToFile()){
            if( !dataFile.open(QIODevice::WriteOnly) ){
                qDebug()<<"[EOGFilter::start]Error opening"<<dataFile.fileName()<<":"<<dataFile.errorString();
            }
        }
    }else{
        qDebug()<<"[EOGFilter::start]Trying to start disabled or broken filter:"<<name();
    }
}
void EOGFilter::stop()
{
    if( !filterProcess_m.state()==QProcess::NotRunning ){
        filterProcess_m.terminate();
    }else{
        qDebug()<<"[EOGFilter::stop]Trying to stop a process which is not started for filter:"<<configurationFilePath();
    }
    if(dataFile.isOpen()){
        if(dataFile.flush()) qDebug()<<"[EOGFilter::stop]Error while flushing dataFile for filter"<<name();
        dataFile.close();
    }
}
void EOGFilter::updateIsStarted(QProcess::ProcessState state)
{
    if(state==QProcess::Running){
        setIsStarted(true);
    }else{
        setIsStarted(false);
    }
}
void EOGFilter::handleQProcessError()
{
    setErrorBuffer(filterProcess_m.errorString());
}
