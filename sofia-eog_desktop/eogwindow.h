#ifndef EOGWINDOW_H
#define EOGWINDOW_H

#include <QMainWindow>
#include <QCheckBox>

#include "../eoglibrary.h"
#include "eogfilterwidget.h"

namespace Ui {
class EOGWindow;
}

class EOGWindow : public QMainWindow
{
    Q_OBJECT

private:
    //For deletion
    Ui::EOGWindow *ui;
    QList<EOGFilterWidget*> filterWidget; //get's deleted by EOGWindow which is its parent
    QList<QCheckBox*> filterCheckBox;

    //No deletion on destruct
    QApplication *app;
    EOGLibrary *lib;

signals:

public:
    //Variables

    //Functions
    explicit EOGWindow(QApplication *app_, EOGLibrary *lib_);
    ~EOGWindow();

public slots:
    void requestStorageDirFromUser();
    void clearSettingsAndExit();
    void restoreDefaultFilterConfigurations();
    void toggleRecording(bool startRecording);

    //int updateInputDevicesList();
    int updateRecordsList();
    int updateFilterUIbits();
};

#endif // EOGWINDOW_H
