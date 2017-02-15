#include "opencvsink.h"
#include <QDebug>

OpenCVSink::OpenCVSink(QObject *parent, ImageBuffer *bufferPtr, cv::Size size)
	: QObject(parent)
	, _bufferPtr(bufferPtr)
	, _imgSize(size)
	, _frameID(0)
	, _isRunning(false)
	, _recordThr(CThreadLite::ThreadProc(&OpenCVSink::recordThreadProc, this), "RecordThread")
{
	_baseDir = Configs::expconfig.outputBaseDir;
	_bubbleBeeID = Configs::expconfig.bumbleId;
	_recordMoment = QDateTime::currentDateTime();
}

bool OpenCVSink::init_VideoWriter()
{
	if(!QDir(_baseDir).exists())
	{
		qDebug()<< "the basedir is not exists";
		return false;
	}

	_outputPath = _baseDir + "/" + _bubbleBeeID + "/" + _recordMoment.date().toString("yyyyMMdd");

	if (!QDir(_outputPath).exists())
	{
		QDir tmp;
		if (tmp.mkpath(_outputPath))
		{
			qDebug() << "make dir" << _outputPath;
		}
		else
		{
			qDebug() << "make new dir false";
			return false;
		}
	}

	_frameID = 0;
	fps = 30.0;

	//不同的编码格式测试，均无法满足要求
	//_codec = CV_FOURCC('X', '2', '6', '4');
	//_codec = CV_FOURCC('D', 'I', 'V', 'X');
	//_codec = CV_FOURCC('X', 'V', 'I', 'D');
	//_codec = CV_FOURCC('M', 'J', 'P', 'G');
	//_codec = CV_FOURCC('P', 'I', 'M', '1');
	_codec = -1;	//不进行编码能达到150M/s左右，达不到HDF5写入速度
	
	_filename = _outputPath + "/" + _recordMoment.time().toString("hh-mm-ss") + ".avi";
	//_timeStamps.clear();

	_writerPtr = new cv::VideoWriter();

	if (!_writerPtr->open(_filename.toStdString(), _codec, fps, _imgSize, false))
	{
		qDebug() << "Opencv Videowriter open failed.";
		return false;
	}

	return true;
}

void OpenCVSink::release()
{
	/* opencv无法写入时间戳
	cv::Mat doubleT = cv::Mat(1, _timeStamps.size(), CV_64F);
	for (uint i = 0; i < _timeStamps.size(); i++)
	{
		doubleT.at<double>(0, i) = _timeStamps.at(i);
	}
	*/
	_writerPtr->release();

	qDebug() << "Opencv Videowriter has released";
}

void OpenCVSink::start()
{
	//init VideoWriter
	if(!init_VideoWriter())
	{
		qDebug() << "init_VideoWriter failed";
		return;
	}
	
	if (!_recordThr.createThread())
	{
		qDebug() << "Create Record Thread failed";
		return;
	}
	_isRunning = true;
	Configs::status.s_recording = true;
	qDebug() << "init_VideoWriter succeed";
	return;
}

bool OpenCVSink::isRecording()
{
	return _isRunning;
}

void OpenCVSink::stop()
{
	Configs::status.s_recording = false;
	_isRunning = false;
}


void OpenCVSink::recordThreadProc(CThreadLite&)
{
	while (_isRunning || !_bufferPtr->isBufferEmpty())
	{
		_tmpImagePacket = _bufferPtr->getFrameToWrite();
		_writerPtr->write(_tmpImagePacket.image);
		_frameID++;
		//_timeStamps.push_back(_tmpImagePacket.timeStamp);
	}
	release();
}
