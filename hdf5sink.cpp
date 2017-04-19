#include "hdf5sink.h"
#include <QDebug>

HDF5Sink::HDF5Sink(QObject *parent, ImageBuffer* bufferPtr, cv::Size size)
    :QObject(parent)
    , _bufferPtr(bufferPtr)
    , _imgSize(size)
    , _frameID(0)
    , _isRunning(false)
    //, _recordThr(CThreadLite::ThreadProc(&HDF5Sink::recordThreadProc, this), "RecordThread")
{
    //传递写文件的地址,熊蜂的编号,记录的时间

    _baseDir = Configs::expconfig.outputBaseDir;
    _bubbleBeeID = Configs::expconfig.bumbleId;
    _recordMoment = QDateTime::currentDateTime();

}

bool HDF5Sink::init_H5File()
{
    if (!QDir(_baseDir).exists())
    {
        qDebug() << "the basedir is not exists";
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
    _filename = _outputPath + "/" + _recordMoment.toString("yyyy.MM.dd_hh-mm-ss") + ".h5";
    _timeStamps.clear();


#ifdef Q_OS_WIN32
    h5_hFilePtr = new H5::H5File(_filename.toStdString().c_str(), H5F_ACC_TRUNC);
#else
    hFile = new H5::H5File(filename.toUtf8().data(), H5F_ACC_TRUNC);
#endif
    //TODO 对不同数据类型的选择
    h5_readType = H5::PredType::NATIVE_UCHAR;
    _dataFormat = "BayerGB";

    if (_dataFormat == "BayerGB")
    {
        h5_reccols = _imgSize.width;
        h5_recrows = _imgSize.height;
    }

    if (_dataFormat == "RGB24")
    {
        h5_reccols = _imgSize.width * 3;
        h5_recrows = _imgSize.height;
    }

    if (_dataFormat == "RGB32")
    {
        h5_reccols = _imgSize.width * 4;
        h5_recrows = _imgSize.height;
    }

    hsize_t dims[3] = { 1, (uint)h5_recrows, (uint)h5_reccols };
    hsize_t maxdims[3] = { H5S_UNLIMITED, (uint)h5_recrows, (uint)h5_reccols };

    H5::DataSpace mspace1(3, dims, maxdims);

    hsize_t chuck_dims[3] = { 1, (uint)h5_recrows, (uint)h5_reccols };

    h5_createParms.setChunk(3, chuck_dims);

    int fill_val = 0;

    h5_createParms.setFillValue(h5_readType, &fill_val);

    //开启压缩，数字越低压缩率越低
    //h5_createParms.setDeflate(1);
    h5_dataSet = h5_hFilePtr->createDataSet("videoframe", h5_readType, mspace1, h5_createParms);

    H5::DataSpace attr_dataspace = H5::DataSpace(H5S_SCALAR);
    if (_dataFormat == "BayerGB")
    {
        H5::StrType strdatatype(H5::PredType::C_S1, 7);
        H5::Attribute attr_in = h5_dataSet.createAttribute("PIXFORMAT", strdatatype, attr_dataspace);
        attr_in.write(strdatatype, "BayerGB");
    }
    else if (_dataFormat == "RGB24")
    {
        //TODO find the predType meaning
        H5::StrType strdatatype(H5::PredType::C_S1, 5);
        H5::Attribute attr_in = h5_dataSet.createAttribute("PIXFORMAT", strdatatype, attr_dataspace);
        attr_in.write(strdatatype, "RGB24");
    }
    else if (_dataFormat == "RGB32")
    {
        H5::StrType strdatatype(H5::PredType::C_S1, 5);
        H5::Attribute attr_in = h5_dataSet.createAttribute("PIXFORMAT", strdatatype, attr_dataspace);
        attr_in.write(strdatatype, "RGB32");
    }

    return true;
}

void HDF5Sink::release()
{
    //write timestamp
    cv::Mat doubleT = cv::Mat(1, _timeStamps.size(), CV_64F);
    //cv::MatIterator_<double> tS, tsEnd;
    //timestamp pointer
    double *tsPtr;
    //write stimulate parameters
    cv::Mat intStiPara = cv::Mat(1, _stiParams.size() * 5, CV_32SC1);
    //cv::MatIterator_<int> sP, spEnd;
    //stimulus parameter pointer
    int *spPtr;
    if (_timeStamps.size() == _stiParams.size())
    {
        for (uint i = 0,j = 0; i < _timeStamps.size(); i++,j+=5)
        {
            //write timestamp from qvector to mat
            tsPtr = doubleT.ptr<double>(0);
            tsPtr[i] = _timeStamps[i];

            //write stimulus parameters from qvector to mat
            spPtr = intStiPara.ptr<int>(0);
            spPtr[j] = _stiParams[i].dutyCycle;
            spPtr[j + 1] = _stiParams[i].frequency;
            spPtr[j + 2] = _stiParams[i].periodCount;
            spPtr[j + 3] = _stiParams[i].stimulusCount;
            spPtr[j + 4] = _stiParams[i].direction;
        }
    }
    else
    {
        for (uint i = 0; i < _timeStamps.size(); i++)
        {
            //write timestamp from qvector to mat
            tsPtr = doubleT.ptr<double>(0);
            tsPtr[i] = _timeStamps[i];
        }

        for (uint i = 0,j = 0; i < _stiParams.size(); i++, j+=5)
        {
            //write stimulus parameters from qvector to mat
            //刺激参数的四列分别是占空比，频率，周期个数，刺激个数
            spPtr = intStiPara.ptr<int>(0);
            spPtr[j] = _stiParams[i].dutyCycle;
            spPtr[j + 1] = _stiParams[i].frequency;
            spPtr[j + 2] = _stiParams[i].periodCount;
            spPtr[j + 3] = _stiParams[i].stimulusCount;
            spPtr[j + 4] = _stiParams[i].direction;
        }
    }
    //写入timestamp
    hsize_t tdims[1] = { _timeStamps.size() };
    H5::DataSpace dataspaceT = H5::DataSpace(1, tdims);
    H5::DataSet datasetT = h5_hFilePtr->createDataSet("time", H5::PredType::NATIVE_DOUBLE, dataspaceT);
    datasetT.write(doubleT.data, H5::PredType::NATIVE_DOUBLE);
    //写入刺激参数
    hsize_t spdims[1] = { _stiParams.size() * 5 };
    H5::DataSpace dataspaceSP = H5::DataSpace(1, spdims);
    H5::DataSet datasetSP = h5_hFilePtr->createDataSet("stiParam", H5::PredType::NATIVE_INT32, dataspaceSP);
    datasetSP.write(intStiPara.data, H5::PredType::NATIVE_INT32);

    h5_createParms.close();
    h5_readType.close();
    //h5_memSpacePtr->close();
    //h5_dataSpacePtr->close();
    h5_dataSet.close();
    h5_hFilePtr->close();
    delete h5_hFilePtr;
    ///_recordThr.destroyThread();
    qDebug() << "H5File Writer release";
    emit(finished());
}
void HDF5Sink::start()
{
    //init H5 files
    if (!init_H5File())
    {
        qDebug() << "init_H5File failed";
        return;
    }
    
    /*
    if (!_recordThr.createThread())
    {
        qDebug() << "Create Record Thread failed";
        return;
    }
    */
    qDebug() << "init_H5File succeed";
    _isRunning = true;
	//Configs::status.s_recording = true;
    recordProc();
    return;
}

bool HDF5Sink::isRecording()
{
    return _isRunning;
}



void HDF5Sink::stop()
{
    _isRunning = false;
	Configs::status.s_recording = false;
}
void HDF5Sink::writeToDisk(ImagePacket& ImagePacket)
{
    if (_frameID == 0)
    {
        h5_dataSet.write(ImagePacket.image.data, h5_readType);
    }
    else
    {
        hsize_t newdims[3] = { _frameID + 1, h5_recrows, h5_reccols };
        h5_dataSet.extend(newdims);
        H5::DataSpace filespace = h5_dataSet.getSpace();
        hsize_t offset[3];
        offset[0] = _frameID;
        offset[1] = 0;
        offset[2] = 0;
        hsize_t dimsIm[3];
        dimsIm[0] = 1;
        dimsIm[1] = h5_recrows;
        dimsIm[2] = h5_reccols;
        filespace.selectHyperslab(H5S_SELECT_SET, dimsIm, offset);

        hsize_t dumbIm[2];
        dumbIm[0] = h5_recrows;
        dumbIm[1] = h5_reccols;
        H5::DataSpace memSpac(2, dumbIm, NULL);

        h5_dataSet.write(ImagePacket.image.data, h5_readType, memSpac, filespace);
    }
}
/*
void HDF5Sink::recordThreadProc(CThreadLite&)
{
    while (_isRunning || !_bufferPtr->isBufferEmpty())
    {
        _tmpImagePacket = _bufferPtr->getFrameToWrite();
        _timeStamps.push_back(_tmpImagePacket.timeStamp);
        _stiParams.push_back(_tmpImagePacket.stimulusParam);
        writeToDisk(_tmpImagePacket);
        _frameID++;
    }
    //缓存清空后释放写入对象
    release();
}
*/
void HDF5Sink::recordProc()
{
    while (_isRunning || !_bufferPtr->isBufferEmpty())
    {
        _tmpImagePacket = _bufferPtr->getFrameToWrite();
        _timeStamps.push_back(_tmpImagePacket.timeStamp);
        _stiParams.push_back(_tmpImagePacket.stimulusParam);
        writeToDisk(_tmpImagePacket);
        _frameID++;
    }
    //缓存清空后释放写入对象
    release();
}
