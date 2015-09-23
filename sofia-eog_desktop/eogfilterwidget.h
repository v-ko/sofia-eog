#ifndef EOGFILTERWIDGET_H
#define EOGFILTERWIDGET_H

#include <QWidget>

#include "../eogfilter.h"

namespace Ui {
class EOGFilterWidget;
}

class EOGFilterWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(EOGFilter * filter READ filter WRITE setFilter NOTIFY filterChanged)

public:
    explicit EOGFilterWidget(EOGFilter * filter_, QWidget *parent, bool isPartOfNewRecord);
    ~EOGFilterWidget();

    //Properties
    EOGFilter * filter();

signals:
    //Property changes
    void filterChanged(EOGFilter*);

    //Other
    void filterStartRequest(EOGFilter* filter_);

public slots:
    //Property sets
    void setFilter(EOGFilter*);

    void emitFilterStartRequest();

    void addBufferToData(QByteArray*buffer_);
    void replot();
private:
    //Properties
    EOGFilter * filter_m;

    //Variables
    double lastTimeStamp;
    double verticalRangeCenter;
    double verticalRangeSize;

    //UI
    Ui::EOGFilterWidget *ui;
};

#endif // EOGFILTERWIDGET_H
