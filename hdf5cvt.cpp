#include "hdf5cvt.h"

HDF52Img::HDF52Img(QObject *parent, QString datasetpath, QString outputdir, QString filetype)
    : QObject(parent)
    , _dataset_name(datasetpath)
    , _storeDir(outputdir)
    , _filetype(filetype)
{

}

bool HDF52Img::readH5File()
{
#ifdef Q_OS_WIN32
    hFilePtr = new H5::H5File(_dataset_name.toStdString().c_str(), H5F_ACC_RDONLY);
#else
    H5::H5File *h5_hFilePtr = new H5::H5File(dataset_name.toUtf8().data(), H5F_ACC_RDONLY);
#endif

    // first find the name of the dataset. This opens all of them to see which ones have 3 dimensions
    // if there are multiple datasets. Give the option to the user which one to choose.
    hsize_t amDataSets = hFilePtr->getNumObjs();
    //        qDebug()<<"Amount datasets is: "<<amDataSets;
    std::vector<H5std_string> goodSets;
    QStringList items;  // only used if more than one valid dataset present
    bool timepresent = false;
    std::string timestring;
    for (hsize_t i = 0; i < amDataSets; i++)
    {
        qDebug() << "begin";
        std::string theName = hFilePtr->getObjnameByIdx(i);
        if (QString::fromStdString(theName).contains("time"))
        {
            timepresent = true;
            timestring = theName;
        }
        qDebug() << "Dataset: " << QString::fromStdString(theName);
        H5::DataSet dumbset = hFilePtr->openDataSet(theName);
        H5::DataSpace dumbspace = dumbset.getSpace();
        int dumbrank = dumbspace.getSimpleExtentNdims();
        if (dumbrank == 3)
        {
            // only requirement is that they have 3 dimensions
            goodSets.push_back(theName);
            items << QString::fromStdString(theName);
        }

        dumbspace.close();
        dumbset.close();
    }
    std::string finalDataset;
    if (goodSets.size() == 0)
    {
        qDebug() << "No valid 3D dataset found";
        //return cv::Mat();
        return false;
    }
    else if (goodSets.size() == 1)
    {
        finalDataset = goodSets[0];
    }
    else
    {
        bool ok;
        QString item = QInputDialog::getItem(NULL, "Which dataset to open",
            "Options:", items, 0, false, &ok);
        if (ok && !item.isEmpty())
        {
            finalDataset = item.toStdString();
            //                qDebug()<<"Selected "<<item;
        }
        else
        {
            //return cv::Mat();
            return false;
        }

    }

    // now open the selected one
    _dataset = hFilePtr->openDataSet(finalDataset);
    _dataSpace = _dataset.getSpace();

    int ndims = _dataSpace.getSimpleExtentDims(_dims, NULL);

    //告知总共Frame数目
    emit(totalFrameNum(_dims[0]));

    if (ndims != 3) qDebug() << "Rank is not 3: " << ndims;
    /*qDebug() << "rank " << ndims << ", dimensions " <<
    (unsigned long)(_dims[0]) << " x " <<
    (unsigned long)(_dims[1]) << " x " <<
    (unsigned long)(_dims[2]);*/



    H5T_class_t _dataclass = _dataset.getTypeClass();
    if (_dataclass == H5T_INTEGER)
    {
        bool hasFormat = _dataset.attrExists("PIXFORMAT");
        if (hasFormat)
        {
            H5::Attribute _attr_pix = _dataset.openAttribute("PIXFORMAT");
            std::string strbuf;
            H5::StrType _strdatatype(H5::PredType::C_S1, 10);
            _attr_pix.read(_strdatatype, strbuf);
            _dataFormat = QString::fromStdString(strbuf);
        }
        //            qDebug()<<"Attribute contents: "<<dataformat;


        H5::IntType _intype = _dataset.getIntType();
        size_t size = _intype.getSize();


        if (size == 1)
        {
            if (_dataFormat == "RGB8")
            {
                h5_readType = H5::PredType::NATIVE_UCHAR;
                _bufferFrame = cv::Mat(_dims[1], _dims[2] / 3., CV_8UC3);
                //dataformat="RGB8";
            }
            else if (_dataFormat == "BayerGB")
            {
                h5_readType = H5::PredType::NATIVE_UCHAR;
                _bufferFrame = cv::Mat(_dims[1], _dims[2], CV_8U);
                if (!hasFormat)
                {
                    _dataFormat = "BayerGB";
                }
            }
            else
            {
                h5_readType = H5::PredType::NATIVE_UCHAR;
                _bufferFrame = cv::Mat(_dims[1], _dims[2] / 4., CV_8UC4);
            }
        }
        else if (size == 2)
        {
            h5_readType = H5::PredType::NATIVE_UINT16;
            _bufferFrame = cv::Mat(_dims[1], _dims[2], CV_16U);
            if (!hasFormat)
            {
                _dataFormat = "MONO14"; // not knowing if it is 12 or 14, 14 is the safer choice for displaying will check attribute later.
            }
        }
        else
        {
            qDebug() << "Integer size not yet handled: " << size;
            return false;
        }

    }
    else if (_dataclass == H5T_FLOAT)
    {
        _dataFormat = "FLOAT";
        h5_readType = H5::PredType::NATIVE_FLOAT;
        _bufferFrame = cv::Mat(_dims[1], _dims[2], CV_32F);
    }
    else if (_dataclass == H5T_COMPOUND)
    {
        //typically a complex number => no meaningfull way to show this so exit
        qDebug() << "Data set has compound type - will exit";
        return false;
    }
    else
    {
        qDebug() << "Data set has unknown type - will exit";
        return false;
    }
    std::vector<double> timestamps;


    if (timepresent)
    {
        timestamps.clear();
        H5::DataSet timeset = hFilePtr->openDataSet(timestring);
        H5::DataSpace timespace = timeset.getSpace();
        hsize_t timedim[1];
        int ndims = timespace.getSimpleExtentDims(timedim, NULL);
        if (ndims != 1) qDebug() << "Rank is not 1: " << ndims;

        cv::Mat timeMat = cv::Mat(1, timedim[0], CV_64F);
        timeset.read(timeMat.data, H5::PredType::NATIVE_DOUBLE);

        for (uint i = 0; i < timedim[0]; i++) {
            timestamps.push_back(timeMat.at<double>(0, i));
        }
    }

	return true;
}

bool HDF52Img::closeH5File()
{
    try {
        hFilePtr->close();
        delete hFilePtr;
    } catch (Exception e) {
        qDebug()<<"Error occured: "<<e.getCDetailMsg();
        return false;
    }

    return true;
}

bool HDF52Img::convertFile()
{
	cv::Mat _outputFrame;
    for (hsize_t frameIdx = 0; frameIdx < _dims[0]; frameIdx++)
    {
        if (_dataFormat == "BayerGB")
        {
            hsize_t dimsSlab[3] = { 1, _dims[1], _dims[2] };
            hsize_t offset[3] = { frameIdx,0,0 };
            _dataSpace.selectHyperslab(H5S_SELECT_SET, dimsSlab, offset);
        }

        hsize_t readDims[2] = { _dims[1],_dims[2] };
        H5::DataSpace memspace(2, readDims);
        
		_dataset.read(_bufferFrame.data, h5_readType, memspace, _dataSpace);
		_dataSpace.selectNone();

        _outputFrame = _bufferFrame.clone();
        cv::cvtColor(_outputFrame, _outputFrame, CV_BayerGB2RGB);

        _outputFileName = _storeDir +"/" + QString::number(frameIdx) + "." + _filetype;

        cv::imwrite(_outputFileName.toStdString(), _outputFrame);

        emit(finishedFrameCount(frameIdx + 1));
    }

    return true;
}


bool HDF52Img::cvtH52Img()
{
    if(!readH5File())
    {
        return false;
    }

    if(!convertFile())
    {
        return false;
    }

    if(!closeH5File())
    {
        return false;
    }

    emit(finished());
    return true;
}
