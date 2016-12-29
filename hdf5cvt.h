#ifndef HDF52IMG_H
#define HDF52IMG_H
#define H5_BUILT_AS_DYNAMIC_LIB

#include <H5Cpp.h>
#include <opencv2/opencv.hpp>
#include <QInputDialog>
#include <QFileInfo>
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QObject>

using namespace H5;

class HDF52Img : public QObject
{
    Q_OBJECT
public:
    explicit HDF52Img(QObject *parent = 0, QString datasetpath = "", QString outputdir = "", QString filetype = "");

signals:
    void totalFrameNum(qulonglong count);
    void finishedFrameCount(qulonglong count);
    void finished();

public slots:
    bool cvtH52Img();

private slots:
    bool convertFile();
    bool readH5File();
    bool closeH5File();

private:
    QString _dataset_name;
    QString _storeDir;
    QString _filetype;
    QString _outputFileName;
    DataSpace _dataSpace;
    QString _dataFormat;
    DataSet _dataset;
    DataType h5_readType;
    H5File *hFilePtr;
    hsize_t _dims[3];

    cv::Mat _bufferFrame;
};

#endif // HDF52IMG_H
