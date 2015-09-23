#include "eoglibrary.h"

#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QStandardPaths>
#include <QDateTime>

EOGLibrary::EOGLibrary(QString recordsFolder_ = QString(), QString filtersFolder_ = QString())
{
    //FOR TESTING
    verboseOutput = true;

    //Basic info
    setOrganizationName("p10");
    setApplicationName("sofia-eog-lib");
    setApplicationVersion("0.0.1");

    dateTimeFormat = "yyyy.MM.dd HH:mm:ss.zzz";

    //If folders are entered - use them
    if( !recordsFolder_.isEmpty() ){
        setRecordsFolder(recordsFolder_);
    }
    if( !filtersFolder_.isEmpty() ){
        setDefaultFiltersFolder(filtersFolder_);
    }

    //Setup the timers
    pipeDataTimer.setInterval(50);
    connect(&pipeDataTimer,SIGNAL(timeout()),this,SLOT(pipeData()));

    flushDataFilesTimer.setInterval(flushDataFilesInterval()*1000);
    connect(&flushDataFilesTimer,SIGNAL(timeout()),this,SLOT(flushDataFilesBuffers()));

    //If the filters list changes we need to update the records list (only available in the conf dir filters are valid)
    connect(this,SIGNAL(defaultFiltersListChanged()),this,SLOT(updateRecordsList()));
    connect(this,SIGNAL(recordsFolderChanged(QString)),this,SLOT(updateRecordsList()));
}

QString EOGLibrary::organizationName()
{
    return organizationName_m;
}
QString EOGLibrary::applicationName()
{
    return applicationName_m;
}
QString EOGLibrary::applicationVersion()
{
    return applicationVersion_m;
}

QString EOGLibrary::recordsFolder()
{
    QSettings settings(organizationName(),applicationName());
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
QString EOGLibrary::defaultFiltersFolder()
{
    QSettings settings(organizationName(),applicationName());
    QString dirStr = settings.value("filters_folder","no_dir").toString();
    QDir dir(dirStr);

    //Fix if dir does not exist or is missing
    if( (!dir.exists()) | dirStr.isEmpty() ){
        qDebug()<<"[filtersFolder] The dir retrieved from QSettings does not exist:"<<dirStr;
        dirStr = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/"+applicationName();
        setDefaultFiltersFolder( dirStr );
        qDebug()<<"[filtersFolder] Saved "<<dirStr<<" to settings.";

        dir.setPath(dirStr);
        if(!dir.exists()){
            qDebug()<<"[EOGLibrary::filtersFolder]The config dir is not present, creating it now.";
            dir.mkpath(dirStr); //TODO catch error
            //Install the .filter files
            restoreDefaultFilterConfigurations();
        }
    }

    return settings.value("filters_folder").toString();
}
float EOGLibrary::flushDataFilesInterval()
{
    QSettings settings(organizationName(),applicationName());
    float flushDataFileInterval_ = settings.value("flushDataFileInterval","60").toFloat();

    //Fix if there is a bad (negative) interval
    if(flushDataFileInterval_<=0){
        setFlushDataFilesInterval(60);
        flushDataFileInterval_ = 60;
        qDebug()<<"[EOGLibrary::flushDataFileInterval] The interval stored in the settings was invalid. Setting to 60.";
    }

    return flushDataFileInterval_;
}

QList<EOGLibrary::EOGRecord> EOGLibrary::recordsList()
{
    return recordsList_m;
}
QList<EOGFilter *> EOGLibrary::defaultFiltersList()
{
    return defaultFiltersList_m;
}
//QList<QAudioDeviceInfo> EOGLibrary::availableInputDevices()
//{
//    return availableInputDevices_m;
//}

//=============Public slots========================
void EOGLibrary::setOrganizationName(QString organizationName_)
{
    organizationName_m = organizationName_;
    emit organizationNameChanged();
}
void EOGLibrary::setApplicationName(QString applicationName_)
{
    applicationName_m = applicationName_;
    emit applicationNameChanged();
}
void EOGLibrary::setApplicationVersion(QString applicationVersion_)
{
    applicationVersion_m = applicationVersion_;
    emit applicationVersionChanged();
}

void EOGLibrary::setRecordsFolder(QString dir)
{
    QSettings settings(organizationName(),applicationName());

    QDir tmpDir(dir);

    if(tmpDir.exists()){
        qDebug()<<"[EOGLibrary::setRecordsFolder]Setting records folder to:"<<tmpDir.absolutePath();

        settings.setValue("records_folder", tmpDir.absolutePath());
        settings.sync();

        emit recordsFolderChanged(tmpDir.absolutePath());
    }else{
        qDebug()<<"[EOGLibrary::setRecordsFolder]The given dir does not exist.";
    }
}
void EOGLibrary::setDefaultFiltersFolder(QString dir)
{
    QSettings settings(organizationName(),applicationName());

    QDir tmpDir(dir);

    if(tmpDir.exists()){
        qDebug()<<"[EOGLibrary::setFiltersFolder]Setting default filters folder to:"<<tmpDir.absolutePath();

        settings.setValue("filters_folder", tmpDir.absolutePath());
        settings.sync();

        emit defaultFiltersFolderChanged(tmpDir.absolutePath());
    }else{
        qDebug()<<"[EOGLibrary::setFiltersFolder]The given dir does not exist.";
    }
}
void EOGLibrary::setFlushDataFilesInterval(float flushDataFilesInterval_)
{
    QSettings settings(organizationName(),applicationName());

    if(flushDataFilesInterval_>0){
        qDebug()<<"[EOGLibrary::setFlushDataFileInterval]Setting flushDataFileInterval to:"<<flushDataFilesInterval_;

        settings.setValue("flushDataFileInterval", flushDataFilesInterval_);
        settings.sync();

        flushDataFilesTimer.setInterval(flushDataFilesInterval_*1000);
        emit flushDataFilesIntervalChanged(flushDataFilesInterval_);
    }else{
        qDebug()<<"[EOGLibrary::setFlushDataFileInterval]The given interval is <=0.";
    }
}

void EOGLibrary::setRecordsList(QList<EOGRecord> recordsList_)
{
    //Delete the old list
    for(EOGRecord record: recordsList_m){
        for(EOGFilter* filter: record.filter){
            delete filter;
        }
    }
    recordsList_m.clear();

    //Copy the new list
    recordsList_m = recordsList_;
    emit recordsListChanged();
}
void EOGLibrary::setDefaultFiltersList(QList<EOGFilter*> defaultFiltersList_)
{
    defaultFiltersList_m = defaultFiltersList_;
    emit defaultFiltersListChanged();
}
/*void EOGLibrary::setAvailableInputDevices(QList<QAudioDeviceInfo> availableInputDevices_)
{
    availableInputDevices_m = availableInputDevices_;
    emit availableInputDevicesChanged();
}*/

void EOGLibrary::updateRecordsList()
{
    QDir dir( recordsFolder() );
    QList<EOGRecord> recordsList_tmp;

    for(QFileInfo entry: dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries) ){

        if( entry.isDir() ){
            recordsList_tmp.push_back(EOGRecord());
            recordsList_tmp.back().name = entry.fileName();
            for(EOGFilter * filter: defaultFiltersList()){
                recordsList_tmp.back().filter.push_back(new EOGFilter(filter, QDir(recordsFolder()).absoluteFilePath(recordsList_tmp.back().name)));
            }

        }else{
            qDebug()<<"[EOGLibrary::updateRecordsList]Unattended file detected:"<<entry.fileName();
            //TODO:add to unattended files
        }
    }

    setRecordsList(recordsList_tmp);
}
void EOGLibrary::updateDefaultFiltersList()
{
    QDir dir( defaultFiltersFolder() );
    QList<EOGFilter*> filtersList_tmp;

    for(EOGFilter* filter: defaultFiltersList() ) delete filter; //clear filters

    for(QFileInfo entry: dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries) ){

        if( entry.isFile() && (entry.suffix()=="filter") ){
            filtersList_tmp.push_back( new EOGFilter(dir.absoluteFilePath(entry.fileName()) ) );
        }else{
            qDebug()<<"[EOGLibrary::updateFiltersList]Unattended dir detected:"<<entry.fileName();
            //TODO:add to unattended files
        }
    }

    setDefaultFiltersList( orderFilters(filtersList_tmp) );
}
/*void EOGLibrary::updateAvailableInputDevices()
{
    setAvailableInputDevices( QAudioDeviceInfo::availableDevices(QAudio::AudioInput) );
}*/

EOGLibrary::EOGRecord * EOGLibrary::newRecord(QString recordName)
{
    EOGLibrary::EOGRecord * record;
    QDir dir(recordsFolder());

    if( recordName.isEmpty() ){
        qDebug()<<"[EOGLibrary::newRecord]No recordName specified.";
        return NULL;
    }else if( getRecordByName(recordName)!=NULL ){
        qDebug()<<"[EOGLibrary::newRecord]This name is already in use:"<<recordName;
        return NULL;
    }else if( dir.mkdir(recordName) ){
        updateRecordsList();
        record = getRecordByName(recordName);
        if(record==NULL){
            qDebug()<<"[EOGLibrary::newRecord]Failed to fetch record after creating it:"<<recordName;
            return NULL;
        }else{
            emit recordsListChanged();
            return record;
        }
    }else{
        qDebug()<<"[EOGLibrary::newRecord]Failed to create the folder.";
        return NULL;
    }
}
void EOGLibrary::startFilter(EOGFilter * filter)
{
    filter->start(filledArgumentsListStringForFilter(filter));
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
void EOGLibrary::startRecording()
{
    QString recordName = QDateTime::currentDateTime().toString(dateTimeFormat);

    //Create the new record dir
    EOGRecord * record = newRecord(recordName);
    if( record!=NULL ){

        for(EOGFilter *filter: record->filter){
            if(filter->isEnabled()){
                filter->start(filledArgumentsListStringForFilter(filter));
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
    for(EOGFilter *filter: defaultFiltersList_m){
        if(filter->isStarted()){
            filter->stop();
            pipeData();
        }
    }
    stopPipingData();

    qDebug()<<"[EOGLibrary::stopRecording]Stopped recording";
}

void EOGLibrary::restoreDefaultFilterConfigurations()
{
    QDir internalFiltersDir(":/filter_configurations"), filtersDir=defaultFiltersFolder();
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
    updateDefaultFiltersList();
}

QList<EOGFilter *> EOGLibrary::orderFilters(QList<EOGFilter*> filtersList_)
{
    QList<EOGFilter*> filterList_processed,filterList_left = filtersList_,filterList_leftBackup;

    //Find filters with no input
    for(EOGFilter *filter: filterList_left)
    {
        if(filter->inputIdentifier().isEmpty()){
            filterList_processed.push_back(filter);
            filterList_left.removeOne(filter);
        }
    }

    //Then try to queue everything else
    int filtersFound = 1; //not to break the while loop

    filterList_leftBackup = filterList_left;

    while(filtersFound>0){ //If there's none that connects with the tree - we're finished (those left are considered broken)

        filtersFound = 0;

        //FIXME it's kind of a mess the code below
        for(EOGFilter *filter: filterList_leftBackup)
        {
            for(EOGFilter *processedFilter: filterList_processed){
                if(processedFilter->identifier()==filter->inputIdentifier()){
                    filterList_processed.push_back(filter);
                    filterList_left.removeOne(filter);
                    break;
                }
            }
        }

        filterList_leftBackup = filterList_left;
    }//wend

    for(EOGFilter *filter: filterList_left){
        qDebug()<<"[EOGLibrary::orderFilters]Filter"<<filter->identifier()<<"is not properly connected to the pipe.";
        delete filter;
    }

    return filterList_processed;
}
void EOGLibrary::pipeData()
{
    QByteArray *buffer;

    qint64 bytesWritten=0;

    //qDebug()<<"Piping data";

    //Parse outputs
    for(EOGRecord record: recordsList()){
        for(EOGFilter * filter: record.filter){

            if( !filter->isStarted() ) continue;

            //Get the output in a buffer
            if(filter->filterProcess()->bytesAvailable()>0){
                buffer = new QByteArray(filter->filterProcess()->readAllStandardOutput()) ; //test it without the constructor

                if(!buffer->isEmpty()){
                    //Push it to every filter that wants this output
                    for(EOGFilter *targetFilter: record.filter){
                        if( !targetFilter->isStarted() ) continue; //only if the filter is started

                        if( targetFilter->inputIdentifier()==filter->identifier() ){

                            bytesWritten = targetFilter->filterProcess()->write(buffer->data(),buffer->size());
                            if( bytesWritten<0 ){
                                qDebug()<<"[EOGLibrary::pipeData]Failed to write the buffer to the targetFilter:"<<targetFilter->configurationFilePath();
                            }else{ //if the write is good
                                //qDebug()<<"Bytes written to"<<filter->name()<<":"<<bytesWritten;
                            }
                        }
                    }

                    //Save the buffer for yourself
                    filter->setOutputBuffer(buffer);
                }
            }//end if bytes available

            //Get the errors
            buffer = new QByteArray( filter->filterProcess()->readAllStandardError() );
            if(buffer->size()>0){
                filter->setErrorBuffer(QString(*buffer));
                qDebug()<<"[EOGLibrary::pipeData]Error buffer for filter"<<filter->name()<<":"<<*buffer;
            }
            delete buffer;
        }
    }

}
void EOGLibrary::flushDataFilesBuffers()
{
    for(EOGRecord record: recordsList()){
        for(EOGFilter * filter: record.filter){

            if( !filter->isStarted() ) continue;

            filter->flushDataFileBuffer();
        }
    }
}

QString EOGLibrary::filledArgumentsListStringForFilter(EOGFilter * filter)
{
    QString argumentsList_tmp = filter->argumentsListString();

    //Set the input sample rate to the output rate of the parent
    EOGFilter * inputFilter = getFilterByIdentifier(filter->inputIdentifier());
    if(inputFilter!=NULL){
        argumentsList_tmp.replace(QRegExp("InputSampleRate"), QString::number(inputFilter->outputSampleRate()) );
    }
    //Set the output sample rate
    argumentsList_tmp.replace(QRegExp("OutputSampleRate"),QString::number(filter->outputSampleRate()) );
    //Set the flush frequency
    if( (filter->flushFrequency()!=0) ){ //If the freq is set (!=0)
        argumentsList_tmp.replace(QRegExp("FlushFrequency"),QString::number(filter->flushFrequency()) );
    }
    return argumentsList_tmp;
}

EOGFilter * EOGLibrary::getFilterByIdentifier(QString identifier)
{
    if(identifier.isEmpty()) return NULL;

    for(EOGFilter * filter: defaultFiltersList()){
        if( filter->identifier()==identifier ) return filter;
    }
    qDebug()<<"[EOGLibrary::getFilterByIdentifier]No filter found with identifier:"<<identifier;
    return NULL;
}
EOGLibrary::EOGRecord * EOGLibrary::getRecordByName(QString name)
{
    for(int i=0;i<recordsList_m.size();i++){
        EOGRecord *record = &recordsList_m[i];
        if(record->name==name) return record;
    }
    //qDebug()<<"[EOGLibrary::getRecordByName]Could not get record with name:"<<name;
    return NULL;
}
