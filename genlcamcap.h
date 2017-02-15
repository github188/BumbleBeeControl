#ifndef GENLCAMCAP_H
#define GENLCAMCAP_H

#define TIMESTAMPFREQUENCY 1000000000	//大华面阵相机的时间戳频率固定为1GHZ

#include <QWidget>
#include <QList>
#include <QPair>
#include <QString>
#include <QTimer>

#include "GenICam/Camera.h"
#include "GenICam/System.h"
#include "Memory/SharedPtr.h"
#include "Infra/PrintLog.h"

#include "configs.h"
#include "imagepacket.h"
#include "imagebuffer.h"

namespace Ui {
	class GenlCamCap;
}

using namespace Dahua::GenICam;
using namespace Dahua::Infra;
using namespace Dahua::Memory;

class GenlCamCap : public QWidget
{
	Q_OBJECT

public:
	explicit GenlCamCap(QWidget *parent = 0, ImageBuffer *_displayBufferPtr = NULL, ImageBuffer *_recordBufferPtr = NULL);
	~GenlCamCap();
signals:
	void sDiscoveryFalid();
	void sDiscoverySuccess(TVector<ICameraPtr> &);
	void sCreateStreamSourceFailed();
	void sStartGrabbing();

public:
	typedef struct _tag_ParamControl_
	{
		IAcquisitionControlPtr		_sptrAcquistionCtrl;
		IAnalogControlPtr			_sptrAnalogCtrl;
		IDeviceControlPtr			_sptrDeviceCtrl;
		IImageFormatControlPtr		_sptrImageFormatCtrl;
		IISPControlPtr				_sptrIspCtrl;
		IUserSetControlPtr			_sptrUserSetCtrl;
	}ParamControlSet;

	typedef TSharedPtr<ParamControlSet> ParamControlSetPtr;
	typedef std::vector<std::string> ParamVectorType;
	ParamControlSetPtr getParamControl();

protected:
	void closeEvent(QCloseEvent *event);

public slots:
	bool discoveryDevice();
	bool connectToCamera(quint32);
	bool disconnectCamera();
	bool startGrabbing();
	bool stopGrabbing();
	bool isGrabbing();
	//    void stopCaptureThread();
	//    int getAvgFPS();
	bool isCameraConnected();
	//    int getInputSourceWidth();
	//    int getInputSourceHeight();
		//First is Cam Name, second is Cam Key
	QList<QPair<QString, QString> > getCamNameKeyList();


private slots:
	//============================================================================
	//=========================控制窗口控件操作====================================
	//============================================================================
	void on_widthSpinBox_valueChanged(int arg1);

	void on_heightSpinbox_valueChanged(int arg1);

	void on_frameRateEnableCombobox_currentIndexChanged(int index);

	void on_exposureAutoCombobox_currentTextChanged(const QString &arg1);

	void on_brightnessSpinBox_valueChanged(int arg1);

	void on_sharpnessSpinBox_valueChanged(int arg1);

	void on_digitalShiftSpinBox_valueChanged(int arg1);

	void on_frameRateSpinBox_valueChanged(double arg1);

	void on_exposureTimeSpinBox_valueChanged(double arg1);

	void updatePanelExpoTime();

private:
	void getParam(ParamControlSetPtr paramControlSptr);
	bool setParam(ParamControlSetPtr paramControlSptr);

	void initPanelParam();
	void readSetting();
	bool isTimeToDisplay(unsigned long long);
	void onGetFrame(const CFrame &frame);



private:
	Ui::GenlCamCap *ui;
	QTimer updateExposureTimeTimer;

	TVector<ICameraPtr> _CameraPtrList;
	bool				_Discoveried;
	bool				_Grabbing;
	ICameraPtr			_cameraSptr;
	IStreamSourcePtr	_streamPtr;
	ParamControlSetPtr	_paramCtrlSetSptr;
	//CThreadLite			m_grabbingThread;
	quint32				_iFrameCount;
	ImagePacket			_tmpPacket;
	ImageBuffer			*_displayBufferPtr;
	ImageBuffer			*_recordBufferPtr;

	//Display acquisition 
	CMutex				_mxTime;
	double				_dDisplayInterval;         // 显示间隔
	unsigned long long	_nTimestampFreq;           // 时间戳频率
	unsigned long long	_nFirstFrameTime;          // 第一帧的时间戳
	unsigned long long	_nLastFrameTime;           // 上一帧的时间戳

	//================================================
	//==========Temp Var for control param============
	//================================================
	//ImageFormat Control Parameter
	qint64 _maxHeight;
	qint64 _minHeight;
	qint64 _height;
	qint64 _maxWidth;
	qint64 _minWidth;
	qint64 _width;
	TVector<CString> _pixelFormat;

	//Acquisition Control Parameter
	bool _frameRateEnable;
	double _frameRate;
	double _maxFrameRate;
	double _minFrameRate;
	TVector<CString> _exposureAuto;
	double _exposureTime;
	double _maxExposureTime;
	double _minExposureTime;

	//Analog Control Parameter
	TVector<CString> _blackLevelAuto;
	TVector<CString> _balanceWhiteAuto;

	//Device Control Parameter
	CString _deviceUserID;

	//ISP Control Parameter
	qint64 _maxBrightness;
	qint64 _minBrightness;
	qint64 _brightness;

	qint64 _maxSharpness;
	qint64 _minSharpness;
	qint64 _sharpness;

	qint64 _maxDigitShift;
	qint64 _minDigitShift;
	qint64 _digitShift;

};

#endif // GENLCAMCAP_H
