#include "eogwindow.h"
#include "ui_eogwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QDateTime>

EOGWindow::EOGWindow(QApplication *app_, EOGLibrary *lib_) :
    ui(new Ui::EOGWindow),
    app(app_),
    lib(lib_),
    settings("org.p10.sofia")
{
    ui->setupUi(this);
    connect(ui->recordsListWidget,SIGNAL(itemSelectionChanged()),this,SLOT(updateFilterUIbits()));
    connect(lib,SIGNAL(recordsListChanged()),this,SLOT(updateRecordsList()));
    connect(ui->recordButton,SIGNAL(toggled(bool)),this,SLOT(toggleRecording(bool)));
    //Disable the listview widget while recording to prevent switching of the selection and breaking stuff
    connect(ui->recordButton,SIGNAL(toggled(bool)),ui->recordsListWidget,SLOT(setDisabled(bool)));
    connect(app, &QApplication::aboutToQuit, lib, &EOGLibrary::stopRecording);
}

EOGWindow::~EOGWindow()
{
    delete ui;
    for(auto cb: filterCheckBoxes) delete cb;
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
    settings.clear();
    settings.sync();
    app->exit(0);
}

void EOGWindow::toggleRecording(bool startRecording)
{
    if(startRecording){
        ui->recordsListWidget->selectionModel()->clearSelection();

        QString newRecordName = QDateTime::currentDateTime().toString(lib->dateTimeFormat);
        lib->renameRecord(lib->getRecordByName("New record"), newRecordName);

        //The line below selects the last item excluding the "New record" item (the one we created).
        //It can be done more stylish I guess
        //(this invokes setting the filters of the FilterWidgets to the new record
        //(its the records count - 2 because the index starts from 0 and the New record is last
        ui->recordsListWidget->selectionModel()->select(ui->recordsListWidget->model()->index(ui->recordsListWidget->count()-2,0),QItemSelectionModel::Select);
        lib->startRecording(newRecordName);

    }else{
        lib->stopRecording();
    }
}

int EOGWindow::updateRecordsList()
{
    ui->recordsListWidget->clear();

    for( Record record: lib->recordsList ){
        ui->recordsListWidget->addItem( record.name );
    }
    if(lib->getRecordByName("New record")==nullptr){
        lib->newRecord("New record");
        updateRecordsList();
    }

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
    for(QCheckBox *checkBox: filterCheckBoxes){
        ui->filtersLayout->removeWidget(checkBox);
        delete checkBox;
    }
    filterCheckBoxes.clear();

    QList<EOGFilter*> filtersList;
    QList<QListWidgetItem*> selectedRecordsList = ui->recordsListWidget->selectedItems();

    //Create all widgets
    if(selectedRecordsList.size()==1){
        bool newRecordIsSelected = selectedRecordsList[0]->text()=="New record";

        Record * record = lib->getRecordByName(selectedRecordsList[0]->text());
        filtersList = record->filters;

        for(EOGFilter *filter: filtersList)
        {
            //Skip filters that have no data files | Not in use because those shouldn't be stored in the record at this time
            //if( (!newRecordIsSelected) && (filter->pathToDataFile.isEmpty()) ){
            //    continue;
            //}
            EOGFilterWidget * fw = new EOGFilterWidget(filter, this, newRecordIsSelected);
            QCheckBox * checkBox = new QCheckBox(this);

            filterWidget.push_back(fw);
            filterCheckBoxes.push_back(checkBox);

            ui->filtersLayout->addWidget(fw);
            ui->chooseFiltersFrame->layout()->addWidget(checkBox);

            //Make connections
            connect(checkBox,SIGNAL(toggled(bool)),fw,SLOT(setVisible(bool)));
            connect(checkBox,SIGNAL(toggled(bool)),filter,SLOT(setIsEnabled(bool)));
            //connect(fw,SIGNAL(filterStartRequest(EOGFilter*)),lib,SLOT(startFilter(EOGFilter*)));

            checkBox->setText(filter->name);
            checkBox->setChecked(filter->isEnabled);
            fw->setVisible(filter->isEnabled); //donno why it doesn't get called with the call above

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

