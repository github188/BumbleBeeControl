#ifndef CANCONFIGS_H
#define CANCONFIGS_H

#include <QSettings>
#include <QtGlobal>
#include <QStringList>
#include <QDir>
#include <QDateTime>
#include <QMap>
#include <QPair>
#include "ControlCAN.h"

class Configs
{
public:
	Configs();
	~Configs();

public:
	struct Status
	{
		//_configPtr state
		bool s_haveRead;
		//UsbCan state
		bool s_canconnect;
		bool s_canstart;
		//Camera state
		bool s_camconnect;
		//Record state
		bool s_recording;
		//simulate state
		bool s_stimulus;
	};

	//USBCAN _configPtr
	struct UsbCanConfig
	{
		QString  acceptCode;     //验证码
		QString  mask;           //掩码
		quint32  filterMode;     //过滤方式
		quint32  timing0;        //定时器0
		quint32  timing1;        //定时器1
		QString  baudRate;       //波特率
		quint32  mode;           //模式
		quint32  canNum;         //CAN路数
		quint32  indexNum;       //索引号
		QString  devName;        //设备名称
		quint32  devType;        //设备类型
		QMap<QString, quint32> mapDevType;
		QMap<QString,QPair<quint32,quint32> > baudRate_to_timers;
	};
	//Camera Config
	struct CamConfig
	{
		quint32 width;
		quint32 height;
		quint32 fps;
		quint32 idx;
	};
	//Stimulus Parameter Configurations   
	struct ParaConfig
	{
		qint32 dutyCycle;
		qint32 frequency;
		qint32 periodCount; 
		qint32 stimulusCount;
		qint32 stimulusInterval;
		qint32 deriction;
	};
	//experiment
	struct ExpConfig
	{
		QString bumbleId;       //输出熊蜂编号
		quint32 trainTrial;		//训练次数
		QString outputBaseDir;  //输出文件基文件夹
		ParaConfig paraconfig;  //刺激参数
	};

	static Status status; 
	static CamConfig camconfig;
	static UsbCanConfig usbcanconfig;
	static ExpConfig expconfig;

	void read_configs();
	void write_configs();
};
#endif // CONFIGS_H
