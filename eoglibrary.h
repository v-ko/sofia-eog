#ifndef EOGLIBRARY_H
#define EOGLIBRARY_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QSettings>
#include "eogfilter.h"
#include <QDir>

class Record
{
public:
    //Variables
    QString name;
    QList<EOGFilter*> filters;

    //Functions
    QString filledArgumentsListStringForFilter(EOGFilter * filter)
    {
        QString argumentsList_tmp = filter->argumentsListString;

        //Set the input sample rate to the output rate of the parent
        EOGFilter * inputFilter = getFilterByIdentifier(filter->inputIdentifier);
        if(inputFilter!=nullptr){
            argumentsList_tmp.replace(QRegExp("InputSampleRate"), QString::number(inputFilter->outputSampleRate) );
        }
        //Set the output sample rate
        argumentsList_tmp.replace(QRegExp("OutputSampleRate"),QString::number(filter->outputSampleRate) );
        //Set the flush frequency
        if( (filter->flushFrequency>0) ){ //If the freq is set (!=0)
            argumentsList_tmp.replace(QRegExp("FlushFrequency"),QString::number(filter->flushFrequency) );
        }
        return argumentsList_tmp;
    }

    EOGFilter * getFilterByIdentifier(QString identifier)
    {
        if(identifier.isEmpty()){
            qDebug()<<"[Record "<<name<<"] No filter with identifier: "<<identifier;
            return nullptr;
        }

        for(EOGFilter *filter: filters){
            if( filter->identifier==identifier ) return filter;
        }
        qDebug()<<"[EOGLibrary::getFilterByIdentifier]No filter found with identifier:"<<identifier;
        return nullptr;
    }
};

class EOGLibrary : public QObject
{
    Q_OBJECT

public:
    explicit EOGLibrary(QString recordsFolder_ = QString(), QString filtersFolder_ = QString());

    QString recordsFolder();
    QString filtersFolder();
    float flushDataFilesInterval();

    QList<EOGFilter*> filtersList(QString folder, QString recordsFolder);

    //Functions
    EOGFilter * getFilterByIdentifier(QString);
    Record * getRecordByName(QString);

    //Variables
    QSettings settings;
    QString dateTimeFormat = "yyyy.MM.dd HH:mm:ss.zzz";

signals:
    void flushDataFilesIntervalChanged(float);
    void recordsListChanged();

    //Other
    void recordingStarted();

public slots:
    //Set properties
    void setRecordsFolder(QString dir);
    void setFiltersFolder(QString dir);
    void setFlushDataFilesInterval(float flushDataFilesInterval_);

    void setRecordsList(QList<Record> recordsList_);

    //Functions
    void updateRecordsList();
    void renameRecord(Record * record, QString newName){
        //Rename the folder
        QDir recordDir(QDir(recordsFolder()).filePath(record->name));
        if( !recordDir.rename(recordDir.absolutePath(), QDir(recordsFolder()).filePath(newName)) ){
            qDebug()<<"An error occured while trying to rename folder:"<<recordDir.absolutePath();
        }
        record->name = newName;
        record->filters = filtersList(recordDir.absolutePath(), recordDir.absolutePath());
        emit recordsListChanged();
    }

    Record * newRecord(QString recordName);
    void stopFilter(EOGFilter *filter);
    void startPipingData();
    void stopPipingData();
    void startRecording(QString recordName);
    void stopRecording();

    void restoreDefaultFilterConfigurations();
    void pipeData();
    void flushDataFilesBuffers();

public:
    QList<Record> recordsList;

    //Variables
    QTimer pipeDataTimer, flushDataFilesTimer;

    //Functions
    QList<EOGFilter *> orderFilters(QList<EOGFilter *> filtersList_);
};

#endif // EOGLIBRARY_H
