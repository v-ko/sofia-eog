#include "eoglibrary.h"

#include <QDir>
#include <QStandardPaths>
#include <QDateTime>

EOGLibrary::EOGLibrary(QString recordsFolder_, QString filtersFolder_):
    settings("org.p10.sofia")
{
    setRecordsFolder(recordsFolder_);
    setFiltersFolder(filtersFolder_);

    //Setup the timers
    pipeDataTimer.setInterval(settings.value("flushDataFileInterval","50").toFloat());
    connect(&pipeDataTimer,SIGNAL(timeout()),this,SLOT(pipeData()));

    flushDataFilesTimer.setInterval(settings.value("flushDataFileInterval","60").toFloat()*1000);
    connect(&flushDataFilesTimer,SIGNAL(timeout()),this,SLOT(flushDataFilesBuffers()));
}

QString EOGLibrary::recordsFolder()
{
    QString dirStr = settings.value("records_folder","no_dir").toString();
    QDir dir(dirStr);

    //Fix if dir does not exist or is missing
    if( (!dir.exists()) | dirStr.isEmpty() ){
        qDebug()<<"[recordsFolder] The dir retrieved from QSettings does not exist:"<<dirStr;
        dirStr = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        setRecordsFolder( dirStr );
        qDebug()<<"[recordsFolder] Saved "<<dirStr<<" to settings.";

        dir.setPath(dirStr);
        if(!dir.exists()){
            qDebug()<<"The data dir is not present, creating it now.";
            dir.mkpath(dirStr); //TODO catch error
        }
    }

    return settings.value("records_folder").toString();
}
QString EOGLibrary::filtersFolder()
{
    QDir dir(settings.value("filters_folder","no_dir").toString());

    //Fix if dir does not exist or is missing
    if( (!dir.exists())){
        qDebug()<<"[filtersFolder] The dir retrieved from QSettings does not exist:"<<dir.path();
    }

    return settings.value("filters_folder").toString();
}

QList<EOGFilter*> EOGLibrary::filtersList(QString filtersFolder, QString recordFolderPath)
{
    QDir dir( filtersFolder );
    QList<EOGFilter*> filtersList_tmp;

    //For every file in the record folder
    for(QFileInfo entry: dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries) ){

        if( entry.isFile() && (entry.suffix()=="filter") ){
            filtersList_tmp.push_back( new EOGFilter(dir.absoluteFilePath(entry.fileName()), recordFolderPath ) );
        }else{
            qDebug()<<"[EOGLibrary::filtersList]Unattended file/dir detected:"<<entry.fileName();
            //TODO:add to unattended files
        }
    }

    return filtersList_tmp;//orderFilters(filtersList_tmp);
}

//=============Public slots========================

void EOGLibrary::setRecordsFolder(QString dirPath)
{
    QDir dir(dirPath);

    if(dir.exists()){
        qDebug()<<"[EOGLibrary::setRecordsFolder]Setting records folder to:"<<dir.absolutePath();
        settings.setValue("records_folder", dir.absolutePath());
        settings.sync();
    }else{
        qDebug()<<"[EOGLibrary::setRecordsFolder]The given dir does not exist. Creating it.";
    }
}
void EOGLibrary::setFiltersFolder(QString dirPath)
{
    QDir dir(dirPath);

    if(!dir.exists()){
        qDebug()<<"[EOGLibrary::setFiltersFolder]The given dir does not exist. Creating it.";
        dir.mkpath(dirPath); //TODO catch error
        //Install the .filter files
        restoreDefaultFilterConfigurations();
    }
    qDebug()<<"[EOGLibrary::setFiltersFolder]Setting default filters folder to:"<<dir.absolutePath();
    settings.setValue("filters_folder", dir.absolutePath());
    settings.sync();
}
void EOGLibrary::setFlushDataFilesInterval(float flushDataFilesInterval_)
{
    qDebug()<<"[EOGLibrary::setFlushDataFileInterval]Setting flushDataFileInterval to:"<<flushDataFilesInterval_;

    settings.setValue("flushDataFileInterval", flushDataFilesInterval_);
    settings.sync();

    flushDataFilesTimer.setInterval(flushDataFilesInterval_*1000);
}

void EOGLibrary::setRecordsList(QList<Record> recordsList_)
{
    //Delete the old list
    for(Record record: recordsList){
        for(EOGFilter* filter: record.filters){
            delete filter;
        }
    }
    recordsList.clear();

    //Copy the new list
    recordsList = recordsList_;
    emit recordsListChanged();
}

void EOGLibrary::updateRecordsList()
{
    QDir dir( recordsFolder() );
    QList<Record> recordsList_tmp;

    //Get all record folders
    for(QFileInfo recordFolder: dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries) ){
        if( recordFolder.isDir() ){
            recordsList_tmp.push_back(Record());
            recordsList_tmp.back().name = recordFolder.fileName();
            recordsList_tmp.back().filters = filtersList(recordFolder.absoluteFilePath(), recordFolder.absoluteFilePath());

        }else{
            qDebug()<<"[EOGLibrary::updateRecordsList]Unattended file detected:"<<recordFolder.fileName();
            //TODO:add to unattended files
        }
    }

    setRecordsList(recordsList_tmp);
}

Record * EOGLibrary::newRecord(QString recordName)
{
    Record * record;
    QDir dir(recordsFolder());

    if( recordName.isEmpty() ){
        qDebug()<<"[EOGLibrary::newRecord]No recordName specified.";
        return nullptr;
    }else if( getRecordByName(recordName)!=nullptr ){
        qDebug()<<"[EOGLibrary::newRecord]This name is already in use:"<<recordName;
        return nullptr;

    //Create the records dir and filter files
    }else if( dir.mkdir(recordName) ){

        updateRecordsList();
        record = getRecordByName(recordName);
        record->filters = filtersList(filtersFolder(), dir.filePath(recordName));
        for(EOGFilter *f: record->filters){
            f->saveFilterConfig();
        }
        updateRecordsList();
        record = getRecordByName(recordName);

        if(record==nullptr){
            qDebug()<<"[EOGLibrary::newRecord]Failed to fetch record after creating it:"<<recordName;
            return nullptr;
        }else{
            qDebug()<<"[EOGLibrary::newRecord]Created record:"<<recordName;
            emit recordsListChanged();
            return record;
        }
    }else{
        qDebug()<<"[EOGLibrary::newRecord]Failed to create the folder.";
        return nullptr;
    }
}
void EOGLibrary::stopFilter(EOGFilter *filter)
{
    filter->stop();
}
void EOGLibrary::startPipingData()
{
    pipeDataTimer.start();
    flushDataFilesTimer.start();
}
void EOGLibrary::stopPipingData()
{
    pipeDataTimer.stop();
    flushDataFilesTimer.stop();
}
void EOGLibrary::startRecording(QString recordName)
{
    //Create the new record dir
    Record * record = getRecordByName(recordName);
    if(record==nullptr) record = newRecord(recordName);

    if( record!=nullptr ){

        for(EOGFilter *filter: record->filters){
            if(filter->isEnabled){
                filter->start(record->filledArgumentsListStringForFilter(filter));
            }
        }

        //Start piping data
        startPipingData();
        emit recordingStarted();
    }else{
        qDebug()<<"[EOGLibrary::startRecording]Failed to create record. Won't start recording.";
    }
}
void EOGLibrary::stopRecording()
{
    //Brute force stop of everything
    for(Record record: recordsList){
        for(EOGFilter *filter: record.filters){
            if(filter->isStarted()){
                filter->stop();
                //pipeData();
            }
        }
    }

    stopPipingData();

    qDebug()<<"[EOGLibrary::stopRecording]Stopped recording";
}

void EOGLibrary::restoreDefaultFilterConfigurations()
{
    QDir internalFiltersDir(":/filter_configurations"), filtersDir=filtersFolder();
    if( !internalFiltersDir.isReadable() ){
        qDebug()<<"[EOGLibrary::restoreDefaultFilterConfigurations]The internal filters dir is not readable:"<<internalFiltersDir.absolutePath();
    }
    if( !filtersDir.isReadable() ){
        qDebug()<<"[EOGLibrary::restoreDefaultFilterConfigurations]The filters dir is not readable:"<<filtersDir.absolutePath();
    }

    QStringList defaultFiltersList = internalFiltersDir.entryList(QDir::NoDotAndDotDot|QDir::AllEntries);
    QStringList currentFiltersList = filtersDir.entryList(QDir::NoDotAndDotDot|QDir::AllEntries);

    //Remove all present filters (and other entries)
    for(QString currentFilter: currentFiltersList){
        if( !filtersDir.remove(currentFilter) ){
            qDebug()<<"[EOGLibrary::restoreDefaultFilterConfigurations]Could not remove an entry from the filters dir:"<<currentFilter;
        }
    }
    //Copy default filters
    for(QString defaultFilter: defaultFiltersList){
        if( !QFile::copy(internalFiltersDir.absoluteFilePath(defaultFilter),filtersDir.absoluteFilePath(defaultFilter)) ){
            qDebug()<<"[EOGLibrary::restoreDefaultFilterConfigurations]Unsuccessful file copy for:"<<defaultFilter;
        }else{
            QFile(filtersDir.absoluteFilePath(defaultFilter)).setPermissions(QFile::ReadOwner | QFile::WriteOwner);
        }
    }
}

QList<EOGFilter *> EOGLibrary::orderFilters(QList<EOGFilter*> filtersList_)
{
    QList<EOGFilter*> filterList_processed, filterList_left = filtersList_, filterList_leftBackup;

    //Find root nodes (no input set)
    int filtersFound = 0;
    for(EOGFilter *filter: filterList_left)
    {
        if(filter->inputIdentifier.isEmpty()){
            filterList_processed.push_back(filter);
            filterList_left.removeOne(filter);
            filtersFound++;
        }
    }

    //Then try to queue everything else
    filterList_leftBackup = filterList_left;

    while(filtersFound>0){ //If there's none that connects with the tree - we're finished (those left are considered broken)

        filtersFound = 0;

        //FIXME it's kind of a mess the code below
        for(EOGFilter *filter: filterList_leftBackup)
        {
            for(EOGFilter *processedFilter: filterList_processed){
                if(processedFilter->identifier==filter->inputIdentifier){
                    filterList_processed.push_back(filter);
                    filterList_left.removeOne(filter);
                    break;
                }
            }
        }

        filterList_leftBackup = filterList_left;
    }//wend

    for(EOGFilter *filter: filterList_left){
        qDebug()<<"[EOGLibrary::orderFilters]Filter"<<filter->identifier<<"has no connection to a root node, adding to list anyway.";
        filterList_processed.push_back(filter);
        filterList_left.removeOne(filter);
    }

    return filterList_processed;
}
void EOGLibrary::pipeData()
{
    QByteArray *buffer;

    qint64 bytesWritten=0;

    //qDebug()<<"Piping data";

    //Parse outputs
    for(Record record: recordsList){
        for(EOGFilter * filter: record.filters){

            if( !filter->isStarted() ) continue;

            //Get the output in a buffer
            if(filter->filterProcess.bytesAvailable()>0){
                buffer = new QByteArray(filter->filterProcess.readAllStandardOutput()) ; //test it without the constructor

                if(!buffer->isEmpty()){
                    //Push it to every filter that wants this output
                    for(EOGFilter *targetFilter: record.filters){
                        if( !targetFilter->isStarted() ) continue; //only if the filter is started

                        if( targetFilter->inputIdentifier==filter->identifier ){

                            bytesWritten = targetFilter->filterProcess.write(buffer->data(),buffer->size());
                            if( bytesWritten<0 ){
                                qDebug()<<"[EOGLibrary::pipeData]Failed to write the buffer to the targetFilter:"<<targetFilter->configurationFilePath;
                            }else{ //if the write is good
                                //qDebug()<<"Bytes written to"<<filter->name<<":"<<bytesWritten;
                            }
                        }
                    }

                    //Save the buffer for yourself
                    filter->updateOutputBuffer(buffer);
                }
            }//end if bytes available

            //Get the errors
            buffer = new QByteArray( filter->filterProcess.readAllStandardError() );
            if(buffer->size()>0){
                filter->updateErrorBuffer(QString(*buffer));
                qDebug()<<"[EOGLibrary::pipeData]Error buffer for filter"<<filter->name<<":"<<*buffer;
            }
            delete buffer;
        }
    }

}
void EOGLibrary::flushDataFilesBuffers()
{
    for(Record record: recordsList){
        for(EOGFilter * filter: record.filters){

            if( !filter->isStarted() ) continue;

            filter->flushDataFileBuffer();
        }
    }
}


Record * EOGLibrary::getRecordByName(QString name)
{
    for(int i=0;i<recordsList.size();i++){
        Record *record = &recordsList[i];
        if(record->name==name) return record;
    }
    qDebug()<<"[EOGLibrary::getRecordByName]Could not get record with name:"<<name;
    return nullptr;
}
