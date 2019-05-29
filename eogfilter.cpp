#include "eogfilter.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "../../petko10q.h"

EOGFilter::EOGFilter(QString configurationFilePath_, QString recordFolderPath):
    configurationFilePath(configurationFilePath_)
{
    connect(this,SIGNAL(outputBufferUpdated(QByteArray*)),this,SLOT(writeBufferToFile(QByteArray*)));
    //connect(&filterProcess_m,SIGNAL(stateChanged(QProcess::ProcessState)),this,SLOT(updateIsStarted(QProcess::ProcessState)));
    connect(&filterProcess,SIGNAL(error(QProcess::ProcessError)),this,SLOT(handleQProcessError()));

    parseConfigurationFile();
    configurationFilePath = QDir(recordFolderPath).absoluteFilePath(name + ".filter");
    dataFile.setFileName(QDir(recordFolderPath).absoluteFilePath(name + ".dat"));
}
bool EOGFilter::isStarted()
{
    if(filterProcess.state()==QProcess::Running){
        return true;
    }else{
        return false;
    }
}

int EOGFilter::setConfigurationFilePath(QString configurationFilePath_)
{
    if( QFile(configurationFilePath_).exists() ){
        configurationFilePath=configurationFilePath_;
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setConfigurationFilePath]The configuration file"<<configurationFilePath_<<" does not exist.for filter"<<name;
        return -1;
    }
}
int EOGFilter::setPathToExecutable(QString pathToExecutable_)
{
    if( QFile(pathToExecutable_).exists() ){
        pathToExecutable=pathToExecutable_;
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setPathToExecutable]The executable"<<pathToExecutable_<<" does not exist.for filter"<<name;
        return -1;
    }
}

int EOGFilter::setIdentifier(QString identifier_)
{
    if( !identifier_.isEmpty() ){
        identifier=identifier_;
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setIdentifier]An empty identifier is given for filter"<<name<<".Returning.";
        return -1;
    }
}
int EOGFilter::setFlushFrequency(float flushFrequency_)
{
    if( (flushFrequency_>0) && (flushFrequency_<=outputSampleRate) ){
        flushFrequency=flushFrequency_;
        //save();
        return 0;
    }else{
        qDebug()<<"[EOGFilter::setFlushFrequency]An invalid flush frequency is given:"<<flushFrequency_<<"for filter"<<name;
        return -1;
    }
}
void EOGFilter::setIsBroken(bool isBroken_)
{
    isBroken = isBroken_;
    emit isBrokenChanged(isBroken_);
}

void EOGFilter::updateOutputBuffer(QByteArray *outputBuffer_)
{
    delete outputBuffer;
    outputBuffer = outputBuffer_;
    emit outputBufferUpdated(outputBuffer_);
}
void EOGFilter::updateErrorBuffer(QString errorBuffer_)
{
    errorBuffer = errorBuffer_;
    emit errorBufferUpdated(errorBuffer_);
}

int EOGFilter::saveFilterConfig()
{
    QFile file(configurationFilePath);
    QTextStream out(&file);

    if( file.open(QIODevice::WriteOnly) ){

        out<<"#This file defines a filter for the sofia-eog library"<<"\n";
        out<<"#See the help on defining filters at FIXME"<<"\n\n";

        out<<"#Static"<<"\n";
        out<<"Exec="<<pathToExecutable<<"\n";
        out<<"Arguments="<<argumentsListString<<"\n";
        out<<"Identifier="<<identifier<<"\n";
        out<<"InputIdentifier="<<inputIdentifier<<"\n";
        out<<"OutputSampleRate="<<outputSampleRate<<"\n";
        out<<"\n";

        out<<"#Dynamic (can be changed by the program)"<<"\n";
        out<<"RecordsToFile="<<recordsToFile<<"\n";
        out<<"DisplaySampleRate="<<displaySampleRate<<"\n";
        if( (flushFrequency!=0) ){ //it means that the freq is set and is valid
            out<<"FlushFrequency="<<flushFrequency<<"\n";
        }
        out<<"SampleSize="<<sampleSize<<"\n";
        out<<"IsEnabled="<<isEnabled<<"\n";

        out<<"FirstSampleDateTime=";
        if(firstSampleTime.isValid()){
            out<<firstSampleTime.toString(dateTimeFormat);
        }

        out<<"\n";


        file.close();

        return 0;
    }else{
        qDebug()<<"[EOGFilter::saveAs]Error when opening the file '"<<configurationFilePath<<"'for writing";
        return -1;
    }
}

bool EOGFilter::parseConfigurationFile()
{
    QFile file(configurationFilePath);
    QByteArray byteArr;
    QString confFileString,tmpQString;
    float tmpFloat;
    bool tmpBool;
    unsigned int tmpUInt;

    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"[EOGFilter::parseConfigurationFile]Error opening configuration file: "<<configurationFilePath;
        return false;
    }

    //Get the filter name from the file name
    name = QFileInfo(file).baseName();

    //Get the .filter file into a string
    byteArr=file.readAll();
    file.close();
    confFileString = QString(byteArr);

    //Extract the rest of the properties
    if( q_get_value_for_key(confFileString,"Exec",tmpQString)==0 ){
        if ( setPathToExecutable(tmpQString)<0 ) setIsBroken(true);
    }else setIsBroken(true);

    if( q_get_value_for_key(confFileString,"Arguments",tmpQString)==0 ){
        argumentsListString = tmpQString;
    }else setIsBroken(true);

    if( q_get_value_for_key(confFileString,"Identifier",tmpQString)==0 ){
        if( setIdentifier(tmpQString)<0 ) setIsBroken(true);
    }else setIsBroken(true);

    if( q_get_value_for_key(confFileString,"InputIdentifier",tmpQString)==0 ){
        inputIdentifier = tmpQString;
    }//no problem if its missing

    if( q_get_value_for_key(confFileString, "OutputSampleRate", tmpFloat)==0 ){
        outputSampleRate = tmpFloat;
    }else setIsBroken(true);

    if( q_get_value_for_key(confFileString, "DisplaySampleRate", tmpFloat)==0 ){
        displaySampleRate = tmpFloat;
    }else setIsBroken(true);

    if( q_get_value_for_key(confFileString, "FlushFrequency", tmpFloat)==0 ){
        if( setFlushFrequency(tmpFloat)<0 ) setIsBroken(true);
    }//no problem if its missing

    if( q_get_value_for_key(confFileString, "SampleSize", tmpUInt)==0 ){
        sampleSize = tmpUInt;
    }else setIsBroken(true);

    if( q_get_value_for_key(confFileString, "FirstSampleDateTime", tmpQString)==0 ){
        QDateTime dateTime(QDateTime::fromString(tmpQString,dateTimeFormat));
        if(!tmpQString.isEmpty() && dateTime.isValid()){ //the filters in the conf dir have no start datetime
            firstSampleTime = dateTime;
        }else{
            setIsBroken(true);
        }

    }//no problem if its missing

    if( q_get_value_for_key(confFileString, "RecordsToFile", tmpBool)==0 ){
        recordsToFile = tmpBool;
    }else setIsBroken(true);

    if( q_get_value_for_key(confFileString, "IsEnabled", tmpBool)==0 ){
        isEnabled = tmpBool;
    }else setIsBroken(true);

    if(isBroken){
        qDebug()<<"[EOGFilter::parseConfigurationFile]The filter"<<name<<"is broken.";
        return false;
    }
    else return true;
}
void EOGFilter::writeBufferToFile(QByteArray* buffer)
{
    if(recordsToFile){
        qint64 bytesWritten = dataFile.write(*buffer);
        if( bytesWritten<0 ){
            qDebug()<<"[EOGFilter::writeBufferToFile]Could not write buffer to file:"<<pathToDataFile;
        }else{
            qDebug()<<"[EOGFilter::writeBufferToFile]Bytes written to"<<pathToDataFile<<bytesWritten;
        }
    }
}
void EOGFilter::flushDataFileBuffer()
{
    if(dataFile.bytesToWrite()>0){
        if( !dataFile.flush() ){
            qDebug()<<"[EOGFilter::flushDataFileBuffer]Unsuccessful flush";
        }
    }
}

void EOGFilter::start(QString argumentsList)
{
    if( isEnabled && (!isBroken) ){
        firstSampleTime = QDateTime::currentDateTime();

        saveFilterConfig();

        qDebug()<<"[EOGFilter::start]Executing"<<pathToExecutable<<"with arguments:"<<argumentsList;
        filterProcess.start(pathToExecutable, argumentsList.split(QString(" "),QString::SkipEmptyParts));

        if(recordsToFile){
            if( !dataFile.open(QIODevice::WriteOnly) ){
                qDebug()<<"[EOGFilter::start]Error opening"<<dataFile.fileName()<<":"<<dataFile.errorString();
            }
        }
        emit isStartedChanged(true);
    }else{
        qDebug()<<"[EOGFilter::start]Trying to start disabled or broken filter:"<<name;
    }
}
void EOGFilter::stop()
{
    if( filterProcess.state()!=QProcess::NotRunning ){
        filterProcess.kill();
        qDebug()<<"Sent SIGKILL to process:"<<filterProcess.pid();
    }else{
        qDebug()<<"Preocess state:"<<filterProcess.state();
        qDebug()<<"[EOGFilter::stop]Trying to stop a process which is not started for filter:"<<configurationFilePath;
    }
    if(dataFile.isOpen()){
        if(dataFile.flush()) qDebug()<<"[EOGFilter::stop]Error while flushing dataFile for filter"<<name;
        dataFile.close();
    }
    emit isStartedChanged(false);
}
void EOGFilter::handleQProcessError()
{
    updateErrorBuffer(filterProcess.errorString());
}

int EOGFilter::q_get_value_for_key(QString string, QString key, QString& result) //string should be a list of INI type key-value pairs
{
    QStringList line;

    key+="="; //to fit the startsWith test more acurately

    line = string.split("\n",QString::SkipEmptyParts);

    for(int i=0;i<line.size();i++){
        if(line[i].startsWith(key)){

            result=q_get_text_between(line[i],'=',0);

            return 0;
        }
    }

    qDebug()<<"[EOGFilter]No value for key '"<<key<<"' in filter"<<name;
    return -1;
}

int EOGFilter::q_get_value_for_key(QString string, QString key, float& result)
{
    QString txt;
    int err = q_get_value_for_key(string,key,txt);
    if (err<0) return err;
    result = txt.toFloat();
    return 0;
}

int EOGFilter::q_get_value_for_key(QString string, QString key, int& result)
{
    QString txt;
    int err = q_get_value_for_key(string,key,txt);
    if (err<0) return err;
    result = txt.toInt();
    return 0;
}

int EOGFilter::q_get_value_for_key(QString string, QString key, unsigned int& result)
{
    QString txt;
    int err = q_get_value_for_key(string,key,txt);
    if (err<0) return err;
    result = txt.toUInt();
    return 0;
}

int EOGFilter::q_get_value_for_key(QString string, QString key, bool& result)
{
    QString txt;
    int err = q_get_value_for_key(string,key,txt);
    if (err<0) return err;
    int res = txt.toInt();
    if( (res==1) | (res==0) ){result = res;}
    else return -1;
    return 0;
}

int EOGFilter::q_get_value_for_key(QString string, QString key, QStringList& results)
{
    QString txt;
    int err = q_get_value_for_key(string,key,txt);
    if (err<0) return err;
    results = txt.split(";",QString::SkipEmptyParts);
    //Trim the names
    for(QString &result :results){
        result = result.trimmed();
    }
    return 0;
}
