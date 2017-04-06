#ifndef HDF5SINK_H
#define HDF5SINK_H

#define H5_BUILT_AS_DYNAMIC_LIB

#include <QObject>
#include <QDateTime>
#include <QDir>
#include <QString>

#include "configs.h"
#include "H5Cpp.h"
#include "imagepacket.h"
#include "imagebuffer.h"
#include "opencv2/core.hpp"
//#include "Infra/Thread.h"

//using namespace Dahua::Infra;

class HDF5Sink : public QObject
{
    Q_OBJECT
public:
    explicit HDF5Sink(QObject *parent = 0, ImageBuffer* bufferPtr = NULL, cv::Size size = cv::Size());
    bool isRecording();

signals:
    void finished();

public Q_SLOTS:
    void start();
    void stop();
    
private Q_SLOTS:
    bool init_H5File();
    //void getOneFrame();
    void writeToDisk(ImagePacket&);
    void release();
    //void recordThreadProc(CThreadLite&);
    void recordProc();

private:
    //HDF5 related 
    H5::H5File *h5_hFilePtr;
    H5::DataSpace *h5_dataSpacePtr;
    H5::DataSet h5_dataSet;
    H5::DataSpace* h5_memSpacePtr;
    hsize_t h5_recrows, h5_reccols;
    H5::DataType h5_readType;
    H5::DSetCreatPropList h5_createParms;

    ImageBuffer *_bufferPtr;
    ImagePacket _tmpImagePacket;

    
    cv::Size _imgSize;
    QString _baseDir;
    QString _outputPath;
    QDateTime _recordMoment;
    QString _bubbleBeeID;
    QString _basename;
    QString _filename;
    QString _dataFormat;
    QVector<double> _timeStamps;
    QVector<Configs::ParaConfig> _stiParams;
    quint64 _frameID;
    
    Configs *_configPtr;
    //CThreadLite _recordThr;
    bool _isRunning;
};
#endif
