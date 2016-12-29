#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QFile>
#include <QFileDialog>

#include "configs.h"

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

private slots:
    void on_getSavePathDirToolButton_clicked();
    void on_saveAndCancel_buttom_accepted();

private:
    Ui::Settings *ui;
    Configs* _configsPtr;
    void read_config_to_dialog();
};

#endif // SETTINGS_H
