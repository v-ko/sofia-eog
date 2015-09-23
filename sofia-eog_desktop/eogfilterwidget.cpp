#include "eogfilterwidget.h"
#include "ui_eogfilterwidget.h"

#include <QDebug>

EOGFilterWidget::EOGFilterWidget(EOGFilter *filter_, QWidget *parent, bool isPartOfNewRecord) :
    QWidget(parent),
    ui(new Ui::EOGFilterWidget)
{
    ui->setupUi(this);

    lastTimeStamp = 0;

    if(isPartOfNewRecord){
        ui->autoSizeButton->hide();
        ui->stickToEnd->hide();
        ui->errorTextEdit->hide();
        ui->graphic->hide();
        ui->verticalRangeSlider->hide();
    }else{
        ui->startFilterButton->hide();
        ui->recordsToFileCheckBox->hide();
    }

    ui->graphic->addGraph();
    ui->graphic->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->graphic->xAxis->setDateTimeFormat("hh:mm:ss");

    ui->graphic->setInteraction(QCP::iRangeDrag, true);
    ui->graphic->setInteraction(QCP::iRangeZoom, true);
    ui->graphic->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->graphic->axisRect()->setRangeZoom(Qt::Horizontal);

    verticalRangeCenter = 0;
    verticalRangeSize = 2000;

    setFilter(filter_);

    connect(ui->startFilterButton,SIGNAL(clicked()),this,SLOT(emitFilterStartRequest()));
    connect(ui->verticalRangeSlider,SIGNAL(sliderMoved(int)),this,SLOT(replot()));
}

EOGFilterWidget::~EOGFilterWidget()
{
    delete ui;
}

EOGFilter * EOGFilterWidget::filter()
{
    return filter_m;
}

void EOGFilterWidget::setFilter(EOGFilter * filter_)
{
    //FIXME da re6a dali teq raboti otdolo da ne se slojat v edna funkciq koqto da se invoke-va ot setFilter
    disconnect(filter_m);
    ui->graphic->graph(0)->clearData();
    lastTimeStamp = 0;

    filter_m = filter_;

    ui->recordsToFileCheckBox->setChecked(filter_m->recordsToFile());
    ui->displaySampleRateLineEdit->setText(QString::number(filter_m->displaySampleRate()));

    QFile file(filter_m->pathToDataFile());

    if(file.exists()){ //FIXME dobavi progress bar za taq rabota (syotvetno dobavqne na chunk-ove
        file.open(QIODevice::ReadOnly);
        QByteArray tmpByteArray = file.readAll();
        addBufferToData( &tmpByteArray );
        file.close();

    }

    connect(filter_m,SIGNAL(isStartedChanged(bool)),ui->startFilterButton,SLOT(setChecked(bool)));
    connect(filter_m,SIGNAL(errorBufferChanged(QString)),ui->errorTextEdit,SLOT(append(QString)));
    connect(ui->recordsToFileCheckBox,SIGNAL(toggled(bool)),filter_m,SLOT(setRecordsToFile(bool)));
    connect(filter_m,SIGNAL(outputBufferChanged(QByteArray*)),this,SLOT(addBufferToData(QByteArray*)));

    emit filterChanged(filter_);
}

void EOGFilterWidget::emitFilterStartRequest()
{
    emit filterStartRequest(filter_m);
}

void EOGFilterWidget::addBufferToData(QByteArray * buffer_)
{
    if( !ui->enableDisplayingCheckBox->isChecked() ) return;

    //FIXME optimize ?
    QBuffer buffer(buffer_);
    QDataStream stream(&buffer);
    stream.setByteOrder(QDataStream::LittleEndian);

    buffer.open(QIODevice::ReadOnly);
    qint16 int16;
    qint32 int32;

    double sample, sampleTime ;
    if(lastTimeStamp!=0){
        sampleTime = lastTimeStamp;
    }else{
        sampleTime= filter_m->firstSampleTime().toMSecsSinceEpoch()/1000; //it should be in secs so qcustomplot can use it
    }
    double sampleTimeInterval = 1/filter_m->outputSampleRate(); //we're working in msecs (sample rate is in hz)
    unsigned int samplesToSkip=0;

    if(filter_m->outputSampleRate()!=filter_m->displaySampleRate()){
        samplesToSkip = filter_m->outputSampleRate()/filter_m->displaySampleRate();
        //qDebug()<<"Reducing stream size by a factor of"<<samplesToSkip+1<<"for displaying.";
    }

    while(!buffer.atEnd()){

        if(filter_m->sampleSize()==2){
            stream>>int16;
            sample = int16;
        }else if(filter_m->sampleSize()==4){
            stream>>int32;
            sample = int32;
        }else{
            qDebug()<<"Bad sample size.";
        }

        //FIXME catch error here
        stream.skipRawData(filter_m->sampleSize()*samplesToSkip);

        ui->graphic->graph(0)->addData(sampleTime,sample);
        //qDebug()<<"Samples displayed:"<<sampleTime<<","<<sample;
        sampleTime+=sampleTimeInterval*(1+samplesToSkip);
    }
    lastTimeStamp = sampleTime;
    buffer.close();

    replot();
}

void EOGFilterWidget::replot()
{
    if(ui->autoSizeButton->isChecked()){
        verticalRangeCenter = ui->graphic->yAxis->range().center() ;
        verticalRangeSize = ui->graphic->yAxis->range().size();
        ui->graphic->rescaleAxes(true);
    }else{
        if(ui->stickToEnd->isChecked()){
            ui->graphic->xAxis->setRange(lastTimeStamp,ui->graphic->xAxis->range().size(),Qt::AlignRight);
            //ui->graphic->yAxis->setRange(0,8000,Qt::AlignCenter);
        }
        double size = verticalRangeSize * (double(ui->verticalRangeSlider->value())/1000);
        double center = verticalRangeCenter * (double(ui->verticalRangeSlider->value())/1000);
        ui->graphic->yAxis->setRange(center,size,Qt::AlignCenter);
    }

    ui->graphic->replot();
}
