#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QPair>
#include <QTime>
#include <QTimer>
#include <QString>
#include <QSettings>
#include <QDebug>
#include <QValidator>

#include <opencv2/opencv.hpp>

#include "ControlCAN.h"
#include "configs.h"
#include "cqtopencvviewergl.h"
#include "cvth5dialog.h"
#include "genlcamcap.h"
#include "imagepacket.h"
#include "imagebuffer.h"
#include "hdf5sink.h"
#include "opencvsink.h"
#include "settings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    ImageBuffer *displayBufferPtr;
    ImageBuffer *recordBufferPtr;

protected:
    void closeEvent(QCloseEvent *event);

private Q_SLOTS:
    //Camera and record related slots
    void on_cameraConnButton_clicked();
    void on_cameraDisConnButton_clicked();
    void on_cameraPlayButton_clicked();
    void on_startRecordButton_clicked();
    void on_stopRecordButton_clicked();

    //OpenGl widget updater
    void updateFrame();
    //RecordInfo panel updater
    void updateRecordInfoPanel();
    //USBCan related slots
    void on_connectDevice_clicked();
    void on_openCan_clicked();
    //left or right side stimulate button
    void on_leftStimulate_clicked();
    void on_rightStimulate_clicked();
    //Change StimulateStatePic
    void set_state_pic_gray_manual();
    void set_state_pic_green_manual();
    //Change StimulateButtonsEnableState;
    void changeStimulusButtonsState();
	//Change record button state
	void changeRecordButtonsState();
    //sendStimulate func
    void sendStimulate(quint32 direction);
	//改变Config中的Status s_stimulus标记位
	void changeStimulateStatus();
    //void init_status();
    //Menu actions
    void on_actionSetting_triggered();
    void on_actionHDF5Convertor_triggered();

private:

    void cleanRecordInfoPanel();
    void readGeometrySetting();
    void get_stimulte_para();
    void reset_devices();

private:
    Ui::MainWindow *ui;
    QPixmap _grayPixmap, _greenPixmap;

    VCI_INIT_CONFIG _init_config;

    QTimer _displayTimer;
    QTimer _recordInfoUpdaterTimer;
    QDateTime _recordLastUpdateTime;
    qint32 _displayInterval;
    cv::Mat _displayImage;

    GenlCamCap *_genlCamCapPtr;
    QList<QPair<QString, QString>> _camNameKeyList;

    HDF5Sink *_HDF5SinkPtr;
    OpenCVSink *_opencvSinkPtr;

    Configs *_configPtr;
    cvtH5Dialog *_cvtDialogPtr;
    Settings *_setdialogPtr;
    //Videosavepath *_videopathPtr;


};

#endif // MAINWINDOW_H
