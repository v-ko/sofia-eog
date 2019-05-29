#ifndef EOGWINDOW_H
#define EOGWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QSettings>

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
    QList<QCheckBox*> filterCheckBoxes;

    //No deletion on destruct
    QApplication *app;
    EOGLibrary *lib;

signals:

public:
    //Variables
    QSettings settings;

    //Functions
    explicit EOGWindow(QApplication *app_, EOGLibrary *lib_);
    ~EOGWindow();

public slots:
    void requestStorageDirFromUser();
    void clearSettingsAndExit();
    void restoreDefaultFilterConfigurations(){
        lib->restoreDefaultFilterConfigurations();
    }
    void toggleRecording(bool startRecording);

    int updateRecordsList();
    int updateFilterUIbits();
};

#endif // EOGWINDOW_H
