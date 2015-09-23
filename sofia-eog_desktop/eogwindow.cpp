#include "eogwindow.h"
#include "ui_eogwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QSettings>
//#include <QStandardPaths>
#include <QAudioDeviceInfo>
#include <QDateTime>

EOGWindow::EOGWindow(QApplication *app_, EOGLibrary *lib_) :
    ui(new Ui::EOGWindow)
{
    ui->setupUi(this);
    app = app_;
    lib = lib_;

    connect(ui->recordsListWidget,SIGNAL(itemSelectionChanged()),this,SLOT(updateFilterUIbits()));
    connect(lib,SIGNAL(recordsListChanged()),this,SLOT(updateRecordsList()));
    connect(ui->recordButton,SIGNAL(toggled(bool)),this,SLOT(toggleRecording(bool)));
    //Disable the listview widget while recording to prevent switching of the selection and breaking stuff
    connect(ui->recordButton,SIGNAL(toggled(bool)),ui->recordsListWidget,SLOT(setDisabled(bool)));
}

EOGWindow::~EOGWindow()
{
    delete ui;
}

//============Public functions===========================
void EOGWindow::requestStorageDirFromUser()
{
    QString dir = QFileDialog::getExistingDirectory(this,tr("Choose a directory to read/write records from/to."));
    if(!dir.isEmpty()){ //If no dir is chosen 'dir' is empty, else it should be a valid dir
        lib->setRecordsFolder( dir );
    }
}
void EOGWindow::clearSettingsAndExit()
{
    QSettings settings(lib->organizationName(),lib->applicationName());
    settings.clear();
    settings.sync();
    app->exit(0);
}

void EOGWindow::restoreDefaultFilterConfigurations()
{
    lib->restoreDefaultFilterConfigurations();
}
void EOGWindow::toggleRecording(bool startRecording)
{
    if(startRecording){
        ui->recordsListWidget->selectionModel()->clearSelection();

        lib->startRecording();

        //A hacky removal of the "New record" item (not needed if the list gets updated)
        //ui->recordsListWidget->removeItemWidget(ui->recordsListWidget->findItems("New record",Qt::MatchExactly)[0]);

        //The line below selects the last item excluding the "New record" item (the one we created).
        //It can be done more stylish I guess
        //(this invokes setting the filters of the FilterWidgets to the new record
        //(its the records count - 2 because the index starts from 0 and the New record is last
        ui->recordsListWidget->selectionModel()->select(ui->recordsListWidget->model()->index(ui->recordsListWidget->count()-2,0),QItemSelectionModel::Select);

    }else{
        lib->stopRecording();
    }
}

/*int EOGWindow::updateInputDevicesList()
{
    int count = 0;

    ui->inputComboBox->clear();
    for( QAudioDeviceInfo devInfo: lib->availableInputDevices() ){
        ui->inputComboBox->addItem( QString::number(count) + devInfo.deviceName() );
        count++;
    }

    return 0;
}*/
int EOGWindow::updateRecordsList()
{
    ui->recordsListWidget->clear();

    for( EOGLibrary::EOGRecord record: lib->recordsList() ){
        ui->recordsListWidget->addItem( record.name );
    }
    ui->recordsListWidget->addItem("New record");

    return 0;
}
int EOGWindow::updateFilterUIbits()
{
    //Remove all current widgets (if any)
    for(EOGFilterWidget *fw: filterWidget){
        ui->filtersLayout->removeWidget(fw);
        delete fw;
    }
    filterWidget.clear();

    //Remove all current checkboxes (if any)
    for(QCheckBox *checkBox: filterCheckBox){
        ui->filtersLayout->removeWidget(checkBox);
        delete checkBox;
    }
    filterCheckBox.clear();

    QList<EOGFilter*> filtersList;
    QList<QListWidgetItem*> selectedRecordsList = ui->recordsListWidget->selectedItems();

    //Create all widgets
    if(selectedRecordsList.size()==1){
        bool newRecordIsSelected = selectedRecordsList[0]->text()=="New record";

        if(newRecordIsSelected){
            //If we're making a new record we'll use the default filters
            filtersList = lib->defaultFiltersList();
        }else{
            //Else we need to display the filters in the record
            EOGLibrary::EOGRecord * record = lib->getRecordByName(selectedRecordsList[0]->text());
            filtersList = record->filter;
        }

        for(EOGFilter *filter: filtersList)
        {
            //Skip filters that have no data files | Not in use because those shouldn't be stored in the record at this time
            //if( (!newRecordIsSelected) && (filter->pathToDataFile().isEmpty()) ){
            //    continue;
            //}
            EOGFilterWidget * fw = new EOGFilterWidget(filter, this, newRecordIsSelected);
            QCheckBox * checkBox = new QCheckBox(this);

            filterWidget.push_back(fw);
            filterCheckBox.push_back(checkBox);

            ui->filtersLayout->addWidget(fw);
            ui->chooseFiltersFrame->layout()->addWidget(checkBox);

            //Make connections
            connect(checkBox,SIGNAL(toggled(bool)),fw,SLOT(setVisible(bool)));
            connect(checkBox,SIGNAL(toggled(bool)),filter,SLOT(setIsEnabled(bool)));
            connect(fw,SIGNAL(filterStartRequest(EOGFilter*)),lib,SLOT(startFilter(EOGFilter*)));

            checkBox->setText(filter->name());
            checkBox->setChecked(filter->isEnabled());
            fw->setVisible(filter->isEnabled()); //donno why it doesn't get called with the call above

            if(selectedRecordsList[0]->text()=="New record"){
                ui->recordButton->setEnabled(true);
            }else{
                if(!ui->recordButton->isChecked())
                ui->recordButton->setEnabled(false);
            }
        }
    }//ELSE THERE's no widget selected - leave it blank

    return 0;
}

