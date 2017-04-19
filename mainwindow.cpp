//TODO 去除global类，替换成config类中
#include "mainwindow.h"
#include "ui_mainwindow.h"




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _displayInterval(1000/40)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //注册ImagePacket,使其能在信号槽中传递
    qRegisterMetaType<ImagePacket>("imagePacket");
    qRegisterMetaType<StimulusParams>("stimulusparams");

    //设置显示时间间隔并连接QTimer与更新函数
    _displayTimer.setInterval(_displayInterval);
	_recordInfoUpdaterTimer.setInterval(500);
    connect(&_displayTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
	connect(&_recordInfoUpdaterTimer, SIGNAL(timeout()), this, SLOT(updateRecordInfoPanel()));
    //初始化Buffers
    recordBufferPtr = new ImageBuffer(1200, ImageBuffer::bufferType::recordBuffer ,false);
    displayBufferPtr = new ImageBuffer(1, ImageBuffer::bufferType::displayBuffer, false);

    //实例化一个CanConfig对象并读取ini文件
    _configPtr = new Configs;
    _configPtr->read_configs();

    //初始化status
//    init_status();

    //刺激状态图片
    _grayPixmap = QPixmap(":/gray.png");
    _greenPixmap = QPixmap (":/green.png");

    //设置刺激参数输入范围
    QValidator *dutyCycle_validator = new QIntValidator(0,99, this);
    QValidator *frequency_validator = new QIntValidator(0,9999, this);
    QValidator *periodCount_validator = new QIntValidator(0,100000, this);
    QValidator *stimulusCount_validator = new QIntValidator(0,1000,this);

    ui->dutyCycle_input_left->setValidator(dutyCycle_validator);
    ui->frequency_input_left->setValidator(frequency_validator);
    ui->periodCount_input_left->setValidator(periodCount_validator);
    ui->stimulusCount_input_left->setValidator(stimulusCount_validator);

    ui->dutyCycle_input_right->setValidator(dutyCycle_validator);
    ui->frequency_input_right->setValidator(frequency_validator);
    ui->periodCount_input_right->setValidator(periodCount_validator);
    ui->stimulusCount_input_right->setValidator(stimulusCount_validator);
    //读取窗口位置信息并还原
    readGeometrySetting();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//重写closeEvent事件，保存窗口位置大小，在相机未断开的情况下关闭控制窗口,在还在录制过程中阻止关闭窗口
void MainWindow::closeEvent(QCloseEvent *event)
{
    if(Configs::status.s_camconnect)
    {
        _genlCamCapPtr->close();
    }
    if(Configs::status.s_recording)
    {
        QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("视频写入还未完成，请勿关闭！"));
		event->ignore();
    }
    else
    {
        QSettings settings("CSDL", "BumbelBeeControl");
        settings.setValue("BumbelBeeControl/geometry", saveGeometry());
        settings.setValue("BumbelBeeControl/windowState", saveState());
		event->accept();
    }
}

//读取上次关闭时窗口大小位置参数
void MainWindow::readGeometrySetting()
{
    QSettings settings("CSDL", "BumbelBeeControl");
    restoreGeometry(settings.value("BumbelBeeControl/geometry").toByteArray());
    restoreState(settings.value("BumbelBeeControl/windowState").toByteArray());
}

//更新OpenGl Widget 函数
void MainWindow::updateFrame()
{
    if (!displayBufferPtr->isBufferEmpty())
    {
        cv::cvtColor(displayBufferPtr->getFrameToDisplay().image, _displayImage, CV_BayerGB2RGB);
        ui->viewfinder->showImage(_displayImage);
    }
}
/*

void MainWindow::init_status()
{
    global::status.isRecording = false;
    global::status.isPause = false;
    global::status.isRunning = false;
    global::status.camCalibIsInit = false;
    global::status.frameId = 0;
    global::status.fRecordTime = QTime(0,0,0,0);
    }
*/

//右侧边栏按钮槽函数
//========================相机操作按钮=========================
void MainWindow::on_cameraConnButton_clicked()
{
    _genlCamCapPtr = new GenlCamCap(NULL, displayBufferPtr, recordBufferPtr);
    
    if (_genlCamCapPtr->discoveryDevice())
    {
        _camNameKeyList = _genlCamCapPtr->getCamNameKeyList();
    }

    if (_camNameKeyList.size() == 0)
    {
        qDebug() << "Can not find camera";
        return;
    }
    
    if(_genlCamCapPtr->connectToCamera(0))
    {
        qDebug() << "Connect Success";
        Configs::status.s_camconnect = true;
        //GenlCamCap::ParamControlSetPtr camPara = _genlCamCapPtr->getParamControl();
        _genlCamCapPtr->show();
    }
    //Enable Button
    ui->cameraConnButton->setEnabled(false);
    ui->cameraDisConnButton->setEnabled(true);
    ui->cameraPlayButton->setEnabled(true);
}
void MainWindow::on_cameraDisConnButton_clicked()
{
    if(_genlCamCapPtr->disconnectCamera())
    {
        Configs::status.s_camconnect = false;
        qDebug() << "Disconnect Success";
        _genlCamCapPtr->close();
    }
    ui->cameraConnButton->setEnabled(true);
    ui->cameraDisConnButton->setEnabled(false);
    ui->cameraPlayButton->setEnabled(false);
    ui->startRecordButton->setEnabled(false);
    ui->stopRecordButton->setEnabled(false);

}

void MainWindow::on_cameraPlayButton_clicked()
{
    //Begin Grabbing
    if(_genlCamCapPtr->isGrabbing())
    {
        if(_genlCamCapPtr->stopGrabbing())
        {
            _displayTimer.stop();
            ui->viewfinder->clearScene();
            ui->cameraPlayButton->setText(QString::fromLocal8Bit("播放"));
            ui->cameraDisConnButton->setEnabled(true);
            ui->startRecordButton->setEnabled(false);
        }

    }
    else
    {
        if (_genlCamCapPtr->startGrabbing())
        {
            ui->viewfinder->clearScene();
            _displayTimer.start();
            ui->cameraPlayButton->setText(QString::fromLocal8Bit("停止"));
            ui->cameraDisConnButton->setEnabled(false);
            ui->startRecordButton->setEnabled(true);
        }
    }
}


void MainWindow::on_startRecordButton_clicked()
{
    _HDF5SinkPtr = new HDF5Sink(NULL, recordBufferPtr, cv::Size(_displayImage.cols, _displayImage.rows));
    //_opencvSinkPtr = new OpenCVSink(NULL, recordBufferPtr, cv::Size(_displayImage.cols, _displayImage.rows));
    //_opencvSinkPtr->start();
    //_HDF5SinkPtr->start();
    //设置定时器并绑定
    

    //改变状态，界面按钮并开启定时器
    //Configs::status.s_recording = true;
	//_recordInfoUpdaterTimer.start();
    

    ///test func
    _h5SinkThreadPtr = new QThread();

    connect(_h5SinkThreadPtr, SIGNAL(started()), _HDF5SinkPtr, SLOT(start()));
    connect(_HDF5SinkPtr, SIGNAL(finished()), _h5SinkThreadPtr, SLOT(quit()));
    connect(_HDF5SinkPtr, SIGNAL(finished()), this, SLOT(changeRecordButtonsState()));
    connect(_h5SinkThreadPtr, SIGNAL(finished()), _h5SinkThreadPtr, SLOT(deleteLater()));

    _HDF5SinkPtr->moveToThread(_h5SinkThreadPtr);

    _h5SinkThreadPtr->start();
	_recordInfoUpdaterTimer.start();
    ///test
    //ui->disconnectButton->setEnabled(false);
}

void MainWindow::on_stopRecordButton_clicked()
{
    _HDF5SinkPtr->stop();
    //_opencvSinkPtr->stop();
    //关闭定时器
    _recordInfoUpdaterTimer.stop();

    cleanRecordInfoPanel();
}

void MainWindow::changeRecordButtonsState()
{
    if (Configs::status.s_recording)
    {
        ui->stopRecordButton->setEnabled(true);
        ui->startRecordButton->setEnabled(false);
    }
    else
    {
        ui->stopRecordButton->setEnabled(false);
        ui->startRecordButton->setEnabled(true);
    }
}

void MainWindow::updateRecordInfoPanel()
{
    if(!Configs::status.s_recording)
    {
        ui->recordBeginTime_value->setText(QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss"));
        ui->recordDuration_value->setText(QTime(0,0,0,0).toString("mm:ss"));
        ui->bumblebeeId_value->setText(Configs::expconfig.bumbleId);
        ui->trainTrial_value->setText(QString::number(Configs::expconfig.trainTrial));
		_recordLastUpdateTime = QDateTime::currentMSecsSinceEpoch();
        Configs::status.s_recording = true;
        changeRecordButtonsState();
    }
    else
    {
		//QDateTime recordDuration;
		//qDebug() << recordDuration.fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch() - _recordLastUpdateTime).time().toString("hh:mm:ss");
		QString during = QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch() - _recordLastUpdateTime, QTimeZone::systemTimeZone()).time().toString("mm:ss");
		//_recordLastUpdateTime = QDateTime::currentDateTime();
        ui->recordDuration_value->setText(during);
    }
}

void MainWindow::cleanRecordInfoPanel()
{
    ui->recordBeginTime_value->setText("");
    ui->recordDuration_value->setText("");
    ui->bumblebeeId_value->setText("");
    ui->trainTrial_value->setText("");
}


//=====================发送指令相关=========================
void MainWindow::on_connectDevice_clicked()
{
    if(Configs::status.s_canconnect)
    {
        reset_devices();
    }
    else
    {
        if(!Configs::status.s_haveRead)
        {
            _configPtr->read_configs();
        }
        VCI_INIT_CONFIG _init_config;
        _init_config.AccCode = Configs::usbcanconfig.acceptCode.toUInt(Q_NULLPTR, 16);
        _init_config.AccMask = Configs::usbcanconfig.mask.toUInt(Q_NULLPTR, 16);
        _init_config.Filter = UCHAR(Configs::usbcanconfig.filterMode);
        _init_config.Mode = UCHAR(Configs::usbcanconfig.mode);
        _init_config.Reserved = DWORD(0xcccccccc);
        _init_config.Timing0 = UCHAR(Configs::usbcanconfig.baudRate_to_timers
                                    .value(Configs::usbcanconfig.baudRate).first);
        _init_config.Timing1 = UCHAR(Configs::usbcanconfig.baudRate_to_timers
                                    .value(Configs::usbcanconfig.baudRate).second);

        if(VCI_OpenDevice(Configs::usbcanconfig.devType, Configs::usbcanconfig.indexNum, 0) != STATUS_OK)
        {
            QMessageBox::warning(this,"Error", QString::fromLocal8Bit("打开设备失败!"));
            return;
        }
        if(VCI_InitCAN(Configs::usbcanconfig.devType, Configs::usbcanconfig.indexNum, Configs::usbcanconfig.canNum, &_init_config) != STATUS_OK)
        {
            QMessageBox::warning(this, "Error", QString::fromLocal8Bit("初始化CAN失败!"));
            reset_devices();
            return;
        }

        Configs::status.s_canconnect = true;
        ui->connectDevice->setText(QString::fromLocal8Bit("断开设备"));
        ui->openCan->setEnabled(true);

    }
}

void MainWindow::on_openCan_clicked()
{
    if(!Configs::status.s_canconnect)
    {
        QMessageBox::warning(this, "Error", QString::fromLocal8Bit("未打开设备!"));
        reset_devices();
        return;
    }

    if(Configs::status.s_canstart)
    {
        if(VCI_ResetCAN(Configs::usbcanconfig.devType, Configs::usbcanconfig.indexNum, Configs::usbcanconfig.canNum)!= STATUS_OK)
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("复位失败"));
        }
        else
        {
            QMessageBox::information(this, "Info", QString::fromLocal8Bit("复位成功"));
            ui->openCan->setText(QString::fromLocal8Bit("打开CAN"));
            changeStimulusButtonsState();
            Configs::status.s_canstart = false;
        }
    }
    else
    {
        if(VCI_StartCAN(Configs::usbcanconfig.devType, Configs::usbcanconfig.indexNum, Configs::usbcanconfig.canNum) != STATUS_OK)
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit( "打开CAN失败!"));
            reset_devices();
        }
        else
        {
            Configs::status.s_canstart = true;
            ui->openCan->setText(QString::fromLocal8Bit("重置CAN"));
            changeStimulusButtonsState();
        }
    }
}

void MainWindow::setStiParamFromPanel(StimulusParams *stiParamPtr, qint32 deriction)
{

    //从面板获取设置参数
    if(deriction == 0)
    {
        Configs::expconfig.paraconfig.dutyCycle = ui->dutyCycle_input_left->text().toInt();
        Configs::expconfig.paraconfig.frequency = ui->frequency_input_left->text().toInt();
        Configs::expconfig.paraconfig.periodCount = ui->periodCount_input_left->text().toInt();
        Configs::expconfig.paraconfig.stimulusCount = ui->stimulusCount_input_left->text().toInt();
    }
    else
    {
        Configs::expconfig.paraconfig.dutyCycle = ui->dutyCycle_input_right->text().toInt();
        Configs::expconfig.paraconfig.frequency = ui->frequency_input_right->text().toInt();
        Configs::expconfig.paraconfig.periodCount = ui->periodCount_input_right->text().toInt();
        Configs::expconfig.paraconfig.stimulusCount = ui->stimulusCount_input_right->text().toInt();
    }

    //刺激间隔暂时无设置，默认为0
    Configs::expconfig.paraconfig.stimulusInterval = 0;
    Configs::expconfig.paraconfig.direction = deriction;

    //写入参数包中
    stiParamPtr->_dutyCycle = quint32(Configs::expconfig.paraconfig.dutyCycle);
    stiParamPtr->_frequency = quint32(Configs::expconfig.paraconfig.frequency);
    stiParamPtr->_periodCount = quint32(Configs::expconfig.paraconfig.periodCount);
    stiParamPtr->_stimulusCount = quint32(Configs::expconfig.paraconfig.stimulusCount);
    stiParamPtr->_stimulusInterval = quint32(Configs::expconfig.paraconfig.stimulusInterval);
    stiParamPtr->_deriction = quint32(Configs::expconfig.paraconfig.direction);

}

void MainWindow::sendStimulate(StimulusParams* params)
{
    VCI_CAN_OBJ frameinfo;
    //
    frameinfo.TimeStamp = 0;
    //
    frameinfo.TimeFlag = 0;
    //发送方式为正常发送
    frameinfo.SendType = 0;
    //帧类型为标准帧
    frameinfo.ExternFlag = 0;
    //帧格式为数据帧
    frameinfo.RemoteFlag = 0;
    //帧ID为0x00000010
    frameinfo.ID = 0x00000010;
    //设置数据长度(已经写死)
    frameinfo.DataLen = 8;

    //设置数据
    memcpy(&frameinfo.Data, params->sendDataTmp, frameinfo.DataLen);
    //此处的传输数据长度不明，需要测试
    if(VCI_Transmit(Configs::usbcanconfig.devType, Configs::usbcanconfig.indexNum, Configs::usbcanconfig.canNum, &frameinfo, 8))
    {
        
        //以刺激时间长度来改变状态图片颜色以及刺激状态
        changeStimulateStatus();
        set_state_pic_green_manual();
        changeStimulusButtonsState();
        //刺激时长(ms)=1000*((周期数/刺激频率)*刺激次数 + 刺激间隔*（刺激次数-1))
        qint32 stimulusTime = qint32(((double(params->_periodCount) / double(params->_frequency)
                * params->_stimulusCount) + (params->_stimulusInterval * (params->_stimulusCount - 1)))*1000);
        QTimer::singleShot(stimulusTime, this, SLOT(set_state_pic_gray_manual()));
        QTimer::singleShot(stimulusTime, this, SLOT(changeStimulateStatus()));
        //enable stimulate button 延迟时间还需要修改
        QTimer::singleShot(1.5 * stimulusTime, this, SLOT(changeStimulusButtonsState()));
        //显示时间为1s
        ui->statusBar->showMessage(QString::fromLocal8Bit("指令写入成功"), 1000);

    }
    else
    {
        ui->statusBar->showMessage(QString::fromLocal8Bit("指令写入失败"), 1000);
    }
}
//改变刺激状态
void MainWindow::changeStimulateStatus()
{
    if (Configs::status.s_stimulus)
    {
        Configs::status.s_stimulus = false;
    }
    else
    {
        Configs::status.s_stimulus = true;
    }
}
//选择左右刺激
//left or right stimulate button click slot
void MainWindow::on_leftStimulate_clicked()
{
    _stiParamPtr = new StimulusParams;
    setStiParamFromPanel(_stiParamPtr, 0);
    _stiParamPtr->paramsTrans();
    sendStimulate(_stiParamPtr);
}

void MainWindow::on_rightStimulate_clicked()
{
    _stiParamPtr = new StimulusParams;
    setStiParamFromPanel(_stiParamPtr, 1);
    _stiParamPtr->paramsTrans();
    sendStimulate(_stiParamPtr);
}

//Change Stimulate button enable state
void MainWindow::changeStimulusButtonsState()
{
    if(!ui->leftStimulate->isEnabled())
    {
        ui->leftStimulate->setEnabled(true);
        ui->rightStimulate->setEnabled(true);
    }
    else
    {
        ui->leftStimulate->setEnabled(false);
        ui->rightStimulate->setEnabled(false);
    }
}

void MainWindow::reset_devices()
{
    Configs::status.s_canconnect = false;
    Configs::status.s_canstart = false;
    ui->connectDevice->setText(QString::fromLocal8Bit("连接设备"));
    ui->openCan->setText(QString::fromLocal8Bit("打开CAN"));
    ui->openCan->setEnabled(false);
    VCI_CloseDevice(Configs::usbcanconfig.devType, Configs::usbcanconfig.indexNum);
}

//点击按钮改变刺激状态图片(改变颜色)
void MainWindow::set_state_pic_gray_manual()
{
    ui->statePic->setPixmap(_grayPixmap);
}

void MainWindow::set_state_pic_green_manual()
{
    ui->statePic->setPixmap(_greenPixmap);
}


//菜单栏按钮槽函数
//=========================setting Dialog=========================
void MainWindow::on_actionSetting_triggered()
{
    _setdialogPtr = new Settings(this);
    _setdialogPtr->show();
}

void MainWindow::on_actionHDF5Convertor_triggered()
{
    _cvtDialogPtr =  new cvtH5Dialog(this);
    _cvtDialogPtr->show();
}





