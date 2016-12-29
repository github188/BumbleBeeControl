#include "genlcamcap.h"
#include "ui_genlcamcap.h"

#include <QDebug>


GenlCamCap::GenlCamCap(QWidget *parent, ImageBuffer *displayBufferPtr, ImageBuffer *recordBufferPtr)
	: QWidget(parent)
	, ui(new Ui::GenlCamCap)
	, _Discoveried(false)
	, _Grabbing(false)
	, _iFrameCount(0)
	, _displayBufferPtr(displayBufferPtr)
	, _recordBufferPtr(recordBufferPtr)
	, _nTimestampFreq(TIMESTAMPFREQUENCY)
	, _nFirstFrameTime(0)
	, _nLastFrameTime(0)
{
	ui->setupUi(this);
	setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint);
	updateExposureTimeTimer.setInterval(1000);
    connect(&updateExposureTimeTimer, SIGNAL(timeout()), this, SLOT(updatePanelExpoTime()));
    readSetting();
}

GenlCamCap::~GenlCamCap()
{
	delete ui;
}

void GenlCamCap::closeEvent(QCloseEvent *event)
{
    QSettings settings("CSDL", "GenlCamPanel");
    settings.setValue("GenlCamPanel/geometry", saveGeometry());
    QWidget::closeEvent(event);
}

void GenlCamCap::readSetting()
{
    QSettings settings("CSDL", "GenlCamPanel");
    restoreGeometry(settings.value("GenlCamPanel/geometry").toByteArray());
}

bool GenlCamCap::discoveryDevice()
{
	return CSystem::getInstance().discovery(_CameraPtrList);
}


bool GenlCamCap::connectToCamera(quint32 _frameID)
{
	_cameraSptr = _CameraPtrList[_frameID];
	//显示相机信息
	if (!_cameraSptr->connect())
	{
		return false;
	}
	else
	{
		_paramCtrlSetSptr = getParamControl();
		getParam(_paramCtrlSetSptr);
		initPanelParam();
		return setParam(_paramCtrlSetSptr);
	}
}

bool GenlCamCap::disconnectCamera()
{
	stopGrabbing();
	if (_cameraSptr == NULL)
	{
		//错误结果需要专门写一个分类
		return false;
	}

	if (!_cameraSptr->disConnect())
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool GenlCamCap::isCameraConnected()
{
	if (_cameraSptr == NULL)
	{
		//显示错误信息
		return false;
	}

	return (_cameraSptr->isConnected());
}

GenlCamCap::ParamControlSetPtr GenlCamCap::getParamControl()
{
	if (_paramCtrlSetSptr == NULL && _cameraSptr->isConnected())
	{
		_paramCtrlSetSptr = ParamControlSetPtr(new ParamControlSet());
		_paramCtrlSetSptr->_sptrDeviceCtrl = CSystem::getInstance().createDeviceControl(_cameraSptr);
		_paramCtrlSetSptr->_sptrAcquistionCtrl = CSystem::getInstance().createAcquisitionControl(_cameraSptr);
		_paramCtrlSetSptr->_sptrAnalogCtrl = CSystem::getInstance().createAnalogControl(_cameraSptr);
		_paramCtrlSetSptr->_sptrImageFormatCtrl = CSystem::getInstance().createImageFormatControl(_cameraSptr);
		_paramCtrlSetSptr->_sptrIspCtrl = CSystem::getInstance().createISPControl(_cameraSptr);
		_paramCtrlSetSptr->_sptrUserSetCtrl = CSystem::getInstance().createUserSetControl(_cameraSptr);
	}

	return _paramCtrlSetSptr;
}

void GenlCamCap::getParam(ParamControlSetPtr paramControlSptr)
{

	//Acquisition Control Options
	CBoolNode FrameRateEnable = paramControlSptr->_sptrAcquistionCtrl->acquisitionFrameRateEnable();
	CDoubleNode FrameRate = paramControlSptr->_sptrAcquistionCtrl->acquisitionFrameRate();
	CEnumNode ExposureAuto = paramControlSptr->_sptrAcquistionCtrl->exposureAuto();
	CDoubleNode ExposureTime = paramControlSptr->_sptrAcquistionCtrl->exposureTime();
	//read option to temp vars
	FrameRateEnable.getValue(_frameRateEnable);
	FrameRate.getValue(_frameRate);
    FrameRate.getMaxVal(_maxFrameRate);
    FrameRate.getMinVal(_minFrameRate);
	_exposureAuto = ExposureAuto.getEnumSymbolList();
	ExposureTime.getValue(_exposureTime);
	ExposureTime.getMaxVal(_maxExposureTime);
	ExposureTime.getMinVal(_minExposureTime);

	//Analog Control Options
	CEnumNode BlackLevelAuto = paramControlSptr->_sptrAnalogCtrl->blackLevelAuto();
	CEnumNode BalanceWhiteAuto = paramControlSptr->_sptrAnalogCtrl->balanceWhiteAuto();
	//read option to temp vars
	_blackLevelAuto = BlackLevelAuto.getEnumSymbolList();
	_balanceWhiteAuto = BalanceWhiteAuto.getEnumSymbolList();
	
	//Device Control Options
	CStringNode DeviceUserID = paramControlSptr->_sptrDeviceCtrl->deviceUserID();
	//read option to temp vars
	DeviceUserID.getValue(_deviceUserID);

	//ImageFormat Control Options
	CEnumNode PixelFormat = paramControlSptr->_sptrImageFormatCtrl->pixelFormat();
	CIntNode Height = paramControlSptr->_sptrImageFormatCtrl->height();
	CIntNode Width = paramControlSptr->_sptrImageFormatCtrl->width();
	//read option to temp vars
	_pixelFormat = PixelFormat.getEnumSymbolList();
	
	Height.getMaxVal(_maxHeight);
	Height.getMinVal(_minHeight);
	Height.getValue(_height);

	Width.getMaxVal(_maxWidth);
	Width.getMinVal(_minWidth);
	Width.getValue(_width);

	//ISP Control Options
	CIntNode Brightness = paramControlSptr->_sptrIspCtrl->brightness();
	CIntNode Sharpness = paramControlSptr->_sptrIspCtrl->sharpness();
	CIntNode DigitShift = paramControlSptr->_sptrIspCtrl->digitalshift();
	//read option to temp vars
	Brightness.getMaxVal(_maxBrightness);
	Brightness.getMinVal(_minBrightness);
	Brightness.getValue(_brightness);

	Sharpness.getMaxVal(_maxSharpness);
	Sharpness.getMinVal(_minSharpness);
	Sharpness.getValue(_sharpness);

	DigitShift.getMaxVal(_maxDigitShift);
	DigitShift.getMinVal(_minDigitShift);
	DigitShift.getValue(_digitShift);
}

bool GenlCamCap::setParam(ParamControlSetPtr paramControlSptr)
{
	//_paramCtrlSetSptr->_sptrUserSetCtrl->setCurrentUserSet();
	//Acquisition Control Options

	//Analog Control Options
	CEnumNode BlackLevelAuto = paramControlSptr->_sptrAnalogCtrl->blackLevelAuto();
	if (!BlackLevelAuto.setValueBySymbol(CString("Continuous")))
	{
		return false;
	}
	CEnumNode BalanceWhiteAuto = paramControlSptr->_sptrAnalogCtrl->balanceWhiteAuto();
	if (!BalanceWhiteAuto.setValueBySymbol(CString("Continuous")))
	{
		return false;
	}
	//Device Control Options
	//修改设备自定义名称
	CStringNode DeviceUserID = paramControlSptr->_sptrDeviceCtrl->deviceUserID();

	//ImageFormat Control Options
	CEnumNode PixelFormat = paramControlSptr->_sptrImageFormatCtrl->pixelFormat();
	if (!PixelFormat.setValueBySymbol(CString("BayerGB8")))
	{
		return false;
	}
	return true;
}

void GenlCamCap::initPanelParam()
{
	//Image Format 
	ui->heightSpinbox->setMaximum(_maxHeight);
	ui->heightSpinbox->setMinimum(_minHeight);
	ui->heightSpinbox->setValue(_height);
	
	ui->widthSpinBox->setMaximum(_maxWidth);
	ui->widthSpinBox->setMinimum(_minWidth);
	ui->widthSpinBox->setValue(_width);

	QStringList items;
	for (size_t i = 0; i < _pixelFormat.size(); ++i)
	{
		items << QString(_pixelFormat[i].c_str());
	}

	//Acquisition Control Options
	//FrameRateEnable
	//Index:0->True 1->False
	if (_frameRateEnable)
	{
		ui->frameRateEnableCombobox->setCurrentIndex(0);
	} 
	else
	{
		ui->frameRateEnableCombobox->setCurrentIndex(1);
	}
	//FrameRate
    ui->frameRateSpinBox->setValue(_frameRate);
    ui->frameRateSpinBox->setMinimum(_minFrameRate);
    ui->frameRateSpinBox->setMaximum(_maxFrameRate);
	//ExposureAuto
	items.clear();
	for (size_t i = 0; i < _exposureAuto.size(); ++i)
	{
		items << QString(_exposureAuto[i].c_str());
	}
	ui->exposureAutoCombobox->clear();
	ui->exposureAutoCombobox->addItems(items);
	//ExposureTime
	ui->exposureTimeSpinBox->setMaximum(_maxExposureTime);
	ui->exposureTimeSpinBox->setMinimum(_minExposureTime);
	ui->exposureTimeSpinBox->setValue(_exposureTime);

	//Analog Control
	items.clear();
	for (size_t i = 0; i < _blackLevelAuto.size(); ++i)
	{
		items << QString(_blackLevelAuto[i].c_str());
	}
	//ui->black->addItems(items);

	items.clear();
	for (size_t i = 0; i < _balanceWhiteAuto.size(); ++i)
	{
		items << QString(_balanceWhiteAuto[i].c_str());
	}

	//Device Control
	
	//ISP Control
	//brightness
	ui->brightnessSpinBox->setMaximum(_maxBrightness);
	ui->brightnessSpinBox->setMinimum(_minBrightness);
	ui->brightnessSpinBox->setValue(_brightness);
	//sharpness
	ui->sharpnessSpinBox->setMaximum(_maxSharpness);
	ui->sharpnessSpinBox->setMinimum(_minSharpness);
	ui->sharpnessSpinBox->setValue(_sharpness);
	//digitShift
	ui->digitalShiftSpinBox->setMaximum(_maxDigitShift);
	ui->digitalShiftSpinBox->setMinimum(_minDigitShift);
	ui->digitalShiftSpinBox->setValue(_digitShift);
}

QList<QPair<QString, QString> >GenlCamCap::getCamNameKeyList()
{
	QList<QPair<QString, QString> >tmpList;
	if (discoveryDevice())
	{
		for (quint32 i = 0; i < _CameraPtrList.size(); i++)
		{
			tmpList.append(QPair<QString, QString>(QString(_CameraPtrList[i]->getName()), QString(_CameraPtrList[i]->getKey())));
		}
	}
	return tmpList;
}
void GenlCamCap::onGetFrame(const CFrame &frame)
{
	if (!frame.valid())
	{
		return;
	}
	++_iFrameCount;
	_tmpPacket.seqNumber = _iFrameCount;

	if (_iFrameCount == 1)
	{
		_nFirstFrameTime = frame.getImageTimeStamp();
		_nLastFrameTime = _nFirstFrameTime;
	}
	else
	{
		_nLastFrameTime = frame.getImageTimeStamp();
	}

	//写入刺激参数
	if (Configs::status.s_stimulus)
	{
		_tmpPacket.stimulusPara.dutyCycle = Configs::expconfig.paraconfig.dutyCycle;
		_tmpPacket.stimulusPara.frequency = Configs::expconfig.paraconfig.frequency;
		_tmpPacket.stimulusPara.periodCount = Configs::expconfig.paraconfig.periodCount;
		_tmpPacket.stimulusPara.stimulusCount = Configs::expconfig.paraconfig.stimulusCount;
	}
	else
	{
		//未刺激是参数为0
		_tmpPacket.stimulusPara.dutyCycle = 0;
		_tmpPacket.stimulusPara.frequency = 0;
		_tmpPacket.stimulusPara.periodCount = 0;
		_tmpPacket.stimulusPara.stimulusCount = 0;
	}
	//写入图片和时标
	_tmpPacket.image = cv::Mat(frame.getImageHeight(), frame.getImageWidth(), CV_8UC1, (unsigned char*)frame.getImage());
	_tmpPacket.timeStamp = 1000 * (double(_nLastFrameTime - _nFirstFrameTime) / double(_nTimestampFreq));
	//qDebug() << _tmpPacket.timeStamp;
	//加入显示缓存
	_displayBufferPtr->addFrame(_tmpPacket);
	//如果开始记录则也要写到记录缓存中
	if (Configs::status.s_recording)
	{
		_recordBufferPtr->addFrame(_tmpPacket);
	}
	return;
}

bool GenlCamCap::startGrabbing()
{
	_streamPtr = CSystem::getInstance().createStreamSource(_cameraSptr);

	if (_streamPtr == NULL)
	{
		//返回错误信息
		emit(sCreateStreamSourceFailed());
		return false;
	}

	if (_streamPtr->isGrabbing())
	{
		return false;
	}
	_streamPtr->setBufferCount(16);

	if (!_streamPtr->attachGrabbing(IStreamSource::Proc(&GenlCamCap::onGetFrame, this)))
	{
		printf("attch Grabbing fail!\n");
		//实际应用中应及时释放相关资源，如diconnect相机等，不宜直接return
		return false;
	}

	if (!_streamPtr->startGrabbing())
	{
		//返回错误信息
		return false;
	}
	_Grabbing = true;
	//emit(sStartGrabbing());
	return true;
}

bool GenlCamCap::stopGrabbing()
{

	if (_streamPtr == NULL)
	{
		return false;
	}

	if (!_streamPtr->isGrabbing())
	{
		_Grabbing = false;
		return true;
	}

	if (!_streamPtr->detachGrabbing(IStreamSource::Proc(&GenlCamCap::onGetFrame, this)))
	{
		return false;
	}

	_streamPtr->stopGrabbing();
	_streamPtr.reset();
	_Grabbing = false;
	return true;

}
bool GenlCamCap::isGrabbing()
{
	return _Grabbing;
}


bool GenlCamCap::isTimeToDisplay(unsigned long long nCurTime)
{
	CGuard guard(_mxTime);

	// 不显示
	if (_dDisplayInterval <= 0)
	{
		return false;
	}

	// 时间戳频率获取失败, 默认全显示. 这种情况理论上不会出现
	if (_nTimestampFreq <= 0)
	{
		return true;
	}

	// 第一帧必须显示
	if (_nFirstFrameTime == 0 || _nLastFrameTime == 0)
	{
		_nFirstFrameTime = nCurTime;
		_nLastFrameTime = nCurTime;

		warnf("set m_nFirstFrameTime: %I64d\n", _nFirstFrameTime);
		return true;
	}

	// 当前时间戳比之前保存的小
	if (nCurTime < _nFirstFrameTime)
	{
		_nFirstFrameTime = nCurTime;
		_nLastFrameTime = nCurTime;
		warnf("reset m_nFirstFrameTime: %I64d\n", _nFirstFrameTime);
		return true;
	}

	// 当前帧和上一帧的间隔
	uintmax_t nDif = nCurTime - _nLastFrameTime;
	double dTimstampInterval = 1.0 / _nTimestampFreq;

	double dAcquisitionInterval = nDif * 1000 * dTimstampInterval;

	if (dAcquisitionInterval >= _dDisplayInterval)
	{
		// 保存最后一帧的时间戳
		_nLastFrameTime = nCurTime;
		return true;
	}

	// 当前帧相对于第一帧的时间间隔
	uintmax_t nDif2 = nCurTime - _nFirstFrameTime;
	double dCurrentFrameTime = nDif2 * 1000 * dTimstampInterval;

	if (dCurrentFrameTime > 1000 * 60 * 30) // 每隔一段时间更新起始时间
	{
		_nFirstFrameTime = nCurTime;
		warnf("reset m_nFirstFrameTime in period: %I64d\n", _nFirstFrameTime);
	}
	// 保存最后一帧的时间戳
	_nLastFrameTime = nCurTime;

	dCurrentFrameTime = fmod(dCurrentFrameTime, _dDisplayInterval);

	if ((dCurrentFrameTime * 2 < dAcquisitionInterval)
		|| ((_dDisplayInterval - dCurrentFrameTime) * 2 <= dAcquisitionInterval))
	{
		return true;
	}

	return false;
}

//============================================================================
//=========================控制窗口控件操作====================================
//============================================================================
void GenlCamCap::on_widthSpinBox_valueChanged(int arg1)
{
    CIntNode Width = _paramCtrlSetSptr->_sptrImageFormatCtrl->width();
    Width.setValue(arg1);
}

void GenlCamCap::on_heightSpinbox_valueChanged(int arg1)
{
    CIntNode Height = _paramCtrlSetSptr->_sptrImageFormatCtrl->height();
    Height.setValue(arg1);

}
void GenlCamCap::on_frameRateEnableCombobox_currentIndexChanged(int index)
{
    CBoolNode FrameRateEnable = _paramCtrlSetSptr->_sptrAcquistionCtrl->acquisitionFrameRateEnable();
    if(index == 0)
    {
        FrameRateEnable.setValue(true);
        ui->frameRateSpinBox->setEnabled(true);
    }
    else
    {
        FrameRateEnable.setValue(false);
        ui->frameRateSpinBox->setEnabled(false);
    }
}

void GenlCamCap::on_exposureAutoCombobox_currentTextChanged(const QString &arg1)
{
    CEnumNode ExposureAuto = _paramCtrlSetSptr->_sptrAcquistionCtrl->exposureAuto();
    ExposureAuto.setValueBySymbol(CString(arg1.toStdString().c_str()));
    if (arg1 != "Off")
	{
		ui->exposureTimeSpinBox->setEnabled(false);
		updateExposureTimeTimer.start();
	} 
	else
	{
		ui->exposureTimeSpinBox->setEnabled(true);
		updateExposureTimeTimer.stop();
	}
}

void GenlCamCap::on_brightnessSpinBox_valueChanged(int arg1)
{
    CIntNode Brightness = _paramCtrlSetSptr->_sptrIspCtrl->brightness();
    Brightness.setValue(arg1);
}

void GenlCamCap::on_sharpnessSpinBox_valueChanged(int arg1)
{
    CIntNode Sharpness = _paramCtrlSetSptr->_sptrIspCtrl->sharpness();
    Sharpness.setValue(arg1);
}

void GenlCamCap::on_digitalShiftSpinBox_valueChanged(int arg1)
{
    CIntNode DigitShift = _paramCtrlSetSptr->_sptrIspCtrl->digitalshift();
    DigitShift.setValue(arg1);
}

void GenlCamCap::on_frameRateSpinBox_valueChanged(double arg1)
{
    CDoubleNode FrameRate = _paramCtrlSetSptr->_sptrAcquistionCtrl->acquisitionFrameRate();
    FrameRate.setValue(arg1);
}

void GenlCamCap::on_exposureTimeSpinBox_valueChanged(double arg1)
{
    CDoubleNode ExposureTime = _paramCtrlSetSptr->_sptrAcquistionCtrl->exposureTime();
    ExposureTime.setValue(arg1);
}

void GenlCamCap::updatePanelExpoTime()
{
    CDoubleNode ExposureTime = _paramCtrlSetSptr->_sptrAcquistionCtrl->exposureTime();
    ExposureTime.getValue(_exposureTime);
	ui->exposureTimeSpinBox->setValue(_exposureTime);
}
