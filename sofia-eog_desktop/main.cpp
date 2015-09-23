#include "eogwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>

//Sorry for the bulgarian below, I'll get to clearing/interpreting those (mostly ideas and bugs)
//I think all of the comments in the actual code are in english.

//CLI display info
//ne6to s dataFile ima problem, za6toto poslediqt flush ne go pravi. Izob6to ne zatvarq dataFile-a kakto tr
//^^^^ moje da probvam da pusna flush s data write-a , koito e dobyr
//EOGfilter procesa ne go spira pri kill-vane na programata
//^^^^ moje da ima vryzka tova 4e pi6e 4e se opitva da spre filtri koito ne sa pusnati

//da sa 4erveni schupenite filtri
//pri stari zapisi da pokazva widgeti samo za grafikite za koito realno ima data
//delete record (+confirm dialog)
//analizira6t filtyr

//opciq za 4etene kakto KST ot file on disk za da moga remote da gledam ot RPi

//Hardware:
//napravi ka4estveni elektrodi ot vryv ~done
//proveri kak se izmenqt poletata kato mesti6 aktivnite elektrodi (aktivniq na uhoto po vyzmojnost)--done
//za6ii samite novi elektrodi da probva6 dali ba4kat i na lentata --done
//da razbera koga/za6to adjeba po4va da shumi s referenten i bez referenten

//Later:
//lame filter
//navsqkyde emit changed pri set, samo ako stoinostta e razli4na
//na filtrite bez input identifier da e "new record" kop4eto a na drugite da e "run filter"
//BUG: ne izkarva error ot arecord
//BUG: cutoff freq ne6to ne se zadava vqrno
//little/big endian property
//da izkaram save-ovete ot setX(x_)v EOGFilter i da gi sloja po-na visoko,4e 6te stavat read/werite loop-ove ina4e
//pri multi select
//ako nqkoi ot available(default) filters e s4upen - checkboxa mu e 4erven. ina4e - v fw widgeta e 4erveno
//updateRecordsListsSignal(float) 0-1
//mp3 kompresiq kato filtyr
//mp3 dekompresiq kato filtyr
//ui design na filter widget-a
//dobavi syotvetnite populate records i filters v eogwindow
//FIXME na nqkoi mesta lipsvat save()-ove sled promqna na config stoinosti na filtrite
//(taka pyk pri signal-slot nqma da bacat) funkciite trqbva samo da vry6tat gre6ka, pyk teq koito gi vikato ako trqbva da pi6at v qDebug


//pri markirane na pove4e grafiki - da gi izkarva vs-te kato gi naslagva vse edno sa v edno denono6tie
//s opcii za priravnqvane po start
//i po predpolagaemo na4alo na synq

//>>get data needed for the filters: sample rates,sizes,etc..
//>>>sample rate-a na arecord programata da go re6ava
//da napisha arecord ekvivalenta 4e si trqbva cross platform
//timezone-a da se pazi. kydeto ne e zapisan - da se zapisva tekushtiq

//vj za FIXME-ta i TODO-ta
//unattended files list

//========Specifications=========

//--------Parameters that the filters accept filtrite-----------
//>Input sample rate - taken from the parent filter
//>Output sample rate - specified by the user

//-------Parameters per filter for the GUI----------------
//>Display sample rate

//-------Parameters for the filter definition (.filter files)-------------
//>Path to executable
//>input from filter 'X'
//>conflicts with 'X,Y,Z'

//-------Parameters that the library needs to set up the filters-------------
//>Record to file or not - from the user
//>Working directory - from the application

//-------Info file parameters------------
//Begginning of the record

//======Other notes========
//Only the filters defined in the filters directory will be visible
//(even if there are older or newer ones defined in the records/ directory)

//========Test cases==========
//Cleared settings
//>No storageDir set

//give bad input to setStorageDir() (?)

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //qDebug()<<"stufff";

    app.setOrganizationName("p10");
    app.setApplicationName("sofia-eog-qt");
    app.setApplicationVersion("0.0.1");

    //==============Command line parser===========================
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt GUI frontend for the sofia-eog library");
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

    // Process the actual command line arguments given by the user
    parser.process(app);

    QString recordsFolder = parser.value(recordsFolderOption);
    QString filtersFolder = parser.value(filtersFolderOption);

    EOGLibrary lib(recordsFolder, filtersFolder);
    EOGWindow w(&app, &lib);

    //Make connections
    //QObject::connect(&lib,SIGNAL(availableInputDevicesChanged()),&w,SLOT(updateInputDevicesList()));
    QObject::connect(&lib,SIGNAL(recordsListChanged()),&w,SLOT(updateRecordsList()));
    QObject::connect(&lib,SIGNAL(defaultFiltersListChanged()),&w,SLOT(updateFilterUIbits()));

    //Initial info gathering
    //lib.updateAvailableInputDevices();
    lib.updateDefaultFiltersList();
    lib.updateRecordsList();
    lib.setFlushDataFilesInterval(1); //TEST

    w.show();

    return app.exec();
}
