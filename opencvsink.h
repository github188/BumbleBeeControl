#ifndef OPENCVSINK_H
#define OPENCVSINK_H

#include <QObject>
#include <QDateTime>
#include <QDir>
#include <QString>

#include "Infra/Thread.h"

#include "configs.h"
#include "imagebuffer.h"
#include "imagepacket.h"
#include "opencv2/core.hpp"

using namespace Dahua::Infra;

class OpenCVSink : public QObject
{
    Q_OBJECT
public:
    explicit OpenCVSink(QObject *parten = 0, ImageBuffer *bufferPtr = NULL, cv::Size size = cv::Size());
    bool isRecording();

public Q_SLOTS:
    void start();
    void stop();

private Q_SLOTS:
    bool init_VideoWriter();
    void release();
	void recordThreadProc(CThreadLite&);

private:
    ImageBuffer *_bufferPtr;
    ImagePacket _tmpImagePacket;

    cv::VideoWriter *_writerPtr;
    cv::Size _imgSize;
    QString _baseDir;
    QString _outputPath;
    QDateTime _recordMoment;
    QString _bubbleBeeID;
    QString _basename;
    QString _filename;
    //QVector<double> _timeStamps;
    quint64 _frameID;

	int _codec;		//视频编码模式
	double fps;	//视频播放fps

	bool _isRunning;

    Configs *_configPtr;
	CThreadLite _recordThr;
};

#endif // OPENCVSINK_H
