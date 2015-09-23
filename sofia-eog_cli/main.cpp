#include <iostream>
#include <csignal>
#include <unistd.h>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>

#include "../eoglibrary.h"

//opravi .filter file-ovete --done
//test desktop --done
//build RPi --done
//record to file every ~second --done
// ^C to stop recording and exit normally --done
//da izkarva vs errorBuffer-i (na filtrite) pri zatvarqne nai-malko

QCoreApplication *ap;

void handleInterruptSignal(int sigNum)
{
    qDebug()<<"Called interrupt with index"<<sigNum;
    ap->quit();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    app.setOrganizationName("p10");
    app.setApplicationName("sofia-eog-cli");
    app.setApplicationVersion("0.0.1");

    //==============Command line parser===========================
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt CLI frontend for the sofia-eog library");
    parser.addHelpOption();
    parser.addVersionOption();

    // Records folder option
    QCommandLineOption recordsFolderOption(QStringList() << "r" << "records-folder",
            app.translate("main", "Set the records folder to <records-folder>."),
            app.translate("main", "records-folder"));
    parser.addOption(recordsFolderOption);

    // Filters folder option
    QCommandLineOption filtersFolderOption(QStringList() << "f" << "filters-folder",
            app.translate("main", "Set the filters folder to <filters-folder>."),
            app.translate("main", "filters-folder"));
    parser.addOption(filtersFolderOption);

    // Flush data files interval option
    QCommandLineOption flushDataFilesIntervalOption(QStringList() << "i" << "flush-interval",
            app.translate("main", "Set the interval to flush the buffer to file to <flush-interval> in seconds."),
            app.translate("main", "flush-interval"));
    parser.addOption(flushDataFilesIntervalOption);

    // Start recording option
    QCommandLineOption startOption(QStringList() << "s" << "start",
            app.translate("main", "Start recording."));
    parser.addOption(startOption);

    // Restore default filters option
    QCommandLineOption restoreDefaultFiltersOption(QStringList() << "d" << "default-filters",
            app.translate("main", "Restore the default filters to the filters folder."));
    parser.addOption(restoreDefaultFiltersOption);

    // Print all settings option
    QCommandLineOption printSettingsOption(QStringList() << "p" << "print-settings",
            app.translate("main", "Print the settings."));
    parser.addOption(printSettingsOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    QString recordsFolder = parser.value(recordsFolderOption);
    QString filtersFolder = parser.value(filtersFolderOption);
    float flushDataFilesInterval = parser.value(flushDataFilesIntervalOption).toFloat();

    QDir recordsDir(recordsFolder);
    QDir filtersDir(filtersFolder);

    //Create the folders if they're missing
    if( !recordsDir.exists() ){
        if( !recordsDir.mkdir(recordsDir.absolutePath()) ){
            qDebug()<<"Failed to create dir:"<<recordsFolder;
            return -1;
        }
    }
    if( !filtersDir.exists() ){
        if( !filtersDir.mkdir(filtersDir.absolutePath()) ){
            qDebug()<<"Failed to create dir:"<<filtersFolder;
            return -1;
        }
    }

    EOGLibrary lib(recordsFolder, filtersFolder);

    //Set the interval on which to flush the buffers to file
    if(lib.flushDataFilesInterval()!=flushDataFilesInterval){
        lib.setFlushDataFilesInterval(flushDataFilesInterval);
    }

    //If the user wants to restore the default filters
    if(parser.isSet(restoreDefaultFiltersOption)){
        lib.restoreDefaultFilterConfigurations();
    }

    if(parser.isSet(printSettingsOption)){
        qDebug()<<"Records folder:"<<lib.recordsFolder();
        qDebug()<<"Default filters folder:"<<lib.defaultFiltersFolder();
        qDebug()<<"Flush data file interval:"<<lib.flushDataFilesInterval();
        //qDebug<<"Records folder:"<<lib.recordsFolder();
    }

    //Initial info gathering
    //lib.updateAvailableInputDevices();
    lib.updateDefaultFiltersList();
    lib.updateRecordsList();

    //Connections
    QObject::connect(&app,SIGNAL(aboutToQuit()),&lib,SLOT(stopRecording()));

    //Start recording
    if(parser.isSet(startOption)){
        lib.startRecording();
    }else{
        app.processEvents();
        app.exit();
    }

    //Handle the CTRL+C signal (SIGINT)
    ap = &app;
    signal (SIGINT, handleInterruptSignal);

    //Start the event loop
    return app.exec();
}
