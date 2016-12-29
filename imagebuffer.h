#ifndef IMAGEBUFFER_H
#define IMAGEBUFFER_H

#include <QMutex>
#include <QVector>
#include <QSemaphore>

#include "boost\circular_buffer.hpp"
#include "opencv2\opencv.hpp"
#include "imagepacket.h"

class ImageBuffer
{

public:
    enum bufferType
	{
		displayBuffer = 0,
		recordBuffer = 1
	};

    ImageBuffer(int size, int type, bool _dropFrame);
    void addFrame(const ImagePacket& frame);
    ImagePacket getFrameToWrite();          //给写图像线程用
    ImagePacket getFrameToDisplay();        //给显示图像线程用
    void clearBuffer();
    int getSizeOfImageBuffer();
	bool isBufferEmpty();

private:
    QMutex imageQueueProtect;
    //QVector<ImagePacket> imageQueue;
	boost::circular_buffer<ImagePacket> imageQueue;
	QSemaphore *freeSlots;
    QSemaphore *usedSlots;
    QSemaphore *clearBuffer1;
    QSemaphore *clearBuffer2;

    int _currentIndex;
    int _bufferSize;
	int _bufferType;
    bool _dropFrame;
};

#endif // IMAGEBUFFER_H
