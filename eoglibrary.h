#ifndef EOGLIBRARY_H
#define EOGLIBRARY_H

#include <QObject>
//#include <QAudioDeviceInfo>
#include <QTimer>

#include "eogfilter.h"

class EOGLibrary : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString organizationName READ organizationName WRITE setOrganizationName NOTIFY organizationNameChanged)
    Q_PROPERTY(QString applicationName READ applicationName WRITE setApplicationName NOTIFY applicationNameChanged)
    Q_PROPERTY(QString applicationVersion READ applicationVersion WRITE setApplicationVersion NOTIFY applicationVersionChanged)

    Q_PROPERTY(QString recordsFolder READ recordsFolder WRITE setRecordsFolder NOTIFY recordsFolderChanged)
    Q_PROPERTY(QString defaultFiltersFolder READ defaultFiltersFolder WRITE setDefaultFiltersFolder NOTIFY defaultFiltersFolderChanged)
    Q_PROPERTY(float flushDataFilesInterval READ flushDataFilesInterval WRITE setFlushDataFilesInterval NOTIFY flushDataFilesIntervalChanged)

    Q_PROPERTY(QList<EOGRecord> recordsList READ recordsList WRITE setRecordsList NOTIFY recordsListChanged)
    Q_PROPERTY(QList<EOGFilter*> defaultFiltersList READ defaultFiltersList WRITE setDefaultFiltersList NOTIFY defaultFiltersListChanged)
    //Q_PROPERTY(QList<QAudioDeviceInfo> availableInputDevices READ availableInputDevices WRITE setAvailableInputDevices NOTIFY availableInputDevicesChanged)

    //Q_PROPERTY(QString selectedRecord READ selectedRecord WRITE setSelectedRecord NOTIFY selectedRecordChanged)

public:
    explicit EOGLibrary(QString recordsFolder_, QString filtersFolder_);

    struct EOGRecord{
        QString name;
        QList<EOGFilter*> filter;
    };

    //Properties
    QString organizationName();
    QString applicationName();
    QString applicationVersion();

    QString recordsFolder();
    QString defaultFiltersFolder();
    float flushDataFilesInterval();

    QList<EOGRecord> recordsList();
    QList<EOGFilter*> defaultFiltersList();
    //QList<QAudioDeviceInfo> availableInputDevices();

    //Functions
    EOGFilter * getFilterByIdentifier(QString);
    EOGRecord * getRecordByName(QString);

    //Other
    QString dateTimeFormat;

signals:
    //Properties
    void organizationNameChanged();
    void applicationNameChanged();
    void applicationVersionChanged();

    void recordsFolderChanged(QString);
    void defaultFiltersFolderChanged(QString);
    void flushDataFilesIntervalChanged(float);

    void recordsListChanged();
    void defaultFiltersListChanged();
    //void availableInputDevicesChanged();

    //Other
    void recordingStarted();

public slots:
    //Set properties
    void setOrganizationName(QString organizationName_);
    void setApplicationName(QString applicationName_);
    void setApplicationVersion(QString applicationVersion_);

    void setRecordsFolder(QString dir);
    void setDefaultFiltersFolder(QString dir);
    void setFlushDataFilesInterval(float flushDataFilesInterval_);

    void setRecordsList(QList<EOGRecord> recordsList_);
    void setDefaultFiltersList(QList<EOGFilter*> defaultFiltersList_);
    //void setAvailableInputDevices(QList<QAudioDeviceInfo> availableInputDevices_);

    //Functions
    void updateRecordsList();
    void updateDefaultFiltersList();
    //void updateAvailableInputDevices();

    EOGLibrary::EOGRecord * newRecord(QString recordName);
    void startFilter(EOGFilter *filter);
    void stopFilter(EOGFilter *filter);
    void startPipingData();
    void stopPipingData();
    void startRecording();
    void stopRecording();

    void restoreDefaultFilterConfigurations();
    void pipeData();
    void flushDataFilesBuffers();

private:
    //Properties
    QString organizationName_m;
    QString applicationName_m;
    QString applicationVersion_m;

    QList<EOGRecord> recordsList_m;
    QList<EOGFilter*> defaultFiltersList_m;

    //QList<QAudioDeviceInfo> availableInputDevices_m;

    //Variables
    bool verboseOutput;
    QTimer pipeDataTimer, flushDataFilesTimer;

    //Functions
    QList<EOGFilter *> orderFilters(QList<EOGFilter *> filtersList_);
    QString filledArgumentsListStringForFilter(EOGFilter *);    
};

#endif // EOGLIBRARY_H
